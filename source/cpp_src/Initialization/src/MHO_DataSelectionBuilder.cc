#include "MHO_DataSelectionBuilder.hh"
#include "MHO_SelectRepack.hh"

namespace hops
{

bool
MHO_DataSelectionBuilder::Build()
{
    if( IsConfigurationOk() )
    {
        msg_debug("initialization", "building data selection operators."<< eom);

        bool do_select_polprods = false;
        std::string polprod_key = "/config/polprod";
        std::string polprod = "";
        if(fParameterStore->IsPresent(polprod_key))
        {
            do_select_polprods = fParameterStore->Get(polprod_key, polprod);
        }

        bool do_select_chans = false;
        std::string select_chan_key = "/control/selection/freqs";
        std::vector< std::string > chans;
        if(fParameterStore->IsPresent(select_chan_key))
        {
            do_select_chans = fParameterStore->Get(select_chan_key, chans);
        }

        bool do_select_aps = false;
        std::string start_key = "/control/selection/start";
        std::string stop_key = "/control/selection/stop";
        int start = 0;
        int stop = 0;
        if(fParameterStore->IsPresent(start_key) || fParameterStore->IsPresent(stop_key) )
        {
            fParameterStore->Get(start_key, start);
            fParameterStore->Get(stop_key, stop);
            if(start != 0 || stop != 0){do_select_aps = true;}

            //negative values are seconds past scan start or before stop
            if(start > 0 || stop > 0) //in original fourfit positive values imply selection by minute past the last hour,
            {
                msg_error("initialization", "start/stop selection by minute past the hour is not yet supported." << eom);
                do_select_aps = false;
            }
        }

        if( !do_select_chans && !do_select_polprods && !do_select_aps)
        {
            msg_info("initialization", "no pol/freq data selection needed." << eom);
            return false;
        }

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject<visibility_type>(std::string("vis"));
        weight_type* wt_data = fContainerStore->GetObject<weight_type>(std::string("weight"));

        if( vis_data == nullptr || wt_data == nullptr )
        {
            msg_error("initialization", "cannot construct MHO_SelectRepack without visibility or weight data." << eom);
            return false;
        }

        ////////////////////////////////////////////////////////////////////////////
        //APPLY COARSE DATA SELECTION
        ////////////////////////////////////////////////////////////////////////////
        //select data repack
        auto spack = new MHO_SelectRepack<visibility_type>();
        auto wtspack = new MHO_SelectRepack<weight_type>();

        //first find indexes which corresponds to the specified pol product
        if(do_select_polprods)
        {
            std::vector<std::size_t> selected_pp;
            msg_debug("initialization", "data selection, selecting pol-product = "<< polprod << eom);
            selected_pp = (&(std::get<POLPROD_AXIS>(*vis_data)))->SelectMatchingIndexes(polprod);
            spack->SelectAxisItems(POLPROD_AXIS,selected_pp);
            wtspack->SelectAxisItems(POLPROD_AXIS,selected_pp);
        }

        if(do_select_chans)
        {
            std::set< std::string > chan_set;
            for(auto it = chans.begin(); it != chans.end(); it++){chan_set.insert(*it);}
            std::string chan_label_key = "channel_label";
            std::vector<std::size_t> selected_ch;

            for(auto it = chan_set.begin(); it != chan_set.end(); it++)
            {
                 auto tmp_ch = (&(std::get<CHANNEL_AXIS>(*vis_data)))->GetMatchingIndexes(chan_label_key, *it);
                 selected_ch.insert(selected_ch.end(), tmp_ch.begin(), tmp_ch.end() );
            }

            msg_debug("initialization", "data selection, selecting "<<selected_ch.size() << " channels." << eom);
            spack->SelectAxisItems(CHANNEL_AXIS,selected_ch);
            wtspack->SelectAxisItems(CHANNEL_AXIS,selected_ch);
        }

        if(do_select_aps)
        {
            std::vector<std::size_t> selected_aps;
            msg_debug("initialization", "data selection, selecting pol-product = "<< polprod << eom);
            auto ap_ax_ptr = &(std::get<TIME_AXIS>(*vis_data));
            std::size_t naps = ap_ax_ptr->GetSize();
            double first_t = ap_ax_ptr->at(0);
            double last_t = ap_ax_ptr->at(naps-1);
            //TODO may want to clean up with selection process
            #pragma message("The stop/start parameters are passed as integers (n seconds), which works fine for 1 sec APs, but for smaller APs maybe we should pass them as floats?")
            for(std::size_t i=0; i<naps; i++)
            {
                double t = (*ap_ax_ptr)(i);
                //std::cout<<" t, stop, start, begin, end = "<< t <<", "<< (last_t + stop)<<", "<< (first_t - start)<<", " << first_t<<", "<<last_t<< std::endl;
                if( t <= (last_t + (double)stop) && t >= (first_t - (double)start) ){selected_aps.push_back(i);}
            }

            spack->SelectAxisItems(TIME_AXIS,selected_aps);
            wtspack->SelectAxisItems(TIME_AXIS,selected_aps);
        }

        spack->SetArgs(vis_data);
        wtspack->SetArgs(wt_data);

        std::string op_name = "coarse_selection";
        std::string op_category = "selection";
        bool replace_duplicates = true;
        #pragma message("TODO - figure out proper naming/retrieval scheme for operators")

        #pragma message("TODO - coarse selection must also be applied to pcal data (particularly AP select) if available!!")

        spack->SetName(op_name + ":vis");
        wtspack->SetName(op_name + ":weight");

        fOperatorToolbox->AddOperator(spack, spack->GetName(), op_category, replace_duplicates);
        fOperatorToolbox->AddOperator(wtspack, wtspack->GetName(), op_category, replace_duplicates);

        return true;
    }
    return false;
}

}//end namespace
