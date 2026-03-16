#!/usr/bin/env julia
# example_jl_operator.jl
#
# Example Julia operator function for use with MHO_JlGenericOperator.
#
# To wire this into the C++ pipeline, set in the control file:
#   "module_path":   "<install_dir>/example_jl_operator.jl"
#   "function_name": "example_jl_operator"
#
# The function is called with one argument (the fringe data interface) by
# MHO_JlGenericOperator::Execute() and must return a Bool (true = success).

using CxxWrap   # for isnull()
using JSON3

"""
Example operator: prints a summary of the current baseline's data and returns true.

Called by MHO_JlGenericOperator::Execute() with the fringe data interface as
the sole argument.  Must return a Bool (true = success, false = failure).
"""
function example_jl_operator(fd)

    # --- Parameter store access ---
    ps = get_parameter_store(fd)
    ref_station = get_by_path(ps, "/ref_station/mk4id")
    rem_station = get_by_path(ps, "/rem_station/mk4id")
    println("[jl_operator] baseline: $ref_station-$rem_station")

    # --- Container store: list objects ---
    cs = get_container_store(fd)
    println("[jl_operator] container store valid=$(is_valid(cs))  nobjects=$(get_nobjects(cs))")

    # --- Visibility array (dispatch on stored type UUID) ---
    vis_type_uuid       = get_visibility_type_uuid(cs)
    vis_store_type_uuid = get_visibility_store_type_uuid(cs)

    objs = JSON3.read(get_object_id_list(cs))
    for obj in objs
        if obj["shortname"] == "vis"
            vis_uuid    = String(obj["object_uuid"])
            stored_type = get_object_type_uuid(cs, vis_uuid)

            vis = if stored_type == vis_type_uuid
                get_visibility(cs, vis_uuid)
            elseif stored_type == vis_store_type_uuid
                get_visibility_store(cs, vis_uuid)
            else
                nothing
            end

            if vis !== nothing && !isnull(vis)
                arr = get_array(vis)
                println("[jl_operator] visibility size (Julia col-major) = $(size(arr))")
                println("[jl_operator] arr[1,1,1,1] = $(arr[1,1,1,1])")
            end
            break
        end
    end

    return true
end
