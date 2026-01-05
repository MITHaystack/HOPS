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
 *@brief Class MHO_StationIdentifier Handles the mapping of two character and one character mk4ids to station names
 */

class MHO_StationIdentifier
{
    public:
        MHO_StationIdentifier();
        virtual ~MHO_StationIdentifier();

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
                fCode2Name[name] = name;
                fCode2Name[code] = name;
                fCode2Name[mk4id] = name;
            }
            //success 
            return 0;
        }

        int Insert(const std::string& name, const std::string& code, const std::string& mk4id)
        {
            MHO_StationIdentity tmp;
            tmp.SetAll(name, code, mk4id);
            return Insert(temp)
        }

        std::string CanonicalStationName(std::string code) const 
        {
            auto it = fCode2Name.find(code);
            if( it != fCode2Name.end() )
            {
                return *it;
            }
            else 
            {
                msg_warn("utilities", "cannot locate cannonical station name for: <"<< code<<">" << eom );
                return std::string("");
            }
        }

    private:
    
        std::set< std::string > fCodeSet;
        std::map< std::string, std::string > fCode2Name; // 1 char, 2 char, name -> proper name
};

} // namespace hops

#endif /*! end of include guard: MHO_StationIdentifier */
