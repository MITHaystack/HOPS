#ifndef MHO_ManualPolPhaseCorrection_HH__
#define MHO_ManualPolPhaseCorrection_HH__

/*
*File: MHO_ManualPolPhaseCorrection.hh
*Class: MHO_ManualPolPhaseCorrection
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


class MHO_ManualPolPhaseCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:

        MHO_ManualPolPhaseCorrection();
        virtual ~MHO_ManualPolPhaseCorrection();

        void SetStation(std::string station){fStationCode = station;}; //2-char station code
        void SetStationMk4ID(std::string station_id){fMk4ID = station_id;} //1-char mk4id
        void SetPolarization(const std::string& pol){fPol = pol; make_upper(fPol);};
        void SetPCPhaseOffset(const double& pc_phase_offset){fPhaseOffset = pc_phase_offset;}

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

        //selection
        std::string fStationCode;
        std::string fMk4ID;
        std::string fPol;

        //pc rotation
        double fPhaseOffset;

        //keys for tag retrieval
        std::string fStationKey;
        std::string fRemStationKey;
        std::string fRefStationKey;
        std::string fRemStationMk4IDKey;
        std::string fRefStationMk4IDKey;
        std::string fChannelLabelKey;

        //minor helper function to make sure all strings are compared as upper-case only
        void make_upper(std::string& s){ for(char& c : s){c = toupper(c); };
    }


};


}


#endif /* end of include guard: MHO_ManualPolPhaseCorrection */
