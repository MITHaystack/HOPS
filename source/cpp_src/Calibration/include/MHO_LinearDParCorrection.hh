#ifndef MHO_LinearDParCorrection_HH__
#define MHO_LinearDParCorrection_HH__

/*
*File: MHO_LinearDParCorrection.hh
*Class: MHO_LinearDParCorrection
*Author:
*Email:
*Date:
*Description:
*/

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


class MHO_LinearDParCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:

        MHO_LinearDParCorrection();
        virtual ~MHO_LinearDParCorrection();

        void SetPolProductSet(std::vector< std::string >& pp_vec){ fPolProductSet = pp_vec;};

        // //these data objects are not used yet, but could be needed if we want to apply 
        // //time-dependence to the pol-product pre-factors
        // void SetReferenceStationCoordinateData(station_coord_type* ref_data){fRefData = ref_data;};
        // void SetRemoteStationCoordinateData(station_coord_type* rem_data){fRemData = rem_data;};

        //parallactic angle values for each station (expects degrees)
        void SetReferenceParallacticAngle(double p){fRefParAngle = p;}
        void SetRemoteParallacticAngle(double p){fRemParAngle = p;}
        
        // //not currently used (but needed for circ-circ pol sum)
        // void SetReferenceMountType(std::string mt){fRefMountType = mt;}
        // void SetRemoteMountType(std::string mt){fRemMountType = mt;}

    protected:

        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        

        //multiplies each pol product by the appropriate pre-factor
        void PreMultiply(visibility_type* in);

        std::complex<double> GetPrefactor(std::string pp_label);

        std::vector< std::string > fPolProductSet;

        double fRefParAngle;
        double fRemParAngle;
        std::string fRefMountType;
        std::string fRemMountType;

};


}


#endif /* end of include guard: MHO_LinearDParCorrection */
