#ifndef MHO_DelayModel_HH__
#define MHO_DelayModel_HH__

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"

namespace hops
{

/*!
 *@file MHO_DelayModel.hh
 *@class MHO_DelayModel
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Aug 18 13:42:45 2023 -0400
 *@brief evaluates the station a priori delay model polynomials
 */

/**
 * @brief Class MHO_DelayModel
 */
class MHO_DelayModel
{
    public:
        MHO_DelayModel();
        virtual ~MHO_DelayModel();

        /**
         * @brief Setter for fourfit reference time vex string
         * 
         * @param fourfit_reftime_string The new fourfit reference time vex string.
         */
        void SetFourfitReferenceTimeVexString(std::string fourfit_reftime_string) { fRefTimeString = fourfit_reftime_string; };

        /**
         * @brief Setter for reference station data
         * 
         * @param ref_data Pointer to station coordinate data
         */
        void SetReferenceStationData(station_coord_type* ref_data) { fRefData = ref_data; };

        /**
         * @brief Setter for remote station data
         * 
         * @param rem_data Pointer to remote station coordinate data
         */
        void SetRemoteStationData(station_coord_type* rem_data) { fRemData = rem_data; };

        /**
         * @brief Calculates delay model for reference and remote stations using fourfit reference time.
         */
        void ComputeModel();

        /**
         * @brief Getter for delay
         * 
         * @return The current delay value as a double.
         */
        double GetDelay() { return fDelay; }

        /**
         * @brief Getter for rate
         * 
         * @return The current rate as a double.
         */
        double GetRate() { return fRate; };

        /**
         * @brief Getter for acceleration
         * 
         * @return The current acceleration as a double.
         */
        double GetAcceleration() { return fAccel; };

        /**
         * @brief Setter for reference station clock offset
         * 
         * @param clock_off New clock offset value in seconds
         */
        void SetReferenceStationClockOffset(double clock_off) { fRefClockOff = clock_off; }

        /**
         * @brief Setter for reference station clock rate
         * 
         * @param clock_rate New clock rate value in double precision
         */
        void SetReferenceStationClockRate(double clock_rate) { fRefClockRate = clock_rate; }

        /**
         * @brief Getter for ref delay
         * 
         * @return The current reference delay as a double.
         */
        double GetRefDelay() { return fRefDelay; }

        /**
         * @brief Getter for ref rate
         * 
         * @return The current reference rate as a double.
         */
        double GetRefRate() { return fRefRate; };

        /**
         * @brief Getter for ref station delay
         * 
         * @return The current reference station delay as a double.
         */
        double GetRefStationDelay() { return fRefStationDelay; }

        // double GetRefAcceleration(){return fRefAccel;};

    private:
        //evalute the delay model (delay, rate, accel) from the spline coefficients
        //XCoeffVectorType is expected to be an MHO_NDArrayView type
        /**
         * @brief Evaluates delay spline model using given coefficients and time delta.
         * 
         * @tparam XCoeffVectorType Template parameter XCoeffVectorType
         * @param coeff Input coefficient vector of type XCoeffVectorType
         * @param delta_t Time difference for evaluation
         * @param results Output array to store delay, rate, and acceleration
         */
        template< typename XCoeffVectorType >
        void EvaluateDelaySpline(const XCoeffVectorType& coeff, double delta_t, double* results);

        //clamp selected interval between [0, n_intervals-1]
        /**
         * @brief Checks spline interval and adjusts int_no if necessary.
         * 
         * @param n_intervals Number of intervals in the spline.
         * @param tdiff Time difference for reference time.
         * @param int_no Reference integer number, adjusted if needed.
         * @param station_id Identifier of the station.
         */
        void CheckSplineInterval(int n_intervals, double tdiff, int& int_no, std::string station_id);

        /**
         * @brief Retrieves a tag value from station data using a given key.
         * 
         * @param data Pointer to station coordinate data.
         * @param key Key string used to retrieve the tag.
         * @return The retrieved tag value of type XTagType.
         */
        template< typename XTagType > XTagType RetrieveTag(station_coord_type* data, std::string key);

        //necessary data
        std::string fRefTimeString;
        station_coord_type* fRefData;
        station_coord_type* fRemData;

        //results
        double fDelay;
        double fRate;
        double fAccel;

        //ref station
        double fRefClockOff;
        double fRefClockRate;

        double fRefDelay;
        double fRefRate;
        double fRefStationDelay;
};

/**
 * @brief Function MHO_DelayModel::EvaluateDelaySpline
 * 
 * @tparam XCoeffVectorType Template parameter XCoeffVectorType
 * @param coeff (const XCoeffVectorType&)
 * @param delta_t (double)
 * @param results (double*)
 */
template< typename XCoeffVectorType >
void MHO_DelayModel::EvaluateDelaySpline(const XCoeffVectorType& coeff, double delta_t, double* results)
{
#define DELAY_INDEX 0
#define RATE_INDEX 1
#define ACCEL_INDEX 2
#define DELAY_COEFF_INDEX 0

    //compute delay, rate accel
    results[DELAY_INDEX] = 0.0;
    results[RATE_INDEX] = 0.0;
    results[ACCEL_INDEX] = 0.0;
    int n_coeff = coeff.GetSize();
    double tp, tpm1, tpm2, c;
    for(int p = 0; p < n_coeff; p++)
    {
        c = coeff(p);
        tp = std::pow(delta_t, p);
        tpm1 = 0.0;
        tpm2 = 0.0;

        //std::cout<<"-----------------"<<std::endl;
        //std::cout<<"p="<<p<<std::endl;
        //std::cout<<"c="<<c<<std::endl;
        //std::cout<<"tpm1="<<tpm1<<std::endl;
        //std::cout<<"tpm2="<<tpm2<<std::endl;

        results[DELAY_INDEX] += c * tp;
        if(p >= 1)
        {
            tpm1 = std::pow(delta_t, p - 1);
            results[RATE_INDEX] += p * c * tpm1;
        }

        if(p >= 2)
        {
            tpm2 = std::pow(delta_t, p - 2);
            results[ACCEL_INDEX] += p * (p - 1) * c * tpm2;
        }
    }
}

/**
 * @brief Retrieves a tag value from station data using a given key.
 * 
 * @param data Pointer to station coordinate data.
 * @param key Key string used to lookup the tag.
 * @return The retrieved tag value of type XTagType.
 */
template< typename XTagType > XTagType MHO_DelayModel::RetrieveTag(station_coord_type* data, std::string key)
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

#endif /*! end of include guard: MHO_DelayModel_HH__ */
