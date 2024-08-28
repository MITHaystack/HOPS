#ifndef MHO_PolProductSummation_HH__
#define MHO_PolProductSummation_HH__



#include <cmath>
#include <complex>
#include <vector>
#include <map>
#include <cctype>

#include "MHO_Message.hh"
#include "MHO_Constants.hh"

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"
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

class MHO_PolProductSummation: public MHO_UnaryOperator< visibility_type >
{
    public:

        MHO_PolProductSummation();
        virtual ~MHO_PolProductSummation();

        //perhaps we should have a separate operator to handle the treatment of the weights?
        void SetWeights(weight_type* w){fWeights = w;}

        void SetPolProductSumLabel(std::string ppl){fSummedPolProdLabel = ppl;}
        void SetPolProductSet(std::vector< std::string >& pp_vec){ fPolProductSet = pp_vec;};

        //these data objects are not used yet, but could be needed if we want to apply
        //time-dependence to the pol-product pre-factors
        void SetReferenceStationCoordinateData(station_coord_type* ref_data){fRefData = ref_data;};
        void SetRemoteStationCoordinateData(station_coord_type* rem_data){fRemData = rem_data;};

        //parallactic angle values for each station (expects degrees)
        void SetReferenceParallacticAngle(double p){fRefParAngle = p;}
        void SetRemoteParallacticAngle(double p){fRemParAngle = p;}

        //not currently used (but needed for circ-circ pol sum)
        void SetReferenceMountType(std::string mt){fRefMountType = mt;}
        void SetRemoteMountType(std::string mt){fRemMountType = mt;}

    protected:

        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:

        weight_type* fWeights;

        //class(es) which do the summation
        MHO_Reducer< visibility_type, MHO_CompoundSum> fReducer;
        MHO_Reducer< weight_type, MHO_CompoundSum> fWReducer;

        //multiplies each pol product by the appropriate pre-factor
        void PreMultiply(visibility_type* in);

        std::complex<double> GetPrefactor(std::string pp_label);

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


}


#endif /*! end of include guard: MHO_PolProductSummation */
