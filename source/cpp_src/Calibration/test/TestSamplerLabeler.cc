#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>

#include "MHO_ChannelLabeler.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_SamplerLabeler.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "TestSamplerLabeler";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    MHO_ChannelLabeler< visibility_type > label_maker;

    visibility_type vis;
    vis.Resize(4, 32, 4, 4);
    auto chan_axis_ptr = &(std::get< CHANNEL_AXIS >(vis));
    label_maker.SetArgs(&vis);
    label_maker.Initialize();
    label_maker.Execute();

    std::string key = "channel_label";
    for(std::size_t i = 0; i < chan_axis_ptr->GetSize(); i++)
    {
        std::string ch_label;
        mho_json ilabel = chan_axis_ptr->GetLabelObject(i);
        ch_label = ilabel[key].get< std::string >();
        std::cout << "channel: " << i << " default label: " << ch_label << std::endl;
    }

    //now lets build the sampler assignment
    std::vector< std::string > sampler_chan_set;
    sampler_chan_set.push_back("abcdefgh");
    sampler_chan_set.push_back("ijklmnop");
    sampler_chan_set.push_back("qrstuvwx");
    sampler_chan_set.push_back("yzABCDEF");

    std::vector< std::string > sampler_chan_set2;
    sampler_chan_set2.push_back("abcdefghijklmnop");
    sampler_chan_set2.push_back("qrstuvwxyzABCDEF");

    MHO_SamplerLabeler< visibility_type > sampler_indexer;
    sampler_indexer.SetReferenceStationSamplerChannelSets(sampler_chan_set);
    sampler_indexer.SetRemoteStationSamplerChannelSets(sampler_chan_set2);
    sampler_indexer.SetArgs(&vis);
    sampler_indexer.Initialize();
    sampler_indexer.Execute();

    std::string key2 = "ref_sampler_index";
    for(std::size_t i = 0; i < chan_axis_ptr->GetSize(); i++)
    {
        mho_json ilabel = chan_axis_ptr->GetLabelObject(i);
        if(ilabel.contains(key2))
        {
            int sampler_label = ilabel[key2].get< int >();
            std::cout << "channel: " << i << " ref_sampler_index: " << sampler_label << std::endl;
        }
    }

    key2 = "rem_sampler_index";
    for(std::size_t i = 0; i < chan_axis_ptr->GetSize(); i++)
    {
        mho_json ilabel = chan_axis_ptr->GetLabelObject(i);
        if(ilabel.contains(key2))
        {
            int sampler_label = ilabel[key2].get< int >();
            std::cout << "channel: " << i << " ref_sampler_index: " << sampler_label << std::endl;
        }
    }

    return 0;
}
