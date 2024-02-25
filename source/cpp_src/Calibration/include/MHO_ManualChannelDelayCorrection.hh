#ifndef MHO_ManualChannelDelayCorrection_HH__
#define MHO_ManualChannelDelayCorrection_HH__



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



namespace hops
{

/*!
*@file MHO_ManualChannelDelayCorrection.hh
*@class MHO_ManualChannelDelayCorrection
*@author J. Barrett - barrettj@mit.edu
*@date Thu Jan 27 10:36:00 2022 -0500
*@brief
*/

class MHO_ManualChannelDelayCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:

        MHO_ManualChannelDelayCorrection();
        virtual ~MHO_ManualChannelDelayCorrection();

        void SetStation(std::string station){fStationCode = station;}; //2-char station code
        void SetStationMk4ID(std::string station_id){fMk4ID = station_id;} //1-char mk4id
        void SetPolarization(const std::string& pol){fPol = pol; make_upper(fPol);};

         //channel label -> pc_phases
        void SetChannelToPCDelayMap(const std::map< std::string, double >& map){fPCDelayMap = map;};

    protected:

        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:

        std::size_t DetermineStationIndex(const visibility_type* in);
        bool PolMatch(std::size_t station_idx, std::string& polprod);

        //constants
        std::complex<double> fImagUnit;
        double fDegToRad;
        double fNanoSecToSecond;
        double fMHzToHz;
        double fPi;

        //selection
        std::string fStationCode;
        std::string fMk4ID;
        std::string fPol;

        //channel label -> pc delay
        std::map< std::string, double > fPCDelayMap;

        //keys for tag retrieval
        std::string fStationKey;
        std::string fRemStationKey;
        std::string fRefStationKey;
        std::string fRemStationMk4IDKey;
        std::string fRefStationMk4IDKey;
        std::string fChannelLabelKey;
        std::string fBandwidthKey;

        //minor helper function to make sure all strings are compared as upper-case only
        void make_upper(std::string& s){ for(char& c : s){c = toupper(c); };
    }


};


}


#endif /*! end of include guard: MHO_ManualChannelDelayCorrection */
