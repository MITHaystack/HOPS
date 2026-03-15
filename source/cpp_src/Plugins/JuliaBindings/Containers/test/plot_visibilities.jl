#!/usr/bin/env julia
# plot_visibilities.jl
#
# Plot visibility amplitude as a 3D surface (freq x time x |vis|) for each
# pol-product / channel combination found in a HOPS4 scan directory.
#
# Usage:
#   julia plot_visibilities.jl <scan-directory> [baseline]
#
# Example:
#   julia plot_visibilities.jl /data/vt9105/1111/105-1800 GE
#
# Requires HOPS_JULIA_MODULES_DIR to be set (source hopsenv.sh first).
# Requires: CxxWrap, JSON3, Plots (all must be installed in the active Julia env)
# Backend: gr() is used by default (install GR); set env JULIA_PLOTS_BACKEND=plotlyjs
# to use PlotlyJS instead.

using CxxWrap
using JSON3
using Plots

# -- Library path from environment ------------------------------------------
const JLMHO_CONTAINERS_LIB = joinpath(ENV["HOPS_JULIA_MODULES_DIR"], "libjlMHO_Containers.so")

@wrapmodule(() -> JLMHO_CONTAINERS_LIB)
function __init__()
    @initcxx
end

# -- Helpers -----------------------------------------------------------------

"""Return the first object entry matching `shortname`, or nothing."""
function find_object(objs, shortname::String)
    for obj in objs
        if obj["shortname"] == shortname
            return obj
        end
    end
    return nothing
end

"""
    load_vis(cs) -> (vis_obj, label) or (nothing, "")

Retrieve the visibility (or visibility_store) object from a ContainerStore,
dispatching on the runtime type UUID.
"""
function load_vis(cs)
    objs = JSON3.read(get_object_id_list(cs))
    vis_type_uuid       = get_visibility_type_uuid(cs)
    vis_store_type_uuid = get_visibility_store_type_uuid(cs)

    entry = find_object(objs, "vis")
    entry === nothing && return (nothing, "")

    vis_uuid    = String(entry["object_uuid"])
    stored_type = get_object_type_uuid(cs, vis_uuid)

    if stored_type == vis_type_uuid
        return (get_visibility(cs, vis_uuid), "visibility")
    elseif stored_type == vis_store_type_uuid
        return (get_visibility_store(cs, vis_uuid), "visibility_store")
    else
        @warn "Unknown type UUID $stored_type for 'vis' object"
        return (nothing, "")
    end
end

"""
    plot_vis_surfaces(vis_obj, baseline, scan_dir)

Create one surface plot per (pol-product, channel) pair, showing visibility
amplitude as a function of frequency lag (x) and accumulation-period index (y).

Julia array layout (from get_array): arr[i_freq, i_time, i_channel, i_pol]
C++ logical layout:                  arr[pol][channel][time][freq]
"""
function plot_vis_surfaces(vis_obj, baseline::String, scan_dir::String)
    # get_array returns a flat 1-D view of C++ memory.
    # get_dimensions returns C++ logical order [n_pol, n_chan, n_time, n_freq].
    # Reversing gives Julia column-major order.
    # copy() is required: CxxWrap declares the return type as Any, so reshape
    # produces a ReshapedArray{T,N,Any,...} whose AbstractArray element-dispatch
    # path misbehaves under Julia 1.12's interpreter.  copy() produces a concrete
    # Array{T,N} with fast native indexing for all subsequent slices.
    raw  = get_array(vis_obj)
    arr  = copy(reshape(raw, reverse(get_dimensions(vis_obj))...))  # (n_freq, n_time, n_chan, n_pol)
    sz   = size(arr)
    n_freq, n_time, n_chan, n_pol = sz
    println("  Array size (Julia): $sz  -> n_freq=$n_freq  n_time=$n_time  n_chan=$n_chan  n_pol=$n_pol")

    # ---- Axis coordinate arrays (C++ axis index order) ----------------------
    # C++ axis 0 = pol products  -> Julia dim 4
    # C++ axis 2 = AP times (s)  -> Julia dim 2
    # C++ axis 3 = freq lags     -> Julia dim 1
    pol_products = JSON3.read(get_axis(vis_obj, 0))   # e.g. ["XX","XY","YX","YY"]
    ap_times     = Float64.(JSON3.read(get_axis(vis_obj, 2)))  # seconds
    freq_lags    = Float64.(JSON3.read(get_axis(vis_obj, 3)))  # lag indices or Hz

    println("  Pol products : $pol_products")
    println("  AP times     : $(length(ap_times)) points  [$(ap_times[1]) ... $(ap_times[end])] s")
    println("  Freq lags    : $(length(freq_lags)) points")

    # Choose backend
    backend_name = get(ENV, "JULIA_PLOTS_BACKEND", "gr")
    if backend_name == "plotlyjs"
        plotlyjs()
    else
        gr()
    end

    plots_made = 0

    for pp in 1:n_pol
        pp_label = length(pol_products) >= pp ? String(pol_products[pp]) : "pol$pp"

        for ch in 1:n_chan
            # Slice: amplitude over (freq_lag, ap_time) for this pol/channel
            # arr[i_freq, i_time, i_channel, i_pol]
            amp = abs.(arr[:, :, ch, pp])   # (n_freq x n_time) matrix

            title_str = "Visibility Amplitude | $baseline  pol=$pp_label  ch=$ch"

            p = surface(
                ap_times,           # x-axis: accumulation period start times (s)
                freq_lags,          # y-axis: frequency lag
                amp;                # z-axis: |vis| - matrix[i_freq, i_time]
                xlabel = "AP time (s)",
                ylabel = "Freq lag",
                zlabel = "|Visibility|",
                title  = title_str,
                legend = false,
                color  = :viridis,
            )

            outfile = "vis_$(baseline)_$(pp_label)_ch$(ch).png"
            savefig(p, outfile)
            println("  Saved: $outfile")
            plots_made += 1
        end
    end

    println("  Total plots saved: $plots_made")
end

# -- Main --------------------------------------------------------------------

function main()
    if length(ARGS) < 1
        println(stderr, "Usage: julia plot_visibilities.jl <scan-directory> [baseline]")
        exit(1)
    end

    scan_dir     = ARGS[1]
    requested_bl = length(ARGS) >= 2 ? ARGS[2] : ""

    println("=" ^ 60)
    println("HOPS Visibility Plot")
    println("=" ^ 60)
    println("Library : $JLMHO_CONTAINERS_LIB")
    println("Scan dir: $scan_dir")

    # Initialise scan store
    ss = MHO_JlScanStoreInterface()
    set_directory(ss, scan_dir)
    ok = initialize(ss)
    if !ok
        println(stderr, "ERROR: initialize() failed for '$scan_dir'")
        exit(1)
    end

    baselines = get_baseline_list(ss)
    println("Baselines: $baselines")
    isempty(baselines) && (println(stderr, "ERROR: no baselines found"); exit(1))

    bl = isempty(requested_bl) ? baselines[1] : requested_bl
    if !has_baseline(ss, bl)
        println(stderr, "ERROR: baseline '$bl' not found. Available: $baselines")
        exit(1)
    end
    println("Using baseline: $bl")

    rc = load_baseline(ss, bl)
    rc < 0 && (println(stderr, "ERROR: load_baseline failed (rc=$rc)"); exit(1))

    cs = get_baseline_data(ss, bl)
    println("ContainerStore: is_valid=$(is_valid(cs))  nobjects=$(get_nobjects(cs))")

    vis_obj, label = load_vis(cs)
    if vis_obj === nothing || isnull(vis_obj)
        println(stderr, "ERROR: no visibility object found in container store")
        exit(1)
    end
    println("Loaded visibility object (type=$label)")

    plot_vis_surfaces(vis_obj, bl, scan_dir)

    println("=" ^ 60)
    println("Done.")
    println("=" ^ 60)
end

main()
