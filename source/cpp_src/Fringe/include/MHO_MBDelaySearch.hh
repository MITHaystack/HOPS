#ifndef MHO_MBDelaySearch_HH__
#define MHO_MBDelaySearch_HH__


#include <cmath>
#include <complex>

#include "MHO_AxisPack.hh"
#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_InspectingOperator.hh"
#include "MHO_CyclicRotator.hh"
#include "MHO_UniformGridPointsCalculator.hh"
#include "MHO_ExtremaSearch.hh"
#include "MHO_DelayRate.hh"


#ifdef HOPS_USE_FFTW3
#include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#else
#include "MHO_MultidimensionalFastFourierTransform.hh"
#endif

namespace hops
{

/*!
*@file MHO_MBDelaySearch.hh
*@class MHO_MBDelaySearch
*@author J. Barrett - barrettj@mit.edu
*@date
*@brief implements the coarse MBD/SBD/DR search, see search.c
*/

using mbd_axis_pack = MHO_AxisPack< time_axis_type >;
using mbd_type = MHO_TableContainer< visibility_element_type, mbd_axis_pack >;
using mbd_amp_type = MHO_TableContainer< double, mbd_axis_pack >;


class MHO_MBDelaySearch: public MHO_InspectingOperator< visibility_type >
{
    public:
        MHO_MBDelaySearch();
        virtual ~MHO_MBDelaySearch();

        void SetWeights(weight_type* wt_data){fWeights = wt_data;}
        void SetReferenceFrequency(double ref_freq){fRefFreq = ref_freq;}

        //configure the search windows (using floating point limits)
        //default is the full range
        void SetSBDWindow(double low, double high);
        void SetMBDWindow(double low, double high);
        void SetDRWindow(double low, double high);

        //retrieve the window limits (that were actually used)
        void GetSBDWindow(double& low, double& high) const;
        void GetMBDWindow(double& low, double& high) const;
        void GetDRWindow(double& low, double& high) const;

        int GetMBDMaxBin() const {return fMBDMaxBin;}
        int GetSBDMaxBin() const {return fSBDMaxBin;}
        int GetDRMaxBin() const {return fDRMaxBin;}

        double GetCoarseMBD() const {return fCoarseMBD;}
        double GetCoarseSBD() const {return fCoarseSBD;}
        double GetCoarseDR() const {return fCoarseDR;}

        double GetSBDBinSeparation() const {return fSBDBinSep;}

        int GetNMBDBins(){return fNGridPoints;};
        int GetNSBDBins(){return fNSBD;};
        int GetNDRBins(){return fNDR;};
        int GetNDRSPBins(){return fDelayRateCalc.GetDelayRateSearchSpaceSize(); }

        double GetSearchMaximumAmplitude() const {return fMax;}
        double GetFrequencySpacing() const {return fGridSpace;}
        double GetAverageFrequency() const {return fAverageFreq;}

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
        int fSBDStart;
        int fSBDStop;

        double fSBDBinSep;
        double fDRBinSep;
        double fMBDBinSep;

        //the window limits
        double fSBDWin[2];
        double fMBDWin[2];
        double fDRWin[2];

        //indicates if window limits have been set
        bool fSBDWinSet;
        bool fMBDWinSet;
        bool fDRWinSet;

        //location and value of the maximum
        double fMax;
        int fMBDMaxBin;
        int fSBDMaxBin;
        int fDRMaxBin;

        double fCoarseMBD;
        double fCoarseSBD;
        double fCoarseDR;

        MHO_Axis<double> fSBDAxis;
        MHO_Axis<double> fMBDAxis;
        MHO_Axis<double> fDRAxis;

        MHO_UniformGridPointsCalculator fGridCalc;
        MHO_DelayRate fDelayRateCalc; //delay rate calculator

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



#endif /*! end of include guard: MHO_MBDelaySearch_HH__ */
