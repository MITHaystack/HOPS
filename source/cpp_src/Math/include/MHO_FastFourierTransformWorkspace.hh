#ifndef MHO_FastFourierTransformWorkspace_HH__
#define MHO_FastFourierTransformWorkspace_HH__

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>

#include "MHO_BitReversalPermutation.hh"
#include "MHO_FastFourierTransformUtilities.hh"
#include "MHO_Message.hh"

namespace hops
{

/*!
 *@file MHO_FastFourierTransformWorkspace.hh
 *@class MHO_FastFourierTransformWorkspace
 *@date Mon Jul 31 15:44:01 2023 -0400
 *@brief navtive FFT workspace definitions
 *@author J. Barrett - barrettj@mit.edu
 */

template< typename XFloatType > class MHO_FastFourierTransformWorkspace
{
    public:
        MHO_FastFourierTransformWorkspace()
        {
            fN = 0;
            fM = 0;
            fPermutation = nullptr;
            fTwiddle = nullptr;
            fConjugateTwiddle = nullptr;
            fScale = nullptr;
            fCirculant = nullptr;
            fWorkspace = nullptr;
        };

        MHO_FastFourierTransformWorkspace(unsigned int n)
        {
            fN = 0;
            fM = 0;
            fPermutation = nullptr;
            fTwiddle = nullptr;
            fConjugateTwiddle = nullptr;
            fScale = nullptr;
            fCirculant = nullptr;
            fWorkspace = nullptr;
            Resize(n);
        };

        MHO_FastFourierTransformWorkspace(const MHO_FastFourierTransformWorkspace& copy)
        {
            fN = 0;
            fM = 0;
            fPermutation = nullptr;
            fTwiddle = nullptr;
            fConjugateTwiddle = nullptr;
            fScale = nullptr;
            fCirculant = nullptr;
            fWorkspace = nullptr;
            Resize(copy.fN);
        };

        virtual ~MHO_FastFourierTransformWorkspace() { Deallocate(); };

        /**
         * @brief Resizes the internal data structure to a new size and fills it.
         * 
         * @param n New size for the data structure.
         */
        void Resize(unsigned int n)
        {
            if(fN != n)
            {
                Deallocate();
                Allocate(n);
                Fill();
            }
        }

        /**
         * @brief Checks if the current workspace is for radix-2 FFT
         * 
         * @return True if radix is 2, false otherwise (Bluestein).
         */
        bool IsRadix2() { return (fM == 0); }

        /**
         * @brief Getter for n
         * 
         * @return Current value of unsigned integer n
         */
        unsigned int GetN() { return fN; }

        /**
         * @brief Getter for m
         * 
         * @return unsigned int value of fM
         */
        unsigned int GetM() { return fM; }

        /**
         * @brief Getter for permutation
         * 
         * @return Pointer to unsigned integer containing the permutation value.
         */
        const unsigned int* GetPermutation() const { return fPermutation; }

        /**
         * @brief Getter for twiddle
         * 
         * @return Current value of twiddle as XFloatType pointer
         */
        const std::complex< XFloatType >* GetTwiddle() const { return fTwiddle; }

        /**
         * @brief Getter for conj twiddle
         * 
         * @return Conjugate twiddle value of type XFloatType*
         */
        const std::complex< XFloatType >* GetConjTwiddle() const { return fConjugateTwiddle; }

        /**
         * @brief Getter for scale
         * 
         * @return Pointer to XFloatType representing the current scale.
         */
        const std::complex< XFloatType >* GetScale() const { return fScale; } //unused for radix-2 (nullptr)

        /**
         * @brief Getter for circulant
         * 
         * @return Current circulant value of type XFloatType
         */
        const std::complex< XFloatType >* GetCirculant() const { return fCirculant; } //unused for radix-2 (nullptr)

        /**
         * @brief Getter for workspace
         * 
         * @return Pointer to XFloatType workspace
         */
        std::complex< XFloatType >* GetWorkspace() const { return fWorkspace; } //unused for radix-2 (nullptr)

        //data is publicly accessible (for now -- eventually re-work FFT classes to access data via getters)
    public:
        unsigned int fN;
        unsigned int fM; //unused for radix-2 (set to zero)
        unsigned int* fPermutation;
        std::complex< XFloatType >* fTwiddle;
        std::complex< XFloatType >* fConjugateTwiddle;
        std::complex< XFloatType >* fScale;     //unused for radix-2 (nullptr)
        std::complex< XFloatType >* fCirculant; //unused for radix-2 (nullptr)
        std::complex< XFloatType >* fWorkspace; //unused for radix-2 (nullptr)

    private:
        void Deallocate()
        {
            delete[] fPermutation;
            fPermutation = nullptr;
            delete[] fTwiddle;
            fTwiddle = nullptr;
            delete[] fConjugateTwiddle;
            fConjugateTwiddle = nullptr;
            delete[] fScale;
            fScale = nullptr;
            delete[] fCirculant;
            fCirculant = nullptr;
            delete[] fWorkspace;
            fWorkspace = nullptr;
            fN = 0;
            fM = 0;
        }

        void Fill()
        {
            if(fN == 0)
            {
                return;
            }
            if(fM == 0)
            {
                //radix-2 algorithm, only need a handful of items
                MHO_BitReversalPermutation::ComputeBitReversedIndicesBaseTwo(fN, fPermutation);
                MHO_FastFourierTransformUtilities< XFloatType >::ComputeTwiddleFactors(fN, fTwiddle);
                MHO_FastFourierTransformUtilities< XFloatType >::ComputeConjugateTwiddleFactors(fN, fConjugateTwiddle);
            }
            else
            {
                //use Bluestein algorithm
                MHO_BitReversalPermutation::ComputeBitReversedIndicesBaseTwo(fM, fPermutation);
                MHO_FastFourierTransformUtilities< XFloatType >::ComputeTwiddleFactors(fM, fTwiddle);
                MHO_FastFourierTransformUtilities< XFloatType >::ComputeConjugateTwiddleFactors(fM, fConjugateTwiddle);
                MHO_FastFourierTransformUtilities< XFloatType >::ComputeBluesteinScaleFactors(fN, fScale);
                MHO_FastFourierTransformUtilities< XFloatType >::ComputeBluesteinCirculantVector(fN, fM, fTwiddle, fScale,
                                                                                                 fCirculant);
            }
        }

        void Allocate(unsigned int n)
        {
            fN = n;
            if(fN == 0)
            {
                return;
            }
            bool is_radix2 = MHO_BitReversalPermutation::IsPowerOfTwo(fN);
            if(!is_radix2)
            {
                fM = MHO_FastFourierTransformUtilities< XFloatType >::ComputeBluesteinArraySize(fN);
                //can't perform an in-place transform, need Bluestein algo workspace
                fPermutation = new unsigned int[fM];
                fTwiddle = new std::complex< XFloatType >[fM];
                fConjugateTwiddle = new std::complex< XFloatType >[fM];
                fScale = new std::complex< XFloatType >[fN];
                fCirculant = new std::complex< XFloatType >[fM];
                fWorkspace = new std::complex< XFloatType >[fM];
            }
            else
            {
                //radix-2 algorithm
                fM = 0;
                fPermutation = new unsigned int[fN];
                fTwiddle = new std::complex< XFloatType >[fN];
                fConjugateTwiddle = new std::complex< XFloatType >[fN];
            }
        }
};

} // namespace hops

#endif /*! MHO_FastFourierTransformWorkspace_HH__ */
