#ifndef MHO_ManualPolDelayCorrection_HH__
#define MHO_ManualPolDelayCorrection_HH__

/*
*File: MHO_ManualPolDelayCorrection.hh
*Class: MHO_ManualPolDelayCorrection
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


namespace hops
{


class MHO_ManualPolDelayCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:

        MHO_ManualPolDelayCorrection();
        virtual ~MHO_ManualPolDelayCorrection();

        void SetReferenceFrequency(double ref_freq){fRefFreq = ref_freq;}
        void SetStation(std::string station){fStationCode = station;}; //2-char station code
        void SetStationMk4ID(std::string station_id){fMk4ID = station_id;} //1-char mk4id
        void SetPolarization(const std::string& pol){fPol = pol; make_upper(fPol);};
        void SetPCDelayOffset(const double& pc_delay_offset){fDelayOffset = pc_delay_offset;}

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
        double fNanoSecToSecond;
        double fMHzToHz;
        double fPi;

        //selection
        std::string fStationCode;
        std::string fMk4ID;
        std::string fPol;
        
        //ref freq and pc delay
        double fRefFreq;
        double fDelayOffset;

        //keys for tag retrieval
        std::string fStationKey;
        std::string fRemStationKey;
        std::string fRefStationKey;
        std::string fRemStationMk4IDKey;
        std::string fRefStationMk4IDKey;

        //minor helper function to make sure all strings are compared as upper-case only
        void make_upper(std::string& s){ for(char& c : s){c = toupper(c); };
    }


};


}


#endif /* end of include guard: MHO_ManualPolDelayCorrection */
