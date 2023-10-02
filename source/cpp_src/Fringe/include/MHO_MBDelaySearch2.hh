#ifndef MHO_MBDelaySearch2_HH__
#define MHO_MBDelaySearch2_HH__


/*
*File: MHO_MBDelaySearch2.hh
*Class: MHO_MBDelaySearch2
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

#ifdef HOPS_USE_FFTW3
#include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#else
#include "MHO_MultidimensionalFastFourierTransform.hh"
#endif

namespace hops
{

using mbd_axis_pack = MHO_AxisPack< time_axis_type >;
using mbd_type = MHO_TableContainer< visibility_element_type, mbd_axis_pack >;
using mbd_amp_type = MHO_TableContainer< double, mbd_axis_pack >;




class MHO_MBDelaySearch2: public MHO_InspectingOperator< visibility_type >
{
    public:
        MHO_MBDelaySearch2();
        virtual ~MHO_MBDelaySearch2();

        void SetWeights(weight_type* wt_data){fWeights = wt_data;}
        void SetReferenceFrequency(double ref_freq){fRefFreq = ref_freq;}

        int GetMBDMaxBin() const {return fMBDMaxBin;}
        int GetSBDMaxBin() const {return fSBDMaxBin;}
        int GetDRMaxBin() const {return fDRMaxBin;}

        double GetFrequencySpacing() const {return fGridSpace;}
        double GetAverageFrequency() const {return fAverageFreq;}

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

        visibility_type fSBDDrWorkspace;
        visibility_type sbd_dr_data;
        weight_type fWeightsWorkspace;
        weight_type* fWeights;

        mbd_type fMBDWorkspace;
        mbd_amp_type fMBDAmpWorkspace;

        //dims and parameters
        double fGridStart;
        double fGridSpace;
        double fAverageFreq;
        std::size_t fNGridPoints;
        std::size_t fNSBD;
        std::size_t fNDR;
        std::map<std::size_t, std::size_t> fMBDBinMap;

        double fRefFreq;

        //location of the maximum
        int fMBDMaxBin;
        int fSBDMaxBin;
        int fDRMaxBin;

        MHO_Axis<double> fMBDAxis;
        MHO_Axis<double> fDRAxis;

        MHO_UniformGridPointsCalculator fGridCalc;

        #ifdef HOPS_USE_FFTW3
        using FFT_ENGINE_TYPE =  MHO_MultidimensionalFastFourierTransformFFTW< mbd_type >;
        #else
        using FFT_ENGINE_TYPE =  MHO_MultidimensionalFastFourierTransform< mbd_type >;
        #endif

        FFT_ENGINE_TYPE fFFTEngine;

        MHO_CyclicRotator< mbd_type > fCyclicRotator;

        MHO_ExtremaSearch< mbd_amp_type > fMaxSearch;

};


}



#endif /* end of include guard: MHO_MBDelaySearch2_HH__ */
