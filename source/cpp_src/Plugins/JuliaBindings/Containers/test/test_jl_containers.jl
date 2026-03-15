#!/usr/bin/env julia
# test_jl_containers.jl
#
# Usage:
#   julia test_jl_containers.jl <path-to-scan-directory> [baseline]
#
# Example:
#   julia test_jl_containers.jl /data/vt9105/1111/105-1800 GE
#
# The scan directory should be a HOPS4 scan directory containing at least
# one baseline data file.
#
# Requires HOPS_JULIA_MODULES_DIR to be set (source hopsenv.sh first).

using CxxWrap
using JSON3

# -- Library path from environment ------------------------------------------
const JLMHO_CONTAINERS_LIB = joinpath(ENV["HOPS_JULIA_MODULES_DIR"], "libjlMHO_Containers.so")

@wrapmodule(() -> JLMHO_CONTAINERS_LIB)
function __init__()
    @initcxx
end

# -- Helpers -----------------------------------------------------------------

"""Pretty-print the container object list from a ContainerStore."""
function print_object_list(cstore)
    objs = JSON3.read(get_object_id_list(cstore))
    println("  Objects in store ($(length(objs))):")
    for obj in objs
        println("    shortname=$(obj["shortname"])  type_uuid=$(obj["type_uuid"])  object_uuid=$(obj["object_uuid"])")
    end
end

# -- Main --------------------------------------------------------------------

function main()
    if length(ARGS) < 1
        println("Usage: julia test_jl_containers.jl <scan-directory> [baseline]")
        println("  baseline defaults to the first baseline found in the scan.")
        exit(1)
    end

    scan_dir = ARGS[1]
    requested_bl = length(ARGS) >= 2 ? ARGS[2] : ""

    println("=" ^ 60)
    println("HOPS Julia Container Binding Test")
    println("=" ^ 60)
    println("Library : $JLMHO_CONTAINERS_LIB")
    println("Scan dir: $scan_dir")

    # -- 1. Initialise scan store --------------------------------------------
    ss = MHO_JlScanStoreInterface()
    set_directory(ss, scan_dir)
    ok = initialize(ss)
    if !ok
        println("ERROR: initialize() failed - check that '$scan_dir' is a valid HOPS4 scan directory.")
        exit(1)
    end
    println("\n[1] Scan store initialised (is_valid=$(is_valid(ss)))")

    # -- 2. List available baselines -----------------------------------------
    baselines = get_baseline_list(ss)
    println("[2] Baselines present: $baselines")
    if isempty(baselines)
        println("ERROR: No baselines found in scan directory.")
        exit(1)
    end

    # Pick baseline
    bl = isempty(requested_bl) ? baselines[1] : requested_bl
    if !has_baseline(ss, bl)
        println("ERROR: Baseline '$bl' not found. Available: $baselines")
        exit(1)
    end
    println("    Using baseline: $bl")

    # -- 3. Load baseline ----------------------------------------------------
    rc = load_baseline(ss, bl)
    println("[3] load_baseline(\"$bl\") returned $rc  (0=ok, 1=already loaded, -1=not init, -2=not present)")
    if rc < 0
        println("ERROR: Failed to load baseline.")
        exit(1)
    end

    # -- 4. Inspect container store ------------------------------------------
    cs = get_baseline_data(ss, bl)
    println("[4] ContainerStore is_valid=$(is_valid(cs))  nobjects=$(get_nobjects(cs))")
    print_object_list(cs)

    # -- 5. Type UUID diagnostics --------------------------------------------
    println("\n[5] Type UUID diagnostics:")
    objs = JSON3.read(get_object_id_list(cs))
    for obj in objs
        sn       = obj["shortname"]
        obj_uuid = obj["object_uuid"]
        type_uuid_file = obj["type_uuid"]
        type_uuid_rt   = get_object_type_uuid(cs, String(obj_uuid))
        match = (type_uuid_file == type_uuid_rt) ? "OK" : "MISMATCH"
        println("    $sn: file_type_uuid=$type_uuid_file  runtime=$type_uuid_rt  $match")
    end

    # -- 5b. Runtime type UUIDs for known container types --------------------
    println("\n[5b] Registered container type UUIDs:")
    vis_type_uuid       = get_visibility_type_uuid(cs)
    vis_store_type_uuid = get_visibility_store_type_uuid(cs)
    wt_type_uuid        = get_weight_type_uuid(cs)
    wt_store_type_uuid  = get_weight_store_type_uuid(cs)
    println("    visibility_type       = $vis_type_uuid")
    println("    visibility_store_type = $vis_store_type_uuid")
    println("    weight_type           = $wt_type_uuid")
    println("    weight_store_type     = $wt_store_type_uuid")

    # -- 6. Access visibility data (dispatch on actual stored type) -----------
    println("\n[6] Visibility access:")
    vis_entry = nothing
    for obj in objs
        if obj["shortname"] == "vis"
            vis_entry = obj
            break
        end
    end

    if isnothing(vis_entry)
        println("  No 'vis' object found in container store - skipping array access test.")
    else
        vis_uuid      = String(vis_entry["object_uuid"])
        stored_type   = get_object_type_uuid(cs, vis_uuid)

        # Dispatch to the correct typed getter based on the stored type UUID.
        # The on-disk format may use visibility_store_type (compressed) rather than
        # visibility_type, so we check both.
        vis = nothing
        vis_label = ""
        if stored_type == vis_type_uuid
            println("  Dispatching to get_visibility (visibility_type) ...")
            vis = get_visibility(cs, vis_uuid)
            vis_label = "visibility_type"
        elseif stored_type == vis_store_type_uuid
            println("  Dispatching to get_visibility_store (visibility_store_type) ...")
            vis = get_visibility_store(cs, vis_uuid)
            vis_label = "visibility_store_type"
        else
            println("  WARNING: unknown type UUID $stored_type for 'vis' - cannot dispatch.")
        end

        if vis === nothing || isnull(vis)
            println("  WARNING: getter returned null pointer for $vis_label.")
        else
            # get_array returns a flat 1-D Julia Array wrapping C++ memory (zero-copy).
            # get_dimensions returns C++ logical order [dim0(slowest)...dimN-1(fastest)].
            # Reversing and reshaping gives Julia column-major layout:
            #   arr[i_freq, i_time, i_channel, i_pol]  (Julia, 1-based)
            #   <->  arr[pol][channel][time][freq]         (C++, 0-based)
            # Do NOT resize!, push!, or append! — C++ owns the backing memory.
            cpp_dims = get_dimensions(vis)   # C++ logical order for reference
            arr = copy(reshape(get_array(vis), reverse(cpp_dims)...))
            println("  type              = $vis_label")
            println("  classname         = $(get_classname(vis))")
            println("  C++ dims (logical)= $cpp_dims")
            println("  Julia size        = $(size(arr))   # reversed")
            println("  ndims             = $(ndims(arr))")
            println("  eltype            = $(eltype(arr))")
            println("  total bytes       = $(sizeof(eltype(arr)) * length(arr))")

            # Direct N-dimensional indexing (no reshape required).
            println("  arr[1,1,1,1]      = $(arr[1,1,1,1])")

            # Axis 0 (pol-products) in C++ / axis RANK (last Julia dim).
            axis0_json = get_axis(vis, 0)
            println("  axis[0] (pol-products, C++ order): $(JSON3.read(axis0_json))")

            # Metadata
            meta = JSON3.read(get_metadata(vis))
            println("  metadata keys: $(collect(keys(meta)))")

            println("  OK Visibility ND array access successful.")
        end
    end

    # -- 7. Access weight data (dispatch on actual stored type) ---------------
    println("\n[7] Weight access:")
    wt_entry = nothing
    for obj in objs
        if obj["shortname"] == "weight"
            wt_entry = obj
            break
        end
    end

    if isnothing(wt_entry)
        println("  No 'weight' object found.")
    else
        wt_uuid     = String(wt_entry["object_uuid"])
        stored_type = get_object_type_uuid(cs, wt_uuid)
        wt = nothing
        if stored_type == wt_type_uuid
            wt = get_weight(cs, wt_uuid)
        elseif stored_type == wt_store_type_uuid
            wt = get_weight_store(cs, wt_uuid)
        else
            println("  WARNING: unknown type UUID $stored_type for 'weight'.")
        end
        if wt === nothing || isnull(wt)
            println("  WARNING: getter returned null pointer.")
        else
            wt_arr = get_array(wt)
            println("  weight size (Julia) = $(size(wt_arr))   # reversed from C++ $(get_dimensions(wt))  OK")
        end
    end

    # -- 8. Parameter store (if accessed via FringeData - available in operator context)
    println("\n[8] Root file data:")
    rootfile_json = get_rootfile_data(ss)
    root = JSON3.read(rootfile_json)
    println("  Root file keys: $(collect(keys(root)))")

    println("\n" * "=" ^ 60)
    println("Test complete.")
    println("=" ^ 60)
end

main()
