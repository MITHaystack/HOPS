#ifndef MHO_PolarizationProductRelabeler_HH__
#define MHO_PolarizationProductRelabeler_HH__

#include <map>
#include <stack>
#include <string>

#include "MHO_Message.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UnaryOperator.hh"

namespace hops
{

/*!
 *@file MHO_PolarizationProductRelabeler.hh
 *@class MHO_PolarizationProductRelabeler
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Feb 20 12:24:39 PM EST 2026
 *@brief Changes the polarization labels  of a station (X -> Y) or (R -> L), etc, as applied to visbility/weight data
 */

/**
 * @brief Class MHO_PolarizationProductRelabeler
 */
template< typename XArrayType > class MHO_PolarizationProductRelabeler: public MHO_UnaryOperator< XArrayType >
{
    public:
        MHO_PolarizationProductRelabeler()
        {
            fRemStationKey = "remote_station";
            fRefStationKey = "reference_station";
            fRemStationMk4IDKey = "remote_station_mk4id";
            fRefStationMk4IDKey = "reference_station_mk4id";
            fPol1 = "";
            fPol2 = "";
            fValid = false;
        };

        virtual ~MHO_PolarizationProductRelabeler(){};

        void SetPolarizationSwapPair(std::string pol1, std::string pol2)
        {
            //set the polarization labels to swap, all instances of pol1 will be replaced by pol2
            //and all instances of pol2 will be replaced with pol1
            if(pol1.size() == 1 && pol2.size() == 1)
            {
                fPol1 = pol1;
                fPol2 = pol2;
                fValid = true;
            }
            else
            {
                fValid = false;
                msg_error("calibration",
                          "MHO_PolarizationProductRelabeler, only single character polarization labels are supported, ignoring."
                              << eom);
            }
        }

        /**
         * @brief Setter for station identifier
         *
         * @param station_id mk4 id of type std::string
         * @details station_id is treated as follows:
         * 1-char => mk4 id
         * 2-char => 2char station code
         */
        void SetStationIdentifier(const std::string& id) { fStationIdentities = {id}; }
        void SetStationIdentifiers(const std::vector<std::string>& ids) { fStationIdentities = ids; }

        std::string GetStationIdentifier() const { return fStationIdentities.empty() ? std::string("") : fStationIdentities[0]; }

    protected:


        /**
         * @brief Function ExecuteInPlace - attaches channel labels based on sky frequency or user specified map
         *
         * @param in (XArrayType*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(XArrayType* in) override
        {
            if(in != nullptr)
            {
                //need to use the user provided frequency <-> channel label map
                auto pprod_axis_ptr = &(std::get< POLPROD_AXIS >(*in));
                std::size_t npprods = pprod_axis_ptr->GetSize();

                for(std::size_t st_idx = 0; st_idx < 2; st_idx++)
                {
                    if(IsApplicable(st_idx, in))
                    {
                        for(std::size_t i = 0; i < npprods; i++)
                        {
                            //swap any instances of pol1 <-> pol2
                            std::string pprod = pprod_axis_ptr->at(i);
                            if(pprod[st_idx] == fPol1[0])
                            {
                                pprod[st_idx] = fPol2[0];
                            }
                            else if(pprod[st_idx] == fPol2[0])
                            {
                                pprod[st_idx] = fPol1[0];
                            }
                            pprod_axis_ptr->at(i) = pprod;
                        }
                    }
                }
            }
            return false;
        }


    private:
        //keys for tag retrieval
        std::string fStationKey;
        std::string fRemStationKey;
        std::string fRefStationKey;
        std::string fRemStationMk4IDKey;
        std::string fRefStationMk4IDKey;

        //data
        std::vector<std::string> fStationIdentities;
        std::string fPol1;
        std::string fPol2;
        bool fValid;

        //determines if to apply the pol relabelling, for the station (ref or rem)
        bool IsApplicable(std::size_t st_idx, const XArrayType* in)
        {
            std::string mk4id_key;
            std::string station_code_key;

            if(st_idx == 0)
            {
                mk4id_key = fRefStationMk4IDKey;
                station_code_key = fRefStationKey;
            }
            else
            {
                mk4id_key = fRemStationMk4IDKey;
                station_code_key = fRemStationKey;
            }

            std::string mk4id_val, code_val;
            in->Retrieve(mk4id_key, mk4id_val);
            in->Retrieve(station_code_key, code_val);

            for(const auto& id : fStationIdentities)
            {
                if(id.size() > 2)
                {
                    msg_error("calibration",
                              "station identity: " << id << " is not a recognizable mark4 or 2-character code" << eom);
                    continue;
                }
                if(id.size() == 1 && (id == mk4id_val || id == "?")) { return true; }
                if(id.size() == 2 && (id == code_val || id == "??")) { return true; }
            }
            return false;
        }
};

} // namespace hops

#endif /*! end of include guard: MHO_PolarizationProductRelabeler_HH__ */
