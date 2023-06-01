#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_ChannelLabeller.hh"
#include "MHO_ContainerDefinitions.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "TestChannelLabeller";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    MHO_ChannelLabeller<visibility_type> label_maker;
    
    label_maker.FillIndexToChannelLabel(64);
    std::cout<<"---------------------------"<<std::endl;
    label_maker.FillIndexToChannelLabel(189);
    std::cout<<"---------------------------"<<std::endl;
    label_maker.FillIndexToChannelLabel(4100);

    return 0;
}
