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
*@brief Class MHO_StationCodeMap Handles the mapping of two character and one character station representation.
* This class can be initialized from a file, otherwise it assigns free 1-char codes
* based on the order in which stations are encountered. Optionally, we can enable
* some legacy station code assignments inherited from difx2mark4
*/

class MHO_StationCodeMap
{
    public:
        MHO_StationCodeMap(bool use_legacy_default_codes = false);
        virtual ~MHO_StationCodeMap();

        /**
         * @brief Sets legacy codes usage flag to true.
         */
        void UseLegacyCodes() { fUseLegacyCodes = true; }

        /**
         * @brief Initializes station codes by reading from a file and optionally using legacy codes.
         * 
         * @param station_codes_file Path to the file containing station codes.
         */
        void InitializeStationCodes(std::string station_codes_file = "");

        /**
         * @brief Getter for station code from mk4id
         * 
         * @param mk4id Input Mk4 ID string to search for in the map.
         * @return Station code as a string if found; otherwise, an empty string.
         */
        std::string GetStationCodeFromMk4Id(std::string mk4id);
        /**
         * @brief Getter for mk4id from station code
         * 
         * @param station_code Input station code to retrieve mk4id
         * @return mk4id corresponding to input station_code
         */
        std::string GetMk4IdFromStationCode(std::string station_code);

        /**
         * @brief Getter for all mk4ids
         * 
         * @return std::vector<std::string containing all Mk4 IDs.
         */
        std::vector< std::string > GetAllMk4Ids();
        /**
         * @brief Getter for all station codes
         * 
         * @return A vector of strings containing all unique station codes.
         */
        std::vector< std::string > GetAllStationCodes();

    private:
        /**
         * @brief Initializes free Mk4 IDs and clears station codes and Mk4 IDs maps.
         */
        void Initialize();
        /**
         * @brief Checks if the size and length of tokens are valid.
         * 
         * @return True if tokens are valid, false otherwise.
         */
        bool TokensAreValid();

        /**
         * @brief Processes a line containing station codes and inserts them into a map if valid.
         */
        void ProcessLine();

        /**
         * @brief Inserts a pair of station_code and mk4id into maps and updates free Mk4Ids.
         * 
         * @param station_code Station code string
         * @param mk4id Mk4 identifier string
         */
        void InsertPair(std::string station_code, std::string mk4id);

        /**
         * @brief Converts a given string to uppercase while preserving non-alphabetic characters.
         * 
         * @param token Input string to be converted to uppercase.
         * @return Uppercase version of the input string, preserving non-alphabetic characters.
         */
        std::string ToUpperCase(std::string token);     //AA

        /**
         * @brief Converts a given string to lowercase while preserving non-alphabetic characters.
         * 
         * @param token Input string to be converted to lowercase.
         * @return The input string converted to lowercase.
         */
        std::string ToLowerCase(std::string token);     //aa

        /**
         * @brief Converts a given string token to canonical case (first character uppercase, second lowercase).
         * 
         * @param token Input string token to be converted
         * @return String token in canonical case
         */
        std::string ToCanonicalCase(std::string token); //Aa

        bool fUseLegacyCodes;
        /**
         * @brief Adds a legacy set of station codes to the MHO_StationCodeMap.
         */
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
