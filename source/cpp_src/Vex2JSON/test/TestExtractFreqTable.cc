#include <fstream>
#include <iostream>
#include <string>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_VexGenerator.hh"

using namespace hops;

int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string jsonfile(argv[1]);

    std::ifstream ifs;
    ifs.open(jsonfile.c_str(), std::ifstream::in);

    mho_json fVex;
    if(ifs.is_open())
    {
        fVex = mho_json::parse(ifs);
    }
    ifs.close();

    //assume we now have all ovex/vex in the fVex object, and that we only have a single scan
    //should only have a single 'scan' element under the schedule section, so find it
    auto sched = fVex["$SCHED"];
    if(sched.size() != 1)
    {
        msg_error("mk4interface", "OVEX file schedule section contains more than one scan." << eom);
    }
    auto scan = sched.begin().value();
    std::string mode_key = scan["mode"].get< std::string >();

    std::cout << "mode key = " << mode_key << std::endl;

    int nst = scan["station"].size(); // nst;

    std::cout << "number of stations = " << nst << std::endl;
    //maps to resolve links
    std::map< std::string, std::string > fStationCodeToSiteID;
    std::map< std::string, std::string > fStationCodeToMk4ID;
    std::map< std::string, std::string > fStationCodeToFreqTableName;
    std::map< std::string, std::string > fMk4IDToFreqTableName;

    for(int ist = 0; ist < nst; ist++)
    {
        //find the frequency table for this station
        //first locate the mode info
        auto mode = fVex["$MODE"][mode_key];
        std::string freq_key;
        for(auto it = mode["$FREQ"].begin(); it != mode["$FREQ"].end(); ++it)
        {
            std::string keyword = (*it)["keyword"].get< std::string >();
            std::size_t n_qual = (*it)["qualifiers"].size();

            std::cout << "freq setup keyword = " << keyword << std::endl;
            for(std::size_t q = 0; q < n_qual; q++)
            {
                std::string station_code = (*it)["qualifiers"][q].get< std::string >();
                fStationCodeToFreqTableName[station_code] = keyword;
                std::cout << "qualifier @" << q << " = " << (*it)["qualifiers"][q].get< std::string >() << std::endl;

                //std::string site_key =
                std::string site_key = fVex["$STATION"][station_code]["$SITE"][0]["keyword"].get< std::string >();
                std::string mk4_id = fVex["$SITE"][site_key]["mk4_site_ID"].get< std::string >();
                fMk4IDToFreqTableName[mk4_id] = keyword;

                std::cout << keyword << " : " << mk4_id << " : " << site_key << std::endl;
            }
        }
    }

    //now we need to fill in the channel labels with information from the vex

    return 0;
}
