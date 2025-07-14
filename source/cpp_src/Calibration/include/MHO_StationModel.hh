#ifndef MHO_StationModel_HH__
#define MHO_StationModel_HH__

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"

namespace hops
{

/*!
 *@file MHO_StationModel.hh
 *@class MHO_StationModel
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Nov 16 11:49:39 2023 -0500
 *@brief evaluates the station a priori coordinate and/or delay spline polynomials
 */

/**
 * @brief Class MHO_StationModel
 */
class MHO_StationModel
{
    public:
        MHO_StationModel();
        virtual ~MHO_StationModel();

        /**
         * @brief Setter for (model) evaluation time vex string
         * 
         * @param time_string Input time string to set
         */
        void SetEvaluationTimeVexString(std::string time_string) { fEvalTimeString = time_string; };

        /**
         * @brief Setter for station data
         * 
         * @param ref_data Input pointer to station_coord_type structure
         */
        void SetStationData(station_coord_type* ref_data) { fData = ref_data; };

        /**
         * @brief Calculates station model parameters such as delay, azimuth, elevation, etc.
         */
        void ComputeModel();

        /**
         * @brief Getter for delay
         * 
         * @return Current delay value as a double.
         */
        double GetDelay() { return fDelay; }

        /**
         * @brief Getter for azimuth
         * 
         * @return Current azimuth value as a double.
         */
        double GetAzimuth() { return fAzimuth; }

        /**
         * @brief Getter for elevation
         * 
         * @return Current elevation as a double.
         */
        double GetElevation() { return fElevation; };

        /**
         * @brief Getter for parallactic angle
         * 
         * @return The current parallactic angle as a double.
         */
        double GetParallacticAngle() { return fParAngle; };

        /**
         * @brief Getter for u coordinate
         * 
         * @return Current U-coordinate value as a double
         */
        double GetUCoordinate() { return fU; };

        /**
         * @brief Getter for v coordinate
         * 
         * @return Current V-coordinate as a double.
         */
        double GetVCoordinate() { return fV; };

        /**
         * @brief Getter for w coordinate
         * 
         * @return Current value of W-coordinate as a double
         */
        double GetWCoordinate() { return fW; };

    private:
        //evalute the coordinate model (az, el, u, v, etc) from the spline coefficients
        //only does this a single coordinate at a time!
        //XCoeffVectorType is expected to be an MHO_NDArrayView type
        /**
         * @brief Evaluates a spline at a given time using provided coefficients for a single coordinate.
         * 
         * @tparam XCoeffVectorType Template parameter XCoeffVectorType
         * @param coeff Input spline coefficients of type XCoeffVectorType
         * @param delta_t Time at which to evaluate the spline
         * @param results (double&)
         */
        template< typename XCoeffVectorType >
        void EvaluateSpline(const XCoeffVectorType& coeff, double delta_t, double& results);

        //clamp selected interval between [0, n_intervals-1]
        /**
         * @brief Checks spline interval validity and sets int_no accordingly.
         * 
         * @param n_intervals Number of intervals in the spline
         * @param tdiff Time difference for evaluation
         * @param int_no Reference to interval number
         * @param station_id ID of the station
         */
        void CheckSplineInterval(int n_intervals, double tdiff, int& int_no, std::string station_id);

        /**
         * @brief Retrieves a tag value from station data using the given key.
         * 
         * @param data Pointer to station coordinate data.
         * @param key Key string used to lookup the tag.
         * @return The retrieved tag value of type XTagType.
         */
        template< typename XTagType > XTagType RetrieveTag(station_coord_type* data, std::string key);

        //necessary data
        std::string fEvalTimeString;
        station_coord_type* fData;

        double fDelay;
        double fAzimuth;
        double fElevation;
        double fParAngle;
        double fU;
        double fV;
        double fW;
};

/**
 * @brief Evaluates a spline at a given time using provided coefficients.
 * 
 * @tparam XCoeffVectorType Template parameter XCoeffVectorType
 * @param coeff Input spline coefficients of type XCoeffVectorType
 * @param delta_t Time at which to evaluate the spline
 * @param result (double&)
 */
template< typename XCoeffVectorType >
void MHO_StationModel::EvaluateSpline(const XCoeffVectorType& coeff, double delta_t, double& result)
{
    //evaluates the spline at the given time delta_t,
    //does this a single coordinate at a time
    result = 0.0;
    int n_coeff = coeff.GetSize();
    double tp, c;
    for(int p = 0; p < n_coeff; p++)
    {
        c = coeff(p);
        tp = std::pow(delta_t, p);
        result += c * tp;
    }
}

/**
 * @brief Retrieves a tag value from station data using the provided key.
 * 
 * @param data Pointer to station coordinate data.
 * @param key Key string used to lookup the desired tag.
 * @return The retrieved tag value of type XTagType.
 */
template< typename XTagType > XTagType MHO_StationModel::RetrieveTag(station_coord_type* data, std::string key)
{
    //get the ref/rem station codes
    XTagType value;
    bool ok = data->Retrieve(key, value);
    if(!ok)
    {
        msg_fatal("calibration", "data tag with key: " << key << " is missing from station data." << eom);
        std::exit(1);
    }
    return value;
}

} // namespace hops

#endif /*! end of include guard: MHO_StationModel_HH__ */
