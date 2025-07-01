#ifndef MHO_CircularFieldRotationCorrection_HH__
#define MHO_CircularFieldRotationCorrection_HH__

#include <cctype>
#include <cmath>
#include <complex>
#include <map>
#include <vector>

#include "MHO_Constants.hh"
#include "MHO_Message.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_StationModel.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UnaryOperator.hh"

#include "MHO_Reducer.hh"

namespace hops
{

/*!
 *@file MHO_CircularFieldRotationCorrection.hh
 *@class MHO_CircularFieldRotationCorrection
 *@author J. Barrett - barrettj@mit.edu
 *@date Wed Feb 7 23:58:10 2024 -0500
 *@brief
 */

/**
 * @brief Class MHO_CircularFieldRotationCorrection
 */
class MHO_CircularFieldRotationCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_CircularFieldRotationCorrection();
        virtual ~MHO_CircularFieldRotationCorrection();

        /**
         * @brief Setter for pol product set
         * 
         * @param pp_vec Reference to a vector of strings representing pol product set
         */
        void SetPolProductSet(std::vector< std::string >& pp_vec) { fPolProductSet = pp_vec; };

        /**
         * @brief Setter for fourfit reference time vex string
         * 
         * @param frt_vex_string Input vex string representing reference time
         */
        void SetFourfitReferenceTimeVexString(std::string frt_vex_string) { fFourfitRefTimeString = frt_vex_string; };

        //these data objects are not yet used to the fullest extent,
        //but they could be if we want to apply a time-dependant corrections
        //to the pol-product pre-factors
        /**
         * @brief Setter for reference station coordinate data
         * 
         * @param ref_data Input pointer to station_coord_type structure
         */
        void SetReferenceStationCoordinateData(station_coord_type* ref_data) { fRefData = ref_data; };

        /**
         * @brief Setter for remote station coordinate data
         * 
         * @param rem_data Pointer to station_coord_type structure containing remote station coordinates
         */
        void SetRemoteStationCoordinateData(station_coord_type* rem_data) { fRemData = rem_data; };

        //set the station mount types
        /**
         * @brief Setter for reference mount type
         * 
         * @param mt New reference mount type as string
         */
        void SetReferenceMountType(std::string mt) { fRefMountType = mt; }

        /**
         * @brief Setter for remote mount type
         * 
         * @param mt New remote mount type as string
         */
        void SetRemoteMountType(std::string mt) { fRemMountType = mt; }

    protected:
        /**
         * @brief Initializes in-place visibility_type pointer.
         * 
         * @param in Input visibility_type pointer to initialize
         * @return True if initialization is successful
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(visibility_type* in) override;
        /**
         * @brief Initializes out-of-place visibility data from input data.
         * 
         * @param in Const pointer to input visibility_type data.
         * @param out (visibility_type*)
         * @return Boolean indicating success (always true in current implementation).
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        /**
         * @brief Applies a circular field rotation correction to visibility data in-place.
         * 
         * @param in Input visibility_type* data to be corrected.
         * @return bool indicating successful execution.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(visibility_type* in) override;
        /**
         * @brief Copies input visibility data and applies pre-multiplication, returning success.
         * 
         * @param in Const reference to input visibility_type data
         * @param out Reference to output visibility_type data
         * @return Boolean indicating successful execution
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        /**
         * @brief Determines mount type from given string and returns corresponding code.
         * 
         * @param mount Input mount type as a string.
         * @return Mount type code (NO_MOUNT_TYPE, CASSEGRAIN, NASMYTHLEFT, NASMYTHRIGHT) or NO_MOUNT_TYPE if unknown.
         */
        int DetermineMountCode(const std::string& mount) const;

        //multiplies each pol product by the appropriate pre-factor
        /**
         * @brief Multiplies each polarization product by an appropriate pre-factor.
         * 
         * @param in Input visibility_type pointer to be multiplied
         */
        void PreMultiply(visibility_type* in);

        /**
         * @brief Getter for prefactor
         * 
         * @param pp_label Input polarization product label to retrieve its corresponding prefactor
         * @return Complex prefactor value associated with the input polarization product label
         */
        std::complex< double > GetPrefactor(std::string pp_label);

        std::vector< std::string > fPolProductSet;

        double fRefParAngle;
        double fRemParAngle;
        double fRefElevation;
        double fRemElevation;

        std::string fFourfitRefTimeString;

        std::string fRefMountType;
        std::string fRemMountType;

        station_coord_type* fRefData;
        station_coord_type* fRemData;

        MHO_StationModel fRefModel;
        MHO_StationModel fRemModel;
};

} // namespace hops

#endif /*! end of include guard: MHO_CircularFieldRotationCorrection */
