#include "MHO_StationCodeReader.hh"

#include <fstream>
#include <iostream>
#include <cctype>

namespace hops 
{


void MHO_StationCodeReader::ReadStationCodes(std::string station_codes_file)
{
    fMap.clear();

    fTokenizer.SetDelimiter(" ");
    fTokenizer.SetIncludeEmptyTokensFalse();

    if(station_codes_file != "")
    {
        //open file
        std::ifstream file(station_codes_file.c_str());
        if(file.is_open())
        {
            //read lines until end 
            while( getline(file,fLine) )
            {
                if(fLine.size() != 0)
                {
                    //parse line and covert to tokens 
                    //std::cout<<"line = "<<fLine<<std::endl;
                    fTokens.clear();
                    fTokenizer.SetString(&fLine);
                    fTokenizer.GetTokens(&fTokens);
                    //if the number of tokens is 2, insert into the map
                    if(fTokens.size() == 2)
                    {
                        std::string mk4_id = fTokens[0];
                        //convert the 2-char station code to upper case to match DiFX convention
                        std::string station_code = ToUpperCase(fTokens[1]);
                        bool ok = true;

                        if(fStationCodes.count(station_code) != 0 ){ok = false;}
                        if(fMk4Ids.count(mk4_id) != 0 ){ok = false;}

                        if(!ok)
                        {
                            msg_error("difx_interface", "ignoring duplicate station code mapping: "<< 
                            station_code<<"->"<<mk4_id<<" in file: " << station_codes_file << eom );
                        }
                        else 
                        {
                            if(station_code.size() == 2 && mk4_id.size() == 1)
                            {
                                fStationCodes.insert(station_code);
                                fMk4Ids.insert(mk4_id);
                                fMap[station_code] = mk4_id;
                            }
                            else 
                            {
                                msg_error("difx_interface", "ignoring station code mapping: "<< 
                                station_code<<"->"<<mk4_id<<" due to improper token length in file: " << station_codes_file << eom );
                            }
                        }
                    }
                    else 
                    {
                        msg_error("difx_interface", "ignoring line with improper number of tokens in file: " 
                        << station_codes_file << eom );
                    }
                }
            }
            file.close();
        }
    }


    //dump the station code map 
    for(auto it = fMap.begin(); it != fMap.end(); it++)
    {
        std::cout<<"station: "<<it->first<<", "<<it->second<<std::endl;
    }

}

std::string 
MHO_StationCodeReader::ToUpperCase(std::string token)
{
    std::string value;
    value.resize( token.size() );
    for(std::size_t i=0; i<token.size(); i++)
    {
        if( isalpha(token[i]) )
        {
            value[i] = toupper(token[i]);
        }
        else 
        {
            value[i] = token[i];
        }
    }

    return value;
}


}//end of namespace