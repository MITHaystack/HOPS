#ifndef MHO_LinearDParCorrection_HH__
#define MHO_LinearDParCorrection_HH__

#include <cctype>
#include <cmath>
#include <complex>
#include <map>
#include <vector>

#include "MHO_Constants.hh"
#include "MHO_Message.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UnaryOperator.hh"

#include "MHO_Reducer.hh"

namespace hops
{

/*!
 *@file MHO_LinearDParCorrection.hh
 *@class MHO_LinearDParCorrection
 *@author J. Barrett - barrettj@mit.edu
 *@date Wed Feb 7 23:58:10 2024 -0500
 *@brief
 */

/**
 * @brief Class MHO_LinearDParCorrection
 */
class MHO_LinearDParCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_LinearDParCorrection();
        virtual ~MHO_LinearDParCorrection();

        /**
         * @brief Setter for pol product set
         * 
         * @param pp_vec Reference to a vector of strings representing the pol product set
         */
        void SetPolProductSet(std::vector< std::string >& pp_vec) { fPolProductSet = pp_vec; };

        // //these data objects are not used yet, but could be needed if we want to apply
        // //time-dependence to the pol-product pre-factors
        // void SetReferenceStationCoordinateData(station_coord_type* ref_data){fRefData = ref_data;};
        // void SetRemoteStationCoordinateData(station_coord_type* rem_data){fRemData = rem_data;};

        //parallactic angle values for each station (expects degrees)
        /**
         * @brief Setter for reference station parallactic angle
         * 
         * @param p reference station parallactic angle value in degrees
         */
        void SetReferenceParallacticAngle(double p) { fRefParAngle = p; }

        /**
         * @brief Setter for remote station parallactic angle
         * 
         * @param p value for remote station parallactic angle in degrees
         */
        void SetRemoteParallacticAngle(double p) { fRemParAngle = p; }

        // //not currently used (but needed for circ-circ pol sum)
        // void SetReferenceMountType(std::string mt){fRefMountType = mt;}
        // void SetRemoteMountType(std::string mt){fRemMountType = mt;}

    protected:
        /**
         * @brief Initializes in-place visibility_type pointer.
         * 
         * @param in Pointer to visibility_type object
         * @return True if initialization is successful
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(visibility_type* in) override;
        /**
         * @brief Initializes out-of-place visibility data using input visibility_type pointer.
         * 
         * @param in Const pointer to input visibility_type data.
         * @param out (visibility_type*)
         * @return Boolean indicating success of initialization.
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        /**
         * @brief Applies linear delta-parallactic angle correction to visibility data in-place.
         * 
         * @param in Input visibility_type* data to be corrected.
         * @return True indicating successful application of correction.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(visibility_type* in) override;
        /**
         * @brief Copies input visibility data and applies the pre-multiplication operation.
         * 
         * @param in Input visibility_type data to be copied.
         * @param out (visibility_type*)
         * @return True indicating successful execution.
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        //multiplies each pol product by the appropriate scaling pre-factor
        /**
         * @brief Multiplies each pol product in input visibility_type by its appropriate pre-factor.
         * 
         * @param in Input visibility_type to be multiplied by pre-factors
         */
        void PreMultiply(visibility_type* in);

        /**
         * @brief Getter for prefactor
         * 
         * @param pp_label Input polarization label (XX, YY, XY, or YX)
         * @return prefactor as a std::complex<double>
         */
        std::complex< double > GetPrefactor(std::string pp_label);

        std::vector< std::string > fPolProductSet;

        double fRefParAngle;
        double fRemParAngle;
        std::string fRefMountType;
        std::string fRemMountType;
};

} // namespace hops

#endif /*! end of include guard: MHO_LinearDParCorrection */
