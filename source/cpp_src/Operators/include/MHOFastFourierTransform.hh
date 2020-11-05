#ifndef MHOFastFourierTransform_HH__
#define MHOFastFourierTransform_HH__

#include <complex>

#include "MHOArrayWrapper.hh"
#include "MHOArrayOperator.hh"
#include "MHOBitReversalPermutation.hh"
#include "MHOFastFourierTransformUtilities.hh"

namespace hops
{

typedef MHOArrayWrapper< std::complex<double>, 1> fft_c2c_type;

class MHOFastFourierTransform: 
    public MHOArrayOperator< MHOArrayWrapper< std::complex<double>, 1>,
                             MHOArrayWrapper< std::complex<double>, 1> >
{
    public:

        MHOFastFourierTransform();
        virtual ~MHOFastFourierTransform();

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


#endif /* MHOFastFourierTransform_H__ */
