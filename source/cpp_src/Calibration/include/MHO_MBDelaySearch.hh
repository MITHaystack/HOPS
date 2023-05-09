#ifndef MHO_MBDelaySearch_HH__
#define MHO_MBDelaySearch_HH__


/*
*File: MHO_MBDelaySearch.hh
*Class: MHO_MBDelaySearch
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <cmath>
#include <complex>

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"

#include "MHO_InspectingOperator.hh"
#include "MHO_CyclicRotator.hh"

#ifdef HOPS_USE_FFTW3
    #include "MHO_FFTWTypes.hh"
    #include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#else
    #include "MHO_FastFourierTransform.hh"
#endif

#include "MHO_MultidimensionalFastFourierTransform.hh"

#include "MHO_UniformGridPointsCalculator.hh"
#include "MHO_ExtremaSearch.hh"

namespace hops
{


class MHO_MBDelaySearch: public MHO_InspectingOperator< visibility_type >
{
    public:
        MHO_MBDelaySearch();
        virtual ~MHO_MBDelaySearch();
        
        int GetMBDMaxBin() const {return fMBDMaxBin;}
        int GetSBDMaxBin() const {return fSBDMaxBin;}
        int GetDRMaxBin() const {return fDRMaxBin;}

        //TODO FIX ME
        time_axis_type* GetMBDAxis(){ return &(std::get<0>(fMBDWorkspace)); };
        delay_rate_axis_type* GetDRAxis(){ return &(std::get<1>(fMBDWorkspace)); };

    protected:

        using XArgType = visibility_type;

        virtual bool InitializeImpl(const XArgType* in) override;
        virtual bool ExecuteImpl(const XArgType* in) override;

    private:

        //workspace
        bool fInitialized;
        std::vector< double > fChannelFreqs;
        mbd_dr_type fMBDWorkspace;
        mbd_dr_amp_type fMBDAmpWorkspace;

        //dims and parameters
        double fGridStart;
        double fGridSpace;
        std::size_t fNGridPoints;
        std::size_t fNSBD; 
        std::size_t fNDR;
        std::map<std::size_t, std::size_t> fMBDBinMap;

        //location of the maximum
        int fMBDMaxBin;
        int fSBDMaxBin;
        int fDRMaxBin;

        MHO_UniformGridPointsCalculator fGridCalc;

        MHO_MultidimensionalFastFourierTransform< mbd_dr_type > fFFTEngine;
        MHO_CyclicRotator< mbd_dr_type > fCyclicRotator;

        MHO_ExtremaSearch< mbd_dr_amp_type > fMaxSearch;

};


}



#endif /* end of include guard: MHO_MBDelaySearch_HH__ */
