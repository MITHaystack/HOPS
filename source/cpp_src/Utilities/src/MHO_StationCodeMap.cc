#include "MHO_StationCodeMap.hh"

#include <cctype>
#include <fstream>

namespace hops
{

MHO_StationCodeMap::MHO_StationCodeMap(bool use_legacy_default_codes)
{
    fUseLegacyCodes = use_legacy_default_codes;
    Initialize();
}

MHO_StationCodeMap::~MHO_StationCodeMap(){};

void MHO_StationCodeMap::InitializeStationCodes(std::string station_codes_file)
{
    Initialize();
    fTokenizer.SetDelimiter(" ");
    fTokenizer.SetIncludeEmptyTokensFalse();

    if(station_codes_file != "")
    {
        //open file
        fFile = station_codes_file;
        std::ifstream file(station_codes_file.c_str());
        if(file.is_open())
        {
            //read lines until end
            while(getline(file, fLine))
            {
                ProcessLine();
            }
            file.close();
        }
    }

    if(fUseLegacyCodes)
    {
        fFile = "<legacy map>";
        AddLegacyCodeMap();
    }
}

void MHO_StationCodeMap::ProcessLine()
{
    if(fLine.size() != 0)
    {
        //parse line and covert to tokens
        fTokens.clear();
        fTokenizer.SetString(&fLine);
        fTokenizer.GetTokens(&fTokens);
        //if the number of tokens is 2, insert into the map
        if(TokensAreValid())
        {
            std::string mk4id = fTokens[0];
            //make sure 2-char code is in canonical case
            std::string station_code = ToCanonicalCase(fTokens[1]);
            bool ok = true;

            if(fStationCodes.count(station_code) != 0)
            {
                ok = false;
            }
            if(fMk4Ids.count(mk4id) != 0)
            {
                ok = false;
            }

            if(!ok)
            {
                msg_warn("utility", "ignoring duplicate station code mapping: " << station_code << "->" << mk4id
                                                                                << " in file: " << fFile << eom);
            }
            else
            {
                InsertPair(station_code, mk4id);
            }
        }
        else
        {
            msg_warn("utility", "ignoring line with improper tokens: " << fLine << eom);
        }
    }
}

void MHO_StationCodeMap::InsertPair(std::string station_code, std::string mk4id)
{
    fStationCodes.insert(station_code);
    fMk4Ids.insert(mk4id);
    fFreeMk4Ids.erase(mk4id);
    fCode2Mk4IdMap[station_code] = mk4id;
    fMk4Id2CodeMap[mk4id] = station_code;
}

bool MHO_StationCodeMap::TokensAreValid()
{
    if(fTokens.size() != 2)
    {
        return false;
    }
    if(fTokens[0].length() != 1)
    {
        return false;
    }
    if(fTokens[1].length() != 2)
    {
        return false;
    }
    return true;
}

std::string MHO_StationCodeMap::GetStationCodeFromMk4Id(std::string mk4id)
{
    std::string code = "";
    auto it = fMk4Id2CodeMap.find(mk4id);
    if(it != fMk4Id2CodeMap.end())
    {
        code = it->second;
    }
    return code;
}

std::string MHO_StationCodeMap::GetMk4IdFromStationCode(std::string station_code)
{
    std::string mk4id = "";
    if(station_code.length() != 2)
    {
        return mk4id;
    }
    std::string code = ToCanonicalCase(station_code);

    auto it = fCode2Mk4IdMap.find(code);
    if(it != fCode2Mk4IdMap.end())
    {
        mk4id = it->second;
    }
    else
    {
        //we've encountered a new station code, so assign it a new Mk4 site ID
        //from the pool of free IDs
        if(fFreeMk4Ids.size() > 0)
        {
            mk4id = *(fFreeMk4Ids.begin());
            InsertPair(code, mk4id);
            msg_debug("utility", "encountered new station code: " << code << ", assigning it mk4 site ID: " << mk4id << eom);
        }
        else
        {
            msg_warn("utility",
                     "could not assign mk4 site ID for station code: " << code << ", no free characters available. " << eom);
        }
    }
    return mk4id;
}

void MHO_StationCodeMap::Initialize()
{
    fFreeMk4Ids.clear();

    char value = 'A';
    for(int i = 0; i < 26; i++)
    {
        std::string code(1, value);
        fFreeMk4Ids.insert(code);
        value++;
    }
    value = 'a';
    for(int i = 0; i < 26; i++)
    {
        std::string code(1, value);
        fFreeMk4Ids.insert(code);
        value++;
    }

    fStationCodes.clear();
    fMk4Ids.clear();
    fMk4Id2CodeMap.clear();
    fCode2Mk4IdMap.clear();
}

void MHO_StationCodeMap::AddLegacyCodeMap()
{
    //This is the legacy default set of station codes inherited from difx2mark4
    //some of which are not even valid, e.g [y Y ]??
    char code_table[52][5] = {"A Ai", "B Bd", "C Sh", "D 13", "E Wf", "F Eb", "G Gb", "H Ho", "I Ma", "J Cc", "K Kk",
                              "L xx", "M Mc", "N Ny", "O Kb", "P Oh", "Q Tc", "R Zc", "S Nt", "T Ts", "U Ur", "V Wz",
                              "W xx", "X On", "Y Yb", "Z Mh", "a Ap", "b Br", "c Cm", "d Cn", "e xx", "f Fd", "g xx",
                              "h Hn", "i xx", "j Jc", "k Kp", "l La", "m Mk", "n Nl", "o Ov", "p Pt", "q Qb", "r Ro",
                              "s Sc", "t Ti", "u Ur", "v Pv", "w Wb", "x xx", "y Y ", "z xx"};

    for(int i = 0; i < 52; i++)
    {
        fLine = std::string(&(code_table[i][0]));
        ProcessLine();
    }
}

std::string MHO_StationCodeMap::ToUpperCase(std::string token)
{
    std::string value;
    value.resize(token.size());
    for(std::size_t i = 0; i < token.size(); i++)
    {
        if(isalpha(token[i]))
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

std::string MHO_StationCodeMap::ToLowerCase(std::string token)
{
    std::string value;
    value.resize(token.size());
    for(std::size_t i = 0; i < token.size(); i++)
    {
        if(isalpha(token[i]))
        {
            value[i] = tolower(token[i]);
        }
        else
        {
            value[i] = token[i];
        }
    }
    return value;
}

std::string MHO_StationCodeMap::ToCanonicalCase(std::string token)
{
    std::string value;
    value.resize(token.size());
    //first char is upper case
    if(isalpha(token[0]))
    {
        value[0] = toupper(token[0]);
    }
    else
    {
        value[0] = token[0];
    }
    //rest are lower case
    for(std::size_t i = 1; i < token.size(); i++)
    {
        if(isalpha(token[i]))
        {
            value[i] = tolower(token[i]);
        }
        else
        {
            value[i] = token[i];
        }
    }
    return value;
}

std::vector< std::string > MHO_StationCodeMap::GetAllMk4Ids()
{
    std::vector< std::string > mk4id_list;
    for(auto it = fMk4Id2CodeMap.begin(); it != fMk4Id2CodeMap.end(); it++)
    {
        mk4id_list.push_back(it->first);
    }
    return mk4id_list;
}

std::vector< std::string > MHO_StationCodeMap::GetAllStationCodes()
{
    std::vector< std::string > code_list;
    for(auto it = fCode2Mk4IdMap.begin(); it != fCode2Mk4IdMap.end(); it++)
    {
        code_list.push_back(it->first);
    }
    return code_list;
}

} // namespace hops
