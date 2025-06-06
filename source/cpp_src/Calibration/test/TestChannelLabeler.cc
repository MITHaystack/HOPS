#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>

#include "MHO_ChannelLabeler.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "TestChannelLabeler";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    MHO_ChannelLabeler< visibility_type > label_maker;

    visibility_type vis;
    vis.Resize(4, 128, 16, 16);

    //provide some fake frequencies, and construct a 'user supplied ' label -> freq map
    std::map< std::string, double > fUserMap;
    std::string fake_charset = "ABCDEFGH";
    auto chan_axis_ptr = &(std::get< CHANNEL_AXIS >(vis));
    for(std::size_t i = 0; i < chan_axis_ptr->GetSize(); i++)
    {
        double freq = 1001.4 * i;
        chan_axis_ptr->at(i) = freq;
        std::string tmp_label = encode_value(i, fake_charset);
        fUserMap[tmp_label] = freq;
    }

    std::string key = "channel_label";
    label_maker.SetArgs(&vis);
    label_maker.Initialize();
    label_maker.Execute();

    for(std::size_t i = 0; i < chan_axis_ptr->GetSize(); i++)
    {
        std::string ch_label;
        mho_json ilabel = chan_axis_ptr->GetLabelObject(i);
        ch_label = ilabel[key].get< std::string >();
        std::cout << "channel: " << i << " default label: " << ch_label << std::endl;
    }

    std::size_t ch_size = chan_axis_ptr->GetSize();

    label_maker.SetChannelLabelToFrequencyMap(fUserMap);
    label_maker.SetArgs(&vis);
    label_maker.Initialize();
    label_maker.Execute();

    for(std::size_t i = 0; i < chan_axis_ptr->GetSize(); i++)
    {
        std::string ch_label;
        mho_json ilabel = chan_axis_ptr->GetLabelObject(i);
        ch_label = ilabel[key].get< std::string >();
        std::cout << "channel: " << i << " user label: " << ch_label << std::endl;
    }

    return 0;
}
