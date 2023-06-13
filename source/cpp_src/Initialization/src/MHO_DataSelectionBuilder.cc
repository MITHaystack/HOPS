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

        //currently this operator only does coarse data selection on pol-product
        //and channels, it does not do start/stop AP selction (but it should)

        bool do_select_polprods = false;
        std::string polprod_key = "selected_polprod";
        std::string polprod = ""; 
        if(fParameterStore->IsPresent(polprod_key))
        {
            do_select_polprods = fParameterStore->Get(polprod_key, polprod);
        }

        bool do_select_chans = false;
        std::string select_chan_key = "freqs";
        std::vector< std::string > chans;
        if(fParameterStore->IsPresent(select_chan_key))
        {
            do_select_chans = fParameterStore->Get(select_chan_key, chans);
        }

        if( !do_select_chans && !do_select_polprods)
        {
            msg_info("initialization", "no data selection needed." << eom);
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
            msg_debug("initialization", "data selection, selecting "<<chan_set.size() << " channels." << eom);
            //TODO, this channel label -> index selection method is pretty crude could be optimized
            std::string chan_label_key = "channel_label";
            std::set< std::size_t > index_set;
            std::vector<std::size_t> selected_ch;
            for(auto it = chan_set.begin(); it!= chan_set.end(); it++)
            {
                std::vector< std::size_t > tmp_idx = (&(std::get<CHANNEL_AXIS>(*vis_data)))->SelectMatchingIndexesByLabelValue(chan_label_key, *it);
                std::copy(tmp_idx.begin(), tmp_idx.end(), std::inserter(index_set,index_set.end() ) );
            }
            std::copy(index_set.begin(), index_set.end(), std::inserter(selected_ch,selected_ch.end() ) );

            msg_debug("initialization", "data selection, selecting "<<selected_ch.size() << " channels." << eom);
            spack->SelectAxisItems(CHANNEL_AXIS,selected_ch);
            wtspack->SelectAxisItems(CHANNEL_AXIS,selected_ch);
        }

        //TODO FIXME implement start/stop AP selection

        spack->SetArgs(vis_data);
        wtspack->SetArgs(wt_data);

        std::string op_name = "coarse_selection";
        bool replace_duplicates = true;
        #pragma message("TODO - figure out proper naming/retrieval scheme for operators")
        fOperatorToolbox->AddOperator(spack, op_name + ":vis", replace_duplicates);
        fOperatorToolbox->AddOperator(wtspack, op_name + ":weight", replace_duplicates);
    }
    return false;
}

}//end namespace
