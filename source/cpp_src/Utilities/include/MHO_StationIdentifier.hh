#ifndef MHO_StationIdentifier_HH__
#define MHO_StationIdentifier_HH__

#include <map>
#include <set>
#include <string>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_StationIdentity.hh"

namespace hops
{

/*!
 *@file  MHO_StationIdentifier.hh
 *@class  MHO_StationIdentifier
 *@author  J. Barrett - barrettj@mit.edu
 *@date Thu Apr 28 12:25:09 2022 -0400
 *@brief Class MHO_StationIdentifier 
 * Handles the mapping of two character and one character mk4ids to station names -- this is a singleton class
 */

class MHO_StationIdentifier
{
    public:
        /**
         * @brief Getter for singleton instance
         *
         * @return MHO_StationIdentifier* singleton instance
         * @note This is a static function.
         */
        static MHO_StationIdentifier* GetInstance();

        int Insert(MHO_StationIdentity station_identity)
        {
            std::string name = station_identity.GetStationName();
            std::string code = station_identity.GetStationCode();
            std::string mk4id = station_identity.GetStationMk4Id();

            bool name_present = ( fCode2Name.find(name) != fCode2Name.end() );
            bool code_present = ( fCode2Name.find(code) != fCode2Name.end() );
            bool mk4id_present = ( fCode2Name.find(mk4id) != fCode2Name.end() );

            if(!name_present && (code_present || mk4id_present))
            {
                msg_warn("utilities", "station: "<< name << " shares a code or mk4id with another station" << eom);
                return -1;
            }

            if(!name_present && !code_present && !mk4id_present)
            {
                msg_debug("utilities", "inserting station identity: "<< station_identity.as_string() << eom );
                fCode2Name[name] = name;
                fCode2Name[code] = name;
                fCode2Name[mk4id] = name;
                fName2Code[name] = code;
                fName2Mk4ID[name] = mk4id;
                fStationIds.push_back(station_identity);
            }
            //success 
            return 0;
        }

        int Insert(const std::string& name, const std::string& code, const std::string& mk4id)
        {
            MHO_StationIdentity tmp;
            tmp.SetAll(name, code, mk4id);
            return Insert(tmp);
        }

        std::string CanonicalStationName(std::string code) const 
        {
            auto it = fCode2Name.find(code);
            if( it != fCode2Name.end() )
            {
                return it->second;
            }
            else 
            {
                msg_warn("utilities", "cannot locate cannonical station name for: <"<< code<<">" << eom );
                return code; //return the same value that was submitted
            }
        }
        
        std::string StationMk4IDFromName(std::string name) const 
        {
            auto it = fName2Mk4ID.find(name);
            if( it != fName2Mk4ID.end() )
            {
                return it->second;
            }
            else 
            {
                msg_warn("utilities", "cannot locate mk4 id from cannonical station name: <"<<name<<">" << eom );
                return name; //return the same value that was submitted
            }
        }
        
        std::string StationCodeFromName(std::string name) const 
        {
            auto it = fName2Code.find(name);
            if( it != fName2Code.end() )
            {
                return it->second;
            }
            else 
            {
                msg_warn("utilities", "cannot locate station code from cannonical station name: <"<<name<<">" << eom );
                return name; //return the same value that was submitted
            }
        }

    private:
        
        //private for singleton 
        MHO_StationIdentifier();
        virtual ~MHO_StationIdentifier();
        static MHO_StationIdentifier* fStationIdentifier;
    
        std::vector< MHO_StationIdentity > fStationIds;
        std::set< std::string > fCodeSet;
        std::map< std::string, std::string > fCode2Name; // 1 char, 2 char, name -> proper name
        std::map< std::string, std::string > fName2Code; //name -> 2 char code
        std::map< std::string, std::string > fName2Mk4ID; //name -> 1 char mk4 id

};






} // namespace hops

#endif /*! end of include guard: MHO_StationIdentifier */
