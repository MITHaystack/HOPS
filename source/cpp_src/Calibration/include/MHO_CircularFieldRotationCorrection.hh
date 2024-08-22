#ifndef MHO_CircularFieldRotationCorrection_HH__
#define MHO_CircularFieldRotationCorrection_HH__

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
#include "MHO_StationModel.hh"

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


class MHO_CircularFieldRotationCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:

        MHO_CircularFieldRotationCorrection();
        virtual ~MHO_CircularFieldRotationCorrection();

        void SetPolProductSet(std::vector< std::string >& pp_vec){ fPolProductSet = pp_vec;};

        void SetFourfitReferenceTimeVexString(std::string frt_vex_string){fFourfitRefTimeString = frt_vex_string;};
        //these data objects are not yet used to the fullest extent,
        //but they could be if we want to apply a time-dependant corrections
        //to the pol-product pre-factors
        void SetReferenceStationCoordinateData(station_coord_type* ref_data){fRefData = ref_data;};
        void SetRemoteStationCoordinateData(station_coord_type* rem_data){fRemData = rem_data;};

        //parallactic angle values for each station (expects degrees)
        void SetReferenceParallacticAngle(double p){fRefParAngle = p;}
        void SetRemoteParallacticAngle(double p){fRemParAngle = p;}

        //set the station mount types
        void SetReferenceMountType(std::string mt){fRefMountType = mt;}
        void SetRemoteMountType(std::string mt){fRemMountType = mt;}

    protected:

        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:

        int DetermineMountCode(const std::string& mount) const;

        //multiplies each pol product by the appropriate pre-factor
        void PreMultiply(visibility_type* in);

        std::complex<double> GetPrefactor(std::string pp_label);

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


}


#endif /*! end of include guard: MHO_CircularFieldRotationCorrection */
