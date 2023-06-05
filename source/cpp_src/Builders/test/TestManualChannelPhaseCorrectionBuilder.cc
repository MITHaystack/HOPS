#include <string>
#include <iostream>


#include "MHO_Message.hh"

#include "MHO_ContainerDefinitions.hh"

#include "MHO_ChannelLabeller.hh"
#include "MHO_ManualChannelPhaseCorrection.hh"
#include "MHO_ManualChannelPhaseCorrectionBuilder.hh"

using namespace hops;

// {
//     "name": "pc_phases_x",
//     "statement_type": "parameter",
//     "type" : "compound",
//     "parameters":
//     {
//         "channel_names": {"type": "string"},
//         "pc_phases": {"type": "list_real"}
//     },
//     "fields": 
//     [
//         "channel_names", 
//         "pc_phases"
//     ]
// }

int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);
    
    MHO_OperatorToolbox toolbox;
    MHO_ContainerStore store;
    
    //construct some trivial visibilities
    visibility_type vis;
    vis.Insert( std::string("reference_station_mk4id"), std::string("E") );
    vis.Resize(1, 32, 1, 1);
    
    //add a default channel labeller
    MHO_ChannelLabeller<visibility_type> ch_labeller;
    ch_labeller.SetArgs(&vis);
    ch_labeller.Initialize();
    ch_labeller.Execute();

    mho_json conditions;
    conditions["name"] = "if";
    conditions["value"].push_back("station");
    conditions["value"].push_back("E");
    
    mho_json attrib;
    std::string chan_ids = "abcdefghijklmnopqrstuvwxyzABCDEF";
    std::vector< double > pc_phases;
    for(std::size_t i=0; i<chan_ids.size(); i++){pc_phases.push_back(90.0*(i%4) );}

    attrib["name"] = "pc_phases_x";
    attrib["channel_names"] = chan_ids;
    attrib["pc_phases"] = pc_phases;

    MHO_ManualChannelPhaseCorrectionBuilder builder(&toolbox, &store);
    builder.SetAttributes(attrib);
    builder.SetConditions(conditions);

    bool ok = builder.Build();
    std::string name = "pc_phases";
    auto op = toolbox.GetOperatorAs< MHO_ManualChannelPhaseCorrection >(name);

    if(op != nullptr)
    {


        for(std::size_t i=0; i<vis.GetSize(); i++){vis[i] = 1.0;}
        auto pol_axis_ptr  = &(std::get<POLPROD_AXIS>(vis));
        auto chan_axis_ptr = &(std::get<CHANNEL_AXIS>(vis));
        pol_axis_ptr->at(0) = "XX";
        op->SetArgs(&vis);
        bool ok = op->Initialize();
        std::cout<<"ok = "<<ok<<std::endl;
        ok = op->Execute();
        std::cout<<"ok = "<<ok<<std::endl;

        for(std::size_t i=0; i<chan_axis_ptr->GetSize(); i++)
        {
            std::cout<< vis.at(0,i,0,0)<<std::endl;
        }


        return 0;

    }

    //error
    return 1;
}
