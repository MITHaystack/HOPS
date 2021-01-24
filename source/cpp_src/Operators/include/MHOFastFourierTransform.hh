#ifndef MHO_FastFourierTransform_HH__
#define MHO_FastFourierTransform_HH__

#include <complex>

#include "MHO_NDArrayWrapper.hh"
#include "MHO_NDArrayOperator.hh"
#include "MHO_BitReversalPermutation.hh"
#include "MHO_FastFourierTransformUtilities.hh"

namespace hops
{

typedef MHO_NDArrayWrapper< std::complex<double>, 1> fft_c2c_type;

class MHO_FastFourierTransform: 
    public MHO_NDArrayOperator< MHO_NDArrayWrapper< std::complex<double>, 1>,
                             MHO_NDArrayWrapper< std::complex<double>, 1> >
{
    public:

        MHO_FastFourierTransform();
        virtual ~MHO_FastFourierTransform();

        virtual void SetSize(unsigned int N);

        virtual void SetForward();
        virtual void SetBackward();

        virtual bool Initialize() override;
        virtual bool ExecuteOperation() override;

    private:

        virtual void AllocateWorkspace();
        virtual void DealocateWorkspace();

        bool fIsValid;
        bool fForward;
        bool fInitialized;
        bool fSizeIsPowerOfTwo;

        //auxilliary workspace needed for basic 1D transform
        unsigned int fN;
        unsigned int fM;
        unsigned int* fPermutation;
        std::complex<double>* fTwiddle;
        std::complex<double>* fConjugateTwiddle;
        std::complex<double>* fScale;
        std::complex<double>* fCirculant;
        std::complex<double>* fWorkspace;

};

}


#endif /* MHO_FastFourierTransform_H__ */
