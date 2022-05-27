#include <string>
#include <iostream>
#include <fstream>

#include "MHO_Message.hh"
#include "MHO_VexParser.hh"


using namespace hops;

int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string vexfile(argv[1]);

    MHO_VexParser vparser;
    vparser.SetVexVersion("2.0");
    vparser.SetVexFile(vexfile);
    vparser.ReadFile();
    std::cout<<"removing comments"<<std::endl;
    vparser.RemoveComments();
    std::cout<<"flagging blocks"<<std::endl;
    vparser.MarkBlocks();

    // std::list< std::string >* vlines = vparser.GetLines();
    // 
    // int i=0;
    // for(auto it = vlines->begin(); it != vlines->end(); it++)
    // {
    //     if((*it)[0] == '$'){std::cout<<*it<<std::endl;}
    // }




    // std::string format_dir = vparser.GetFormatDirectory();
    // std::string blocknames_v2 = format_dir + "block-names.json";
    // std::ifstream bn_ifs;
    // bn_ifs.open( blocknames_v2.c_str(), std::ifstream::in );
    // 
    // if(bn_ifs.is_open())
    // {
    //     json blocknames = mho_ordered_json::parse(bn_ifs);
    //     std::cout<< blocknames.dump(2) << std::endl;
    // }
    // 
    // bn_ifs.close();
    // 
    // vparser.SetVexVersion("1.5");
    // format_dir = vparser.GetFormatDirectory();
    // std::string antenna = format_dir + "antenna.json";
    // std::ifstream ant_ifs;
    // ant_ifs.open( antenna.c_str(), std::ifstream::in );
    // 
    // if(ant_ifs.is_open())
    // {
    //     json ant = mho_ordered_json::parse(ant_ifs);
    //     std::cout<< ant.dump(2) << std::endl;
    // }
    // 
    // ant_ifs.close();

    return 0;
}
