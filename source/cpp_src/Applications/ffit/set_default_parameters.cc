#include "ffit.hh"

void set_default_parameters(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    //set the selected pol-product
    std::string polprod = paramStore->GetAs<std::string>("/cmdline/polprod");
    paramStore->Set("selected_polprod", polprod);

    //grab the visibility data, so we can determine the default reference frequency
    visibility_type* vis_data = conStore->GetObject<visibility_type>(std::string("vis"));

    //the first frequency in the array serves as the reference frequency if this value remains unset in the control file
    double first_freq = std::get<CHANNEL_AXIS>(*vis_data)(0);
    paramStore->Set("ref_freq", first_freq);

    //default mbd_anchor is model (instead of sbd)
    std::string mbd_anchor = "model";
    paramStore->Set("mbd_anchor", mbd_anchor);
};
