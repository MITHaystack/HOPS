#include "MHO_StationCodeReader.hh"

#include <fstream>


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
                    if(fTokens.size() == 2){fMap[fTokens[0]] = fTokens[1];}
                }
            }
            file.close();
        }
    }

}


}//end of namespace