#ifndef MHO_PolarizationRelabeler_HH__
#define MHO_PolarizationRelabeler_HH__

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
 *@file MHO_PolarizationRelabeler.hh
 *@class MHO_PolarizationRelabeler
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Feb 20 12:24:39 PM EST 2026
 *@brief Changes the polarization labels  of a station (X -> Y) or (R -> L), etc
 */

/**
 * @brief Class MHO_PolarizationRelabeler
 */
template< typename visibility_type > class MHO_PolarizationRelabeler: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_PolarizationRelabeler()
        {
            fStationIdentity = "";
            fRemStationKey = "remote_station";
            fRefStationKey = "reference_station";
            fRemStationMk4IDKey = "remote_station_mk4id";
            fRefStationMk4IDKey = "reference_station_mk4id";
            fPol1 = "";
            fPol2 = "";
            fValid = false;
        };

        virtual ~MHO_PolarizationRelabeler(){};

        void SetPolarizationSwapPair(std::string pol1, std::string pol2)
        {
            //set the polarization labels to swap, all instances of pol1 will be replaced by pol2 
            //and all instances of pol2 will be replaced with pol1
            if(pol1.size() == 1 && pol2.size() = 1)
            {
                fPol1 = pol1;
                fPol2 = pol2;
                fValid = true;
            }
            else 
            {
                fValid = false;
                msg_error("calibration", "MHO_PolarizationRelabeler, only single character polarization labels are supported, ignoring." << eom);
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
        void SetStationIdentifier(std::string station_id) { fStationIdentity = station_id; }

    protected:
        /**
         * @brief Initializes visibility_type in-place and returns success.
         *
         * @param in Pointer to visibility_type object to initialize.
         * @return True if initialization was successful, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(visibility_type* in) override { return true; }

        /**
         * @brief Initializes output array out-of-place from input array
         *
         * @param !in Const input visibility_type
         * @param !out Output visibility_type initialized out-of-place
         * @return True if initialization was successful, false otherwise
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const visibility_type* /*!in*/, visibility_type* /*!out*/) override { return true; }

        /**
         * @brief Function ExecuteInPlace - attaches channel labels based on sky frequency or user specified map
         *
         * @param in (visibility_type*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(visibility_type* in) override
        {
            if(in != nullptr)
            {
                //need to use the user provided frequency <-> channel label map
                auto pprod_axis_ptr = &(std::get< POLPROD_AXIS >(*in));
                std::size_t npprods = pprod_axis_ptr->GetSize();

                for(std::size_t st_idx = 0; st_idx < 2; st_idx++)
                {
                    if( IsApplicable(st_idx, in) )
                    {
                        for(std::size_t i=0; i<npprods; i++)
                        {
                            //swap any instances of pol1 <-> pol2
                            std::string pprod = pprod_axis_ptr->at(i);
                            if(pprod[st_idx] == fPol1){pprod[st_idx] = fPol2;}
                            else if(pprod[st_idx] == fPol2){pprod[st_idx] = fPol1;}
                            pprod_axis_ptr->at(i) = pprod;
                        }
                    }
                }
            }
            return false;
        }

        /**
         * @brief Copies input array to output and executes in-place operation on output.
         *
         * @param in Const reference to input visibility_type
         * @param out Reference to output visibility_type
         * @return Result of ExecuteInPlace operation on out
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override
        {
            out->Copy(*in);
            return ExecuteInPlace(out);
        }

    private:

        //keys for tag retrieval
        std::string fStationKey;
        std::string fRemStationKey;
        std::string fRefStationKey;
        std::string fRemStationMk4IDKey;
        std::string fRefStationMk4IDKey;

        //data 
        std::string fStationIdentity;
        std::string fPol1;
        std::string fPol2;

        //determines if to apply the pol relabelling, for the station (ref or rem)
        bool IsApplicable(std::size_t st_idx, const visibility_type* in)
        {
            bool apply_correction = false;
            std::string val;
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

            if(fStationIdentity.size() > 2)
            {
                msg_error("calibration",
                          "station identiy: " << fStationIdentity << " is not a recognizable mark4 or 2-character code" << eom);
                return false;
            }

            if(fStationIdentity.size() == 1) //selection by mk4 id
            {
                in->Retrieve(mk4id_key, val);
                if(fStationIdentity == val || fStationIdentity == "?")
                {
                    apply_correction = true;
                }
            }

            if(fStationIdentity.size() == 2) //selection by 2-char station code
            {
                in->Retrieve(station_code_key, val);
                if(fStationIdentity == val || fStationIdentity == "??")
                {
                    apply_correction = true;
                }
            }
            return apply_correction;
        }


};

} // namespace hops

#endif /*! end of include guard: MHO_PolarizationRelabeler_HH__ */
