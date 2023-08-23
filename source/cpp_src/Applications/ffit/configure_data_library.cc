#include "ffit.hh"

void configure_data_library(MHO_ContainerStore* store)
{
    //retrieve the (first) visibility and weight objects
    //(currently assuming there is only one object per type)
    visibility_store_type* vis_store_data = nullptr;
    weight_store_type* wt_store_data = nullptr;

    vis_store_data = store->GetObject<visibility_store_type>(0);
    wt_store_data = store->GetObject<weight_store_type>(0);

    if(vis_store_data == nullptr)
    {
        msg_fatal("main", "failed to read visibility data from the .cor file." <<eom);
        std::exit(1);
    }

    if(wt_store_data == nullptr)
    {
        msg_fatal("main", "failed to read weight data from the .cor file." <<eom);
        std::exit(1);
    }

    std::size_t n_vis = store->GetNObjects<visibility_store_type>();
    std::size_t n_wt = store->GetNObjects<weight_store_type>();

    if(n_vis != 1 || n_wt != 1)
    {
        msg_warn("main", "multiple visibility and/or weight types not yet supported" << eom);
    }

    std::string vis_shortname = store->GetShortName(vis_store_data->GetObjectUUID() );
    std::string wt_shortname = store->GetShortName(wt_store_data->GetObjectUUID() );

    visibility_type* vis_data = new visibility_type();
    weight_type* wt_data = new weight_type();

    MHO_ElementTypeCaster<visibility_store_type, visibility_type> up_caster;
    up_caster.SetArgs(vis_store_data, vis_data);
    up_caster.Initialize();
    up_caster.Execute();

    MHO_ElementTypeCaster< weight_store_type, weight_type> wt_up_caster;
    wt_up_caster.SetArgs(wt_store_data, wt_data);
    wt_up_caster.Initialize();
    wt_up_caster.Execute();

    //remove the original objects
    store->DeleteObject(vis_store_data);
    store->DeleteObject(wt_store_data);

    #pragma message("TODO - if we plan to rely on short-names to identify objects, we need to validate them here")
    //TODO make sure that the visibility object is called 'vis' and weights are called 'weights', etc.
    //TODO also validate the station data

    //now shove the double precision data into the container store with the same shortname
    store->AddObject(vis_data);
    store->AddObject(wt_data);
    store->SetShortName(vis_data->GetObjectUUID(), vis_shortname);
    store->SetShortName(wt_data->GetObjectUUID(), wt_shortname);
}
