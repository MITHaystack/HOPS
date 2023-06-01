#ifndef MHO_ManualChannelPhaseCorrection_HH__
#define MHO_ManualChannelPhaseCorrection_HH__

/*
*File: MHO_ManualChannelPhaseCorrection.hh
*Class: MHO_ManualChannelPhaseCorrection
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


class MHO_ManualChannelPhaseCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:

        MHO_ManualChannelPhaseCorrection();
        virtual ~MHO_ManualChannelPhaseCorrection();

        void SetPhaseCorrections(const manual_pcal_type& man_pcal){fPCal = man_pcal;};

    protected:

        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:

        manual_pcal_type fPCal;

        bool fInitialized;

        //constants
        std::complex<double> fImagUnit;
        double fDegToRad;

        //keys for tag retrieval 
        std::string fStationKey;
        std::string fRemStationKey;
        std::string fRefStationKey;
        std::string fBaselineKey;
        std::string fNetSidebandKey;

        std::map< std::size_t, std::size_t> fPolIdxMap; //map pcal pol index to vis pol-product index
        std::map< std::size_t, std::size_t> fChanIdxMap; // map pcal chan index to vis chan index

        //minor helper function to make sure all strings are compared as upper-case only 
        void make_upper(std::string& s){ for(char& c : s){c = toupper(c); };
    }


};


}


#endif /* end of include guard: MHO_ManualChannelPhaseCorrection */
