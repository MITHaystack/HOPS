#include <string>
#include <iostream>


#include "MHO_Message.hh"
#include "MHO_ChannelLabellerBuilder.hh"
#include "MHO_ChannelLabeller.hh"

using namespace hops;

// {
//     "name": "chan_ids",
//     "statement_type": "parameter",
//     "type" : "compound",
//     "parameters":
//     {
//         "channel_names": {"type": "string"},
//         "channel_frequencies": {"type": "list_real"}
//     },
//     "fields":
//     [
//         "channel_names",
//         "channel_frequencies"
//     ]
// }


int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string chan_ids = "abcdefghijklmnopqrstuvwxyzABCDEF";
    std::vector< double > freqs =
    {
        215979.203125, 215920.609375, 215862.015625, 215803.421875,
        215744.828125, 215686.234375, 215627.640625, 215569.046875,
        215510.453125, 215451.859375, 215393.265625, 215334.671875,
        215276.078125, 215217.484375, 215158.890625, 215100.296875,
        215041.703125, 214983.109375, 214924.515625, 214865.921875,
        214807.328125, 214748.734375, 214690.140625, 214631.546875,
        214572.953125, 214514.359375, 214455.765625, 214397.171875,
        214338.578125, 214279.984375, 214221.390625, 214162.796875
    };

    MHO_OperatorToolbox toolbox;
    MHO_ContainerStore store;

    mho_json attrib;

    attrib["name"] = "chan_ids";
    attrib["channel_names"] = chan_ids;
    attrib["channel_frequencies"] = freqs;

    MHO_ChannelLabellerBuilder builder(&toolbox, &store);
    builder.SetAttributes(attrib);

    bool ok = builder.Build();
    std::string name = "chan_ids";
    auto op = toolbox.GetOperatorAs< MHO_ChannelLabeller<visibility_type> >(name);

    if(op != nullptr)
    {
        visibility_type vis;
        vis.Resize(4, 32, 2, 2);
        auto chan_axis_ptr = &(std::get<CHANNEL_AXIS>(vis));

        for(std::size_t i=0; i<chan_axis_ptr->GetSize(); i++)
        {
            chan_axis_ptr->at(i) = freqs[32 - 1 - i]; //reverse labeling to make this interesting
        }

        std::string key = "channel_label";
        op->SetArgs(&vis);
        op->Initialize();
        op->Execute();

        std::cout << std::setprecision(12);
        for(std::size_t i=0; i<chan_axis_ptr->GetSize(); i++)
        {
            std::string ch_label;
            std::vector< MHO_IntervalLabel* > iLabels;
            iLabels.clear();
            iLabels = chan_axis_ptr->GetIntervalsWhichIntersect(i);
            if(iLabels.size() == 1)
            {
                iLabels[0]->Retrieve(key, ch_label);
                std::cout<<"channel: "<<i<<" user label: "<<ch_label<<" freq: "<< chan_axis_ptr->at(i) <<std::endl;
            }
            else{std::cout<<"unlabelled channel present"<<std::endl;}
        }

        return 0;
    }

    //error
    return 1;
}
