#ifndef MHO_DelayModel_HH__
#define MHO_DelayModel_HH__

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"

namespace hops 
{

class MHO_DelayModel
{
    public:
        MHO_DelayModel();
        virtual ~MHO_DelayModel();
        
        void SetFourfitReferenceTimeVexString(std::string fourfit_reftime_string){fRefTimeString = fourfit_reftime_string;};
        void SetReferenceStationData(station_coord_type* ref_data){fRefData = ref_data;};
        void SetRemoteStationData(station_coord_type* rem_data){fRemData = rem_data;};
        void ComputeModel();

        double GetDelay(){return fDelay;}
        double GetRate(){return fRate;};
        double GetAcceleration(){return fAccel;};

        void SetReferenceStationClockOffset(double clock_off){fRefClockOff = clock_off;}
        void SetReferenceStationClockRate(double clock_rate){fRefClockRate = clock_rate;}
    
        double GetRefDelay(){return fRefDelay;}
        double GetRefRate(){return fRefRate;};
        double GetRefStationDelay(){return fRefStationDelay;}
        // double GetRefAcceleration(){return fRefAccel;};

    private:

        //evalute the delay model (delay, rate, accel) from the spline coefficients
        //XCoeffVectorType is expected to be an MHO_NDArrayView type
        template< typename XCoeffVectorType >
        void EvaluateDelaySpline(const XCoeffVectorType& coeff, double delta_t, double* results);

        //clamp selected interval between [0, n_intervals-1]
        void CheckSplineInterval(int n_intervals, double tdiff, int& int_no, std::string station_id);

        template< typename XTagType >
        XTagType RetrieveTag(station_coord_type* data, std::string key);

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


template< typename XCoeffVectorType >
void
MHO_DelayModel::EvaluateDelaySpline(const XCoeffVectorType& coeff, double delta_t, double* results)
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
    for(int p=0; p<n_coeff; p++)
    {
        c = coeff(p);
        tp = std::pow(delta_t, p);
        tpm1 = std::pow(delta_t, p-1);
        tpm2 = std::pow(delta_t, p-2);
        results[DELAY_INDEX] += c*tp;
        results[RATE_INDEX] += p*c*tpm1;
        results[ACCEL_INDEX] += p*(p-1)*c*tpm2;
    }
}


template< typename XTagType >
XTagType 
MHO_DelayModel::RetrieveTag(station_coord_type* data, std::string key)
{
    //get the ref/rem station codes
    XTagType value;
    bool ok = data->Retrieve(key, value);
    if(!ok)
    {
        msg_fatal("calibration", "data tag with key: "<< key <<" is missing from station data." << eom);
        std::exit(1);
    }
    return value;
}


}//end of namespace

#endif /* end of include guard: MHO_DelayModel_HH__ */
