#ifndef MHO_PolProductSummation_HH__
#define MHO_PolProductSummation_HH__

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
 *@file MHO_PolProductSummation.hh
 *@class MHO_PolProductSummation
 *@author J. Barrett - barrettj@mit.edu
 *@date Sun Jan 14 17:16:10 2024 -0500
 *@brief
 */

/**
 * @brief Class MHO_PolProductSummation
 */
class MHO_PolProductSummation: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_PolProductSummation();
        virtual ~MHO_PolProductSummation();

        //perhaps we should have a separate operator to handle the treatment of the weights?
        /**
         * @brief Setter for weights
         * 
         * @param w Input pointer to weight_type array
         */
        void SetWeights(weight_type* w) { fWeights = w; }

        /**
         * @brief Setter for pol product sum label
         * 
         * @param ppl New pol product sum label
         */
        void SetPolProductSumLabel(std::string ppl) { fSummedPolProdLabel = ppl; }

        /**
         * @brief Setter for pol product set
         * 
         * @param pp_vec Reference to a vector of strings representing the pol product set
         */
        void SetPolProductSet(std::vector< std::string >& pp_vec) { fPolProductSet = pp_vec; };

        //these data objects are not used yet, but could be needed if we want to apply
        //time-dependence to the pol-product pre-factors
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

        //parallactic angle values for each station (expects degrees)
        /**
         * @brief Setter for reference parallactic angle
         * 
         * @param p Input parallactic angle value in degrees
         */
        void SetReferenceParallacticAngle(double p) { fRefParAngle = p; }

        /**
         * @brief Setter for remote parallactic angle
         * 
         * @param p New value for remote parallactic angle
         */
        void SetRemoteParallacticAngle(double p) { fRemParAngle = p; }

        //not currently used (but needed for circ-circ pol sum)
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
         * @brief Initializes reducers and checks initialization status.
         * 
         * @param in Input visibility_type pointer.
         * @return Boolean indicating successful initialization of both reducers.
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(visibility_type* in) override;
        /**
         * @brief Initializes reducers and checks their initialization status for out-of-place processing.
         * 
         * @param in Input visibility_type data
         * @param out Output visibility_type data
         * @return Boolean indicating successful initialization of both reducers.
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        /**
         * @brief Executes pol-product summation in-place and updates weights.
         * 
         * @param in Input visibility_type pointer for processing.
         * @return Boolean indicating successful execution of both operations.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(visibility_type* in) override;
        /**
         * @brief Copies input visibility data to output, pre-multiplies it, executes reducers and fixes labels.
         * 
         * @param in Input visibility data
         * @param out Output visibility data
         * @return Boolean indicating successful execution of reducers
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        weight_type* fWeights;

        //class(es) which do the summation
        MHO_Reducer< visibility_type, MHO_CompoundSum > fReducer;
        MHO_Reducer< weight_type, MHO_CompoundSum > fWReducer;

        //multiplies each pol product by the appropriate pre-factor
        /**
         * @brief Multiplies each polarization product by its appropriate pre-factor.
         * 
         * @param in Input visibility_type pointer.
         */
        void PreMultiply(visibility_type* in);

        /**
         * @brief Getter for prefactor
         * 
         * @param pp_label Input polarization product label (XX, YY, XY, YX)
         * @return Complex prefactor value
         */
        std::complex< double > GetPrefactor(std::string pp_label);

        void FixLabels(visibility_type* in);

        std::string fSummedPolProdLabel;
        std::vector< std::string > fPolProductSet;

        double fRefParAngle;
        double fRemParAngle;
        std::string fRefMountType;
        std::string fRemMountType;
        station_coord_type* fRefData;
        station_coord_type* fRemData;
};

} // namespace hops

#endif /*! end of include guard: MHO_PolProductSummation */
