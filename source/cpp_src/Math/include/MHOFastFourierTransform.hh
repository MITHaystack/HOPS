#ifndef MHOFastFourierTransform_HH__
#define MHOFastFourierTransform_HH__

#include <complex>

#include "MHOArrayWrapper.hh"
#include "MHOUnaryArrayOperator.hh"
#include "MHOBitReversalPermutation.hh"
#include "MHOFastFourierTransformUtilities.hh"

namespace hops
{

class MHOFastFourierTransform: public MHOUnaryArrayOperator< std::complex<double>, 1 >
{
    public:

        MHOFastFourierTransform();
        virtual ~MHOFastFourierTransform();

        virtual void SetSize(unsigned int N);

        virtual void SetForward();
        virtual void SetBackward();

        virtual void Initialize();

        virtual void ExecuteOperation();

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


#endif /* MHOFastFourierTransform_H__ */
