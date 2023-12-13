#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_ChannelLabeller.hh"
#include "MHO_SamplerLabeller.hh"
#include "MHO_ContainerDefinitions.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "TestSamplerLabeller";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    MHO_ChannelLabeller<visibility_type> label_maker;
    
    visibility_type vis;
    vis.Resize(4, 32, 4, 4);
    auto chan_axis_ptr = &(std::get<CHANNEL_AXIS>(vis));
    label_maker.SetArgs(&vis);
    label_maker.Initialize();
    label_maker.Execute();

    std::string key = "channel_label";
    for(std::size_t i=0; i<chan_axis_ptr->GetSize(); i++)
    {
        std::string ch_label;
        std::vector< MHO_IntervalLabel > iLabels = chan_axis_ptr->GetIntervalsWhichIntersect(i);
        iLabels[0].Retrieve(key, ch_label);
        std::cout<<"channel: "<<i<<" default label: "<<ch_label<<std::endl;
    }

    //now lets build the sampler assignment
    std::vector< std::string > sampler_chan_set;
    sampler_chan_set.push_back("abcdefgh");
    sampler_chan_set.push_back("ijklmnop");
    sampler_chan_set.push_back("qrstuvwx");
    sampler_chan_set.push_back("yzABCDEF");
    
    MHO_SamplerLabeller<visibility_type> sampler_indexer;
    sampler_indexer.SetSamplerChannelSets(sampler_chan_set);
    sampler_indexer.SetArgs(&vis);
    sampler_indexer.Initialize();
    sampler_indexer.Execute();

    std::string key2 = "sampler_index";
    for(std::size_t i=0; i<chan_axis_ptr->GetSize(); i++)
    {
        std::string ch_label;
        std::vector< MHO_IntervalLabel > iLabels = chan_axis_ptr->GetIntervalsWhichIntersect(i);
        for(auto iter = iLabels.begin(); iter != iLabels.end(); iter++)
        {
            if (iter->ContainsKey<int>(key2))
            {
                int sampler_label;
                iter->Retrieve(key2, sampler_label);
                std::cout<<"channel: "<<i<<" sampler_index: "<<sampler_label<<std::endl;
            }
        }
    }
    

    return 0;
}
