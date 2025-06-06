#ifndef MHO_StationCodeMap_HH__
#define MHO_StationCodeMap_HH__

#include <map>
#include <set>
#include <stack>
#include <string>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

/*!
*@file  MHO_StationCodeMap.hh
*@class  MHO_StationCodeMap
*@author  J. Barrett - barrettj@mit.edu
*@date Thu Apr 28 12:25:09 2022 -0400
*@brief
    Handles the mapping of two character and one character station representation.
    This class can be initialized from a file, otherwise it assigns free 1-char codes
    based on the order in which stations are encountered. Optionally, we can enable
    some legacy station code assignments inherited from difx2mark4
*/

class MHO_StationCodeMap
{
    public:
        MHO_StationCodeMap(bool use_legacy_default_codes = false);
        virtual ~MHO_StationCodeMap();

        void UseLegacyCodes() { fUseLegacyCodes = true; }

        void InitializeStationCodes(std::string station_codes_file = "");

        std::string GetStationCodeFromMk4Id(std::string mk4id);
        std::string GetMk4IdFromStationCode(std::string station_code);

        std::vector< std::string > GetAllMk4Ids();
        std::vector< std::string > GetAllStationCodes();

    private:
        void Initialize();
        bool TokensAreValid();
        void ProcessLine();
        void InsertPair(std::string station_code, std::string mk4id);

        std::string ToUpperCase(std::string token);     //AA
        std::string ToLowerCase(std::string token);     //aa
        std::string ToCanonicalCase(std::string token); //Aa

        bool fUseLegacyCodes;
        void AddLegacyCodeMap();

        std::string fFile;
        std::string fLine;
        MHO_Tokenizer fTokenizer;
        std::vector< std::string > fTokens;

        std::set< std::string > fStationCodes;
        std::set< std::string > fMk4Ids;
        std::set< std::string > fFreeMk4Ids;

        std::map< std::string, std::string > fMk4Id2CodeMap; // 1 char to 2 char
        std::map< std::string, std::string > fCode2Mk4IdMap; // 2 char to 1 char
};

} // namespace hops

#endif /*! end of include guard: MHO_StationCodeMap */
