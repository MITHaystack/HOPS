#include "MHO_DataSelectionBuilder.hh"
#include "MHO_SelectRepack.hh"

namespace hops
{

bool MHO_DataSelectionBuilder::Build()
{
    if(IsConfigurationOk())
    {
        msg_debug("initialization", "building data selection operators." << eom);

        bool do_select_fgroup = false;
        std::string fgroup_key = "/config/fgroup";
        std::string fgroup = "";
        if(fParameterStore->IsPresent(fgroup_key))
        {
            fParameterStore->Get(fgroup_key, fgroup);
            if(fgroup != "")
            {
                do_select_fgroup = true;
                msg_debug("initialization", "will select data by frequency group: " << fgroup << eom);
            }
        }

        std::string polprod_key = "/config/polprod";
        std::string polprod = "";
        if(fParameterStore->IsPresent(polprod_key))
        {
            fParameterStore->Get(polprod_key, polprod);
        }

        bool do_select_polprods = false;
        std::string polprod_set_key = "/config/polprod_set";
        std::vector< std::string > pp_vec;
        if(fParameterStore->IsPresent(polprod_set_key))
        {
            do_select_polprods = fParameterStore->Get(polprod_set_key, pp_vec);

            if(do_select_polprods)
            {
                std::stringstream ss;
                for(std::size_t i = 0; i < pp_vec.size(); i++)
                {
                    ss << pp_vec[i];
                    if(i != pp_vec.size() - 1)
                    {
                        ss << ", ";
                    }
                }
                msg_debug("initialization", "will select data by pol-products: {" << ss.str() << "}." << eom);
            }
        }

        bool do_select_chans = false;
        std::string select_chan_key = "/control/selection/freqs";
        std::vector< std::string > chans;
        if(fParameterStore->IsPresent(select_chan_key))
        {
            do_select_chans = fParameterStore->Get(select_chan_key, chans);
            if(do_select_chans)
            {
                std::stringstream ss;
                for(std::size_t i = 0; i < chans.size(); i++)
                {
                    ss << chans[i];
                    if(i != chans.size() - 1)
                    {
                        ss << ", ";
                    }
                }
                msg_debug("initialization", "will select data by channels: " << ss.str() << "." << eom);
            }
        }

        bool do_select_aps = false;
        std::string start_key = "/control/selection/start";
        std::string stop_key = "/control/selection/stop";
        int start = 0;
        int stop = 0;
        if(fParameterStore->IsPresent(start_key) || fParameterStore->IsPresent(stop_key))
        {
            fParameterStore->Get(start_key, start);
            fParameterStore->Get(stop_key, stop);
            if(start != 0 || stop != 0)
            {
                do_select_aps = true;
            }

            if(do_select_aps)
            {
                msg_debug("initialization",
                          "will select data by AP, start offset: " << start << " and stop offset: " << stop << "." << eom);
            }

            //negative values are seconds past scan start or before stop
            if(start > 0 || stop > 0) //in original fourfit positive values imply selection by minute past the last hour,
            {
                msg_error("initialization", "start/stop selection by minute past the hour is not yet supported." << eom);
                do_select_aps = false;
            }
        }

        if(!do_select_chans && !do_select_polprods && !do_select_aps && !do_select_fgroup)
        {
            msg_info("initialization", "no pol/freq data selection needed." << eom);
            return false;
        }

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        weight_type* wt_data = fContainerStore->GetObject< weight_type >(std::string("weight"));

        if(vis_data == nullptr || wt_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_SelectRepack without visibility or weight data." << eom);
            return false;
        }

        ////////////////////////////////////////////////////////////////////////////
        //APPLY COARSE DATA SELECTION
        ////////////////////////////////////////////////////////////////////////////
        //select data repack
        auto spack = new MHO_SelectRepack< visibility_type >();
        auto wtspack = new MHO_SelectRepack< weight_type >();

        //first find indexes which corresponds to the specified pol product
        if(do_select_polprods)
        {
            std::set< std::string > pp_set;
            std::vector< std::size_t > selected_pp;
            pp_set.insert(pp_vec.begin(), pp_vec.end());

            std::stringstream ss;
            for(std::size_t i = 0; i < pp_vec.size(); i++)
            {
                ss << pp_vec[i];
                if(i != pp_vec.size() - 1)
                {
                    ss << ", ";
                }
            }
            msg_debug("initialization", "data selection, selecting for pol-product set: {" << ss.str() << "}." << eom);

            selected_pp = (&(std::get< POLPROD_AXIS >(*vis_data)))->SelectMatchingIndexes(pp_set);
            if(selected_pp.size() == 0)
            {
                msg_warn("initialization", "pol-product selection failed to match any data." << eom);
            }
            spack->SelectAxisItems(POLPROD_AXIS, selected_pp);
            wtspack->SelectAxisItems(POLPROD_AXIS, selected_pp);
        }

        std::vector< std::size_t > fgroup_idx;
        if(do_select_fgroup)
        {
            //get all of channels in this frequency group
            std::string fgroup_label_key = "frequency_band";
            fgroup_idx = (&(std::get< CHANNEL_AXIS >(*vis_data)))->GetMatchingIndexes(fgroup_label_key, fgroup);
            if(fgroup_idx.size() == 0)
            {
                msg_warn("initialization",
                         "frequency band/group selection by " << fgroup << ", failed to match any data." << eom);
            }
        }

        std::vector< std::size_t > selected_ch;
        if(do_select_chans)
        {
            std::set< std::string > chan_set; //set of channels selected by control
            for(auto it = chans.begin(); it != chans.end(); it++)
            {
                chan_set.insert(*it);
            }
            std::string chan_label_key = "channel_label";

            for(auto it = chan_set.begin(); it != chan_set.end(); it++)
            {
                auto tmp_ch = (&(std::get< CHANNEL_AXIS >(*vis_data)))->GetMatchingIndexes(chan_label_key, *it);
                selected_ch.insert(selected_ch.end(), tmp_ch.begin(), tmp_ch.end());
            }

            msg_debug("initialization", "data selection, selecting " << selected_ch.size() << " channels." << eom);
            if(selected_ch.size() == 0)
            {
                msg_warn("initialization", "channel selection failed to match any data." << eom);
            }
        }

        //figure out the channel selection
        std::vector< std::size_t > channel_selection;
        if(do_select_chans && !do_select_fgroup)
        {
            channel_selection = selected_ch; //only channel selection
        }

        if(do_select_fgroup && !do_select_chans)
        {
            channel_selection = fgroup_idx; //only freq group selection
        }

        //both, so figure out the intersection of the frequency group
        //selection and the channel group selection
        if(do_select_chans && do_select_fgroup)
        {
            channel_selection.clear();
            //dumb brute force O(N^2) union
            for(auto fit = fgroup_idx.begin(); fit != fgroup_idx.end(); fit++)
            {
                bool include = false;
                for(auto cit = selected_ch.begin(); cit != selected_ch.end(); cit++)
                {
                    if(*fit == *cit)
                    {
                        include = true;
                        break;
                    }
                }
                if(include)
                {
                    channel_selection.push_back(*fit);
                }
            }
            std::sort(channel_selection.begin(), channel_selection.end());
        }

        //finally assign the selected channels
        if(do_select_fgroup || do_select_chans)
        {
            spack->SelectAxisItems(CHANNEL_AXIS, channel_selection);
            wtspack->SelectAxisItems(CHANNEL_AXIS, channel_selection);
        }

        if(do_select_aps)
        {
            std::vector< std::size_t > selected_aps;
            auto ap_ax_ptr = &(std::get< TIME_AXIS >(*vis_data));
            std::size_t naps = ap_ax_ptr->GetSize();
            double first_t = ap_ax_ptr->at(0);
            double last_t = ap_ax_ptr->at(naps - 1);
            //Note: The stop/start parameters are passed as integers (n seconds), 
            //which works fine for 1 sec APs, but
            //for smaller APs maybe we should pass them as floats?
            for(std::size_t i = 0; i < naps; i++)
            {
                double t = (*ap_ax_ptr)(i);
                //std::cout<<" t, stop, start, begin, end = "<< t <<", "<< (last_t + stop)<<", "<< (first_t - start)<<", " << first_t<<", "<<last_t<< std::endl;
                if(t <= (last_t + (double)stop) && t >= (first_t - (double)start))
                {
                    selected_aps.push_back(i);
                }
            }

            msg_debug("initialization", "data selection, selecting " << selected_aps.size() << " APs." << eom);

            if(selected_aps.size() == 0)
            {
                msg_warn("initialization", "AP selection eliminated all data." << eom);
            }

            spack->SelectAxisItems(TIME_AXIS, selected_aps);
            wtspack->SelectAxisItems(TIME_AXIS, selected_aps);
        }

        spack->SetArgs(vis_data);
        wtspack->SetArgs(wt_data);

        std::string op_name = "coarse_selection";
        std::string op_category = "selection";
        bool replace_duplicates = true;

        //coarse time selection must also be applied to pcal data...this is done in MHO_PhaseCalibrationTrim

        double priority = fFormat["priority"].get< double >();
        spack->SetName(op_name + ":vis");
        wtspack->SetName(op_name + ":weight");
        spack->SetPriority(priority);
        wtspack->SetPriority(priority);

        fOperatorToolbox->AddOperator(spack, spack->GetName(), op_category, replace_duplicates);
        fOperatorToolbox->AddOperator(wtspack, wtspack->GetName(), op_category, replace_duplicates);

        return true;
    }
    return false;
}

} // namespace hops
