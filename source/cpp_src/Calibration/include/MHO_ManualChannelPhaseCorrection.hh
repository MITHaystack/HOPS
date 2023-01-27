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

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_BinaryOperator.hh"
#include "MHO_Message.hh"


namespace hops
{


class MHO_ManualChannelPhaseCorrection: public MHO_BinaryOperator<
    ch_visibility_type,
    manual_pcal_type,
    ch_visibility_type >
{
    public:

        MHO_ManualChannelPhaseCorrection();
        virtual ~MHO_ManualChannelPhaseCorrection();

        using XArgType1 = ch_visibility_type;
        using XArgType2 = manual_pcal_type;
        using XArgType3 = ch_visibility_type;

        virtual bool InitializeImpl(const XArgType1* in_vis, const XArgType2* pcal, XArgType3* out_vis) override;
        virtual bool ExecuteImpl(const XArgType1* in_vis, const XArgType2* pcal, XArgType3* out_vis) override;

    private:

        bool fInitialized;

        //TODO FIXME migrate these to a constants header
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
