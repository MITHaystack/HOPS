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

#include "MHO_AxisPack.hh"
#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_InspectingOperator.hh"
#include "MHO_CyclicRotator.hh"
#include "MHO_UniformGridPointsCalculator.hh"
#include "MHO_ExtremaSearch.hh"

namespace hops
{

using mbd_axis_pack = MHO_AxisPack< time_axis_type >;
using mbd_type = MHO_TableContainer< visibility_element_type, mbd_axis_pack >;
using mbd_amp_type = MHO_TableContainer< double, mbd_axis_pack >;

}

#ifdef HOPS_USE_FFTW3
#include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#define FFT_ENGINE_TYPE MHO_MultidimensionalFastFourierTransformFFTW< mbd_type >
#else
#include "MHO_MultidimensionalFastFourierTransform.hh"
#define FFT_ENGINE_TYPE MHO_MultidimensionalFastFourierTransform< mbd_type >
#endif


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
        // time_axis_type* GetMBDAxis(){ return &(std::get<0>(fMBDWorkspace)); };
        //delay_rate_axis_type* GetDRAxis(){ return &(std::get<1>(fMBDWorkspace)); };

        time_axis_type* GetMBDAxis(){ return &fMBDAxis; };
        delay_rate_axis_type* GetDRAxis(){ return &fDRAxis; };



    protected:

        using XArgType = visibility_type;

        virtual bool InitializeImpl(const XArgType* in) override;
        virtual bool ExecuteImpl(const XArgType* in) override;

    private:


        //workspace
        bool fInitialized;
        std::vector< double > fChannelFreqs;
        mbd_type fMBDWorkspace;
        mbd_amp_type fMBDAmpWorkspace;

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

        MHO_Axis<double> fMBDAxis;
        MHO_Axis<double> fDRAxis;

        MHO_UniformGridPointsCalculator fGridCalc;

        FFT_ENGINE_TYPE fFFTEngine;

        MHO_CyclicRotator< mbd_type > fCyclicRotator;

        MHO_ExtremaSearch< mbd_amp_type > fMaxSearch;

};


}



#endif /* end of include guard: MHO_MBDelaySearch_HH__ */
