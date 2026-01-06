#ifndef MHO_StationIdentity_HH__
#define MHO_StationIdentity_HH__

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <cctype>

#include "MHO_Message.hh"
#include "MHO_Types.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{

/*!
 *@file MHO_StationIdentity.hh
 *@class MHO_StationIdentity
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Nov 13 01:34:27 PM EST 2025
 *@brief Class MHO_StationIdentity - a class to store, associate, and compare 
 * 1-char, 2-char, and 8-char station code/id/name identifiers
 */


class MHO_StationIdentity
{

    public:
        MHO_StationIdentity():
            fStationName(""),
            fStationCode(""),
            fStationMk4ID("")
        {}
            
        MHO_StationIdentity(const std::string& name, const std::string& code, const std::string& mk4id)
        {
            SetAll(name,code,mk4id);
        }
        
        MHO_StationIdentity( const mho_json& site_vex ):
            fStationName(""),
            fStationCode(""),
            fStationMk4ID("")
        {
            if(site_vex.contains("mk4_site_ID") && site_vex.contains("site_ID") && site_vex.contains("site_name"))
            {
                std::string mk4id = site_vex["mk4_site_ID"].get<std::string>();
                std::string code = site_vex["site_ID"].get<std::string>();
                std::string name = site_vex["site_name"].get<std::string>();
                SetAll(name,code,mk4id);
            }
        }

        virtual ~MHO_StationIdentity(){};

        MHO_StationIdentity(const MHO_StationIdentity& copy)
        {
            fStationName = copy.fStationName;
            fStationCode = copy.fStationCode;
            fStationMk4ID = copy.fStationMk4ID;
        }

        MHO_StationIdentity& operator=(const MHO_StationIdentity& rhs)
        {
            fStationName = rhs.fStationName;
            fStationCode = rhs.fStationCode;
            fStationMk4ID = rhs.fStationMk4ID;
            return *this;
        }

        bool operator==(const MHO_StationIdentity& rhs) const
        {
            if(fStationName == rhs.fStationName ){return true;}
            // if(fStationCode == rhs.fStationCode){return true;}
            // if(fStationMk4ID == rhs.fStationMk4ID){return true;}
            return false;
        }
        
        bool operator!=(const MHO_StationIdentity& rhs) const { return !(*this == rhs); }

        //ordering on name
        bool operator<(const MHO_StationIdentity& rhs) const
        {
            //compare station names
            if(fStationName.size() != 0 && rhs.fStationName.size() != 0)
            {
                return (fStationName < rhs.fStationName);
            }
            //compare station codes
            if(fStationCode.size() != 0 && rhs.fStationCode.size() != 0)
            {
                return (fStationCode < rhs.fStationCode);
            }
            //compare mk4 1-char codes
            if(fStationMk4ID.size() != 0 && rhs.fStationMk4ID.size() != 0)
            {
                return (fStationMk4ID < rhs.fStationMk4ID);
            }
        }
        
        /*!*
         * Check if this station identity matches the given 1-char/2-char code or multi-char name
         * @return A boolean if successful match
         */
        bool matches(const std::string& station_identifier) const
        {
            if(station_identifier.size() == 1)
            {
                return (fStationMk4ID == station_identifier);
            }
            if(station_identifier.size() == 2)
            {
                return (fStationCode == station_identifier);
            }
            //default compare with name (but only as upper case)
            std::string tmp = station_identifier;
            std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
            return (fStationName == tmp);
        }

        /*!*
         * Convert the station identifier to a formatted string
         * @return A std::string containing station id info, delimited by commas
         */
        std::string as_string() const
        {
            std::stringstream ss;
            ss << fStationName;
            ss << ",";
            ss << fStationCode;
            ss << ",";
            ss << fStationMk4ID;
            return ss.str();
        }

        /*!*
         * Read a formatted string to fill this object
         * @return success on updating the station identity object using string info
         */
        bool from_string(const std::string& station_id_string)
        {
            std::string input = station_id_string;
            std::vector< std::string > tokens;
            MHO_Tokenizer tokenizer;
            tokenizer.SetDelimiter(",");
            tokenizer.SetIncludeEmptyTokensTrue();
            tokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
            tokenizer.SetString(&input);
            tokenizer.GetTokens(&tokens);
            
            if(tokens.size() < 3)
            {
                return false;
            }
            else 
            {
                SetStationName(tokens[0]);
                SetStationCode(tokens[1]);
                SetStationCode(tokens[2]);
                return true;
            }
        }

        void SetStationName(const std::string& name)
        {
            //(only) station names are always stored as pure-upper case
            fStationName = name;
            std::transform(fStationName.begin(), fStationName.end(), fStationName.begin(), ::toupper);
        }
        
        std::string GetStationName() const {return fStationName;}
        void SetStationCode(const std::string& code){fStationCode = code.substr(0,2);}
        std::string GetStationCode() const {return fStationCode;}
        void SetStationMk4ID(const std::string& id){fStationMk4ID = id.substr(0,1);}
        std::string GetStationMk4Id() const {return fStationMk4ID;}

        void SetAll(const std::string& name, const std::string& code, const std::string& mk4id)
        {
            SetStationName(name);
            SetStationCode(code);
            SetStationMk4ID(mk4id);
        }

    protected:
        
        std::string fStationName; //8 or more char
        std::string fStationCode; //2 char
        std::string fStationMk4ID; //1 char;

};

template< typename XStream > XStream& operator>>(XStream& s, MHO_StationIdentity& stid)
{
    std::string tmp;
    s >> tmp;
    stid.from_string(tmp);
    return s;
};

template< typename XStream > XStream& operator<<(XStream& s, const MHO_StationIdentity& stid)
{
    s << stid.as_string();
    return s;
};

} // namespace hops

#endif /*! end of include guard: MHO_StationIdentity */
