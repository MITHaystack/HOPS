#ifndef MHO_ComputePlotData_HH__
#define MHO_ComputePlotData_HH__



#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"

#include "MHO_BinaryOperator.hh"
#include "MHO_NaNMasker.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_ComplexConjugator.hh"
#include "MHO_CyclicRotator.hh"
#include "MHO_SubSample.hh"
#include "MHO_FringeRotation.hh"

#ifdef HOPS_USE_FFTW3
    #include "MHO_FFTWTypes.hh"
    #include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#else
    #include "MHO_FastFourierTransform.hh"
#endif


#include "MHO_MultidimensionalFastFourierTransform.hh"
#include "MHO_MultidimensionalPaddedFastFourierTransform.hh"



namespace hops
{

class MHO_ComputePlotData
{
    public:
        MHO_ComputePlotData(){};
        virtual ~MHO_ComputePlotData(){};

        void SetVisibilities();

        void SetReferenceFrequency(double ref_freq){fRefFreq = ref_freq;}
        void SetMBDelay(double mbdelay){fMBDelay = mbdelay;}
        void SetDelayRate(double dr){fDelayRate = dr;}

        void SetSBDArray(visibility_type* sbd_arr){fSBDArray = sbd_arr;}
        void SetWeights(weight_type* weights){fWeights = weights;}




    protected:

        //void calculate();

        void calc_mbd();
        void calc_sbd();
        void calc_dr();

        double fRefFreq;
        double fTotalSummedWeights;
        double fMBDelay;
        double fDelayRate;

        visibility_type* fSBDArray;
        weight_type* fWeights;

};

}

#endif /* end of include guard: MHO_ComputePlotData_HH__ */
