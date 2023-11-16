#ifndef MHO_StationModel_HH__
#define MHO_StationModel_HH__

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"

namespace hops 
{

class MHO_StationModel
{
    public:
        MHO_StationModel();
        virtual ~MHO_StationModel();
        
        void SetEvaluationTimeVexString(std::string time_string){fEvalTimeString = time_string;};
        void SetStationData(station_coord_type* ref_data){fData = ref_data;};

        void ComputeModel();
        
        double GetDelay(){return fDelay;}
        double GetAzimuth(){return fAzimuth;}
        double GetElevation(){return fElevation;};
        double GetParallacticAngle(){return fParAngle;};
        double GetUCoordinate(){return fU;};
        double GetVCoordinate(){return fV;};
        double GetWCoordinate(){return fW;};

    private:

        //evalute the coordinate model (az, el, u, v, etc) from the spline coefficients
        //only does this a single coordinate at a time!
        //XCoeffVectorType is expected to be an MHO_NDArrayView type
        template< typename XCoeffVectorType >
        void EvaluateSpline(const XCoeffVectorType& coeff, double delta_t, double& results);

        //clamp selected interval between [0, n_intervals-1]
        void CheckSplineInterval(int n_intervals, double tdiff, int& int_no, std::string station_id);

        template< typename XTagType >
        XTagType RetrieveTag(station_coord_type* data, std::string key);

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


template< typename XCoeffVectorType >
void
MHO_StationModel::EvaluateSpline(const XCoeffVectorType& coeff, double delta_t, double& result)
{
    //evaluates the spline at the given time delta_t,
    //does this a single coordinate at a time
    result = 0.0;
    int n_coeff = coeff.GetSize();
    double tp, c;
    for(int p=0; p<n_coeff; p++)
    {
        c = coeff(p);
        tp = std::pow(delta_t, p);
        result += c*tp;
    }
}


template< typename XTagType >
XTagType 
MHO_StationModel::RetrieveTag(station_coord_type* data, std::string key)
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

#endif /* end of include guard: MHO_StationModel_HH__ */
