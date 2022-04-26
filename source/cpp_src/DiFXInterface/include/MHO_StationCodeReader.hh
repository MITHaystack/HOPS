#ifndef MHO_StationCodeReader_HH__
#define MHO_StationCodeReader_HH__

/*
*@file: MHO_StationCodeReader.hh
*@class: MHO_StationCodeReader
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief: read the file which maps 2-char station codes to 1-char station codes, and put them in a map
*/

#include <string>
#include <map>

#include "MHO_Tokenizer.hh"

namespace hops 
{

class MHO_StationCodeReader
{
    public:
        MHO_StationCodeReader(){};
        virtual ~MHO_StationCodeReader(){};

        void ReadStationCodes(std::string station_codes_file);
        std::map<std::string, std::string> GetStationCodeMap(){return fMap;}

    private:

        std::string fLine;
        MHO_Tokenizer fTokenizer;
        std::vector< std::string > fTokens;

        std::map<std::string, std::string> fMap;
};

}//end of namespace


#endif /* end of include guard: MHO_StationCodeReader */