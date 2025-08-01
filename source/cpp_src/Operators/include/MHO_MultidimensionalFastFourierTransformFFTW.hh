#ifndef MHO_MultidimensionalFastFourierTransformFFTW_HH__
#define MHO_MultidimensionalFastFourierTransformFFTW_HH__

#include <cstring>

#include "MHO_FFTWTypes.hh"

#include "MHO_Message.hh"
#include "MHO_Meta.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryOperator.hh"

#include "MHO_MultidimensionalFastFourierTransformInterface.hh"
#include "MHO_TableContainer.hh"

#define HOPS_FFTW_PLAN_ALGO FFTW_ESTIMATE

//could use FFTW_MEASURE instead

namespace hops
{

/*!
 *@file MHO_MultidimensionalFastFourierTransformFFTW.hh
 *@class MHO_MultidimensionalFastFourierTransformFFTW
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Oct 23 12:02:01 2020 -0400
 *@brief Operator for multidimensional FFTs (FFTW3 implementation)
 */

/**
 * @brief Class MHO_MultidimensionalFastFourierTransformFFTW
 */
template< typename XArgType >
class MHO_MultidimensionalFastFourierTransformFFTW: public MHO_UnaryOperator< XArgType >,
                                                    public MHO_MultidimensionalFastFourierTransformInterface< XArgType >
{
    public:
        static_assert(is_complex< typename XArgType::value_type >::value,
                      "Array element type must be a complex floating point type.");

        using complex_value_type = typename XArgType::value_type;
        using floating_point_value_type = typename XArgType::value_type::value_type;

        MHO_MultidimensionalFastFourierTransformFFTW(): MHO_MultidimensionalFastFourierTransformInterface< XArgType >()
        {
            fTotalArraySize = 0;
            fInPtr = NULL;
            fOutPtr = NULL;
            fCurrentPlan = NULL;
            fInPlacePtr = NULL;
            fPlanForward = NULL;
            fPlanBackward = NULL;
            fPlanForwardInPlace = NULL;
            fPlanBackwardInPlace = NULL;
            AllocateWorkspace(16); //pre-allocate a bit of space, so we can test for memory alignment
            fHaveAlignmentFuncs = false;

            //determine what version of FFTW3 we have
            //(fftw versions < 3.3.4 do not have functions to determine memory alignment)
            int fftw3_major = MHO_FFTWTypeInfo::get_fftw_version_major();
            int fftw3_minor = MHO_FFTWTypeInfo::get_fftw_version_minor();
            int fftw3_patch = MHO_FFTWTypeInfo::get_fftw_version_patch();
            if(fftw3_minor > 3)
            {
                fHaveAlignmentFuncs = true;
            }
            else if(fftw3_minor == 3)
            {
                if(fftw3_patch >= 4)
                {
                    fHaveAlignmentFuncs = true;
                }
            }
        };

        virtual ~MHO_MultidimensionalFastFourierTransformFFTW()
        {
            DeallocateWorkspace();
            DestructPlan();
        };

    protected:
        /**
         * @brief Function InitializeInPlace
         *
         * @param in (XArgType*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(XArgType* in) override
        {
            if(in != nullptr)
            {
                this->fIsValid = true;
            }
            else
            {
                this->fIsValid = false;
            }

            if(this->fIsValid)
            {
                //check if the current transform sizes are the same as input
                bool need_to_resize = false;
                for(std::size_t i = 0; i < XArgType::rank::value; i++)
                {
                    if(this->fDimensionSize[i] != in->GetDimension(i))
                    {
                        need_to_resize = true;
                        break;
                    }
                }
                if(need_to_resize || !this->fInitialized)
                {
                    in->GetDimensions(this->fDimensionSize);
                    fTotalArraySize = MHO_NDArrayMath::TotalArraySize< XArgType::rank::value >(this->fDimensionSize);
                    bool aligned = HaveSameAlignment(in->GetData(), fInPtr);
                    if(!aligned)
                    {
                        DeallocateWorkspace();
                        AllocateWorkspace(fTotalArraySize);
                    }
                    DestructPlan();
                    this->fInitialized = ConstructPlan();
#ifdef HOPS_ENABLE_DEBUG_MSG
                    msg_debug("operators", "initialized an in-place FFTW3 FFT plan." << eom);
                    for(std::size_t i = 0; i < XArgType::rank::value; i++)
                    {
                        msg_debug("operators", "fft plan dimension: " << i << " has size: " << this->fDimensionSize[i]
                                                                      << ", enabled? " << this->fAxesToXForm[i] << "." << eom);
                    }
#endif
                }
                else
                {
                    this->fInitialized = true;
                }
            }
            return (this->fInitialized && this->fIsValid);
        }

        /**
         * @brief Function InitializeOutOfPlace
         *
         * @param in (const XArgType*)
         * @param out (XArgType*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const XArgType* in, XArgType* out) override
        {
            if(in != nullptr && out != nullptr)
            {
                this->fIsValid = true;
            }
            else
            {
                this->fIsValid = false;
            }

            if(this->fIsValid)
            {
                //check if the arrays have the same dimensions
                if(!HaveSameDimensions(in, out))
                {
                    //resize the output array to match input
                    out->Resize(in->GetDimensions());
                }
                //check if the current transform sizes are the same as input
                bool need_to_resize = false;
                for(std::size_t i = 0; i < XArgType::rank::value; i++)
                {
                    if(this->fDimensionSize[i] != in->GetDimension(i))
                    {
                        need_to_resize = true;
                        break;
                    }
                }
                if(need_to_resize || !this->fInitialized)
                {
                    in->GetDimensions(this->fDimensionSize);
                    fTotalArraySize = MHO_NDArrayMath::TotalArraySize< XArgType::rank::value >(this->fDimensionSize);
                    DeallocateWorkspace();
                    AllocateWorkspace(fTotalArraySize);
                    DestructPlan();
                    this->fInitialized = ConstructPlan();
#ifdef HOPS_ENABLE_DEBUG_MSG
                    msg_debug("operators", "initialized an out-of-place FFTW3 FFT plan." << eom);
                    for(std::size_t i = 0; i < XArgType::rank::value; i++)
                    {
                        msg_debug("operators", "fft plan dimension: " << i << " has size: " << this->fDimensionSize[i]
                                                                      << ", enabled? " << this->fAxesToXForm[i] << "." << eom);
                    }
#endif
                }
                else
                {
                    this->fInitialized = true;
                }
            }
            return (this->fInitialized && this->fIsValid);
        }

        /**
         * @brief Function ExecuteInPlace
         *
         * @param in (XArgType*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(XArgType* in) override
        {
            if(!this->fIsValid || !this->fInitialized)
            {
                //error
                if(!this->fIsValid)
                {
                    msg_error("operators",
                              "FFT input/output array dimensions/pointers are not valid. Aborting transform." << eom);
                }
                if(!this->fInitialized)
                {
                    msg_error("operators", "FFT intialization failed. Aborting transform." << eom);
                }
                return false;
            }

            //check memory alignment to determine if we can avoid copying the data around
            if(HaveSameAlignment(in->GetData(), fInPtr))
            {
                //msg_info("operators", "FFT input/workspace data have the same memory alignment, no copy needed" << eom);
                if(this->fForward)
                {
                    fCurrentPlan = &fPlanForwardInPlace;
                }
                else
                {
                    fCurrentPlan = &fPlanBackwardInPlace;
                }
                MHO_FFTWTypes< floating_point_value_type >::execute_dft_func(
                    *fCurrentPlan,
                    reinterpret_cast< typename MHO_FFTWTypes< floating_point_value_type >::fftw_complex_type_ptr >(
                        in->GetData()),
                    reinterpret_cast< typename MHO_FFTWTypes< floating_point_value_type >::fftw_complex_type_ptr >(
                        in->GetData()));
            }
            else
            {
                if(this->fForward)
                {
                    fCurrentPlan = &fPlanForward;
                }
                else
                {
                    fCurrentPlan = &fPlanBackward;
                }
                //msg_info("operators", "FFT input/workspace do not have the same memory alignment, copy needed" << eom);
                //alignment doesn't match so we need to use memcpy
                std::memcpy(fInPtr, in->GetData(),
                            fTotalArraySize * sizeof(typename MHO_FFTWTypes< floating_point_value_type >::fftw_complex_type));
                MHO_FFTWTypes< floating_point_value_type >::execute_func(*fCurrentPlan);
                std::memcpy(in->GetData(), fOutPtr,
                            fTotalArraySize * sizeof(typename MHO_FFTWTypes< floating_point_value_type >::fftw_complex_type));
            }

            for(size_t d = 0; d < XArgType::rank::value; d++)
            {
                if(this->fAxesToXForm[d])
                {
                    this->IfTableTransformAxis(in, d);
                }
            };

            return true;
        }

        /**
         * @brief Copies input data to output and executes in-place if input/output are different.
         *
         * @param in Input data of type XArgType*
         * @param out Output data of type XArgType*
         * @return Result of ExecuteInPlace function call
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const XArgType* in, XArgType* out) override
        {
            //if input and output point to the same array, don't bother copying data over
            if(in && out && in != out)
            {
                out->Copy(*in);
            }
            return ExecuteInPlace(out);
        }

    private:
        /**
         * @brief Allocate workspace memory for FFTW operations.
         *
         * @param total_array_size Size of arrays to allocate.
         * @note This is a virtual function.
         */
        virtual void AllocateWorkspace(std::size_t total_array_size)
        {
            fInPtr = MHO_FFTWTypes< floating_point_value_type >::alloc_func(total_array_size);
            fOutPtr = MHO_FFTWTypes< floating_point_value_type >::alloc_func(total_array_size);
            fInPlacePtr = MHO_FFTWTypes< floating_point_value_type >::alloc_func(total_array_size);
        }

        /**
         * @brief Frees allocated workspace pointers using MHO_FFTWTypes free_func.
         * @note This is a virtual function.
         */
        virtual void DeallocateWorkspace()
        {
            MHO_FFTWTypes< floating_point_value_type >::free_func(fInPtr);
            MHO_FFTWTypes< floating_point_value_type >::free_func(fOutPtr);
            MHO_FFTWTypes< floating_point_value_type >::free_func(fInPlacePtr);
        }

        /**
         * @brief Destroys all initialized FFTW plans and sets them to NULL.
         */
        void DestructPlan()
        {
            if(this->fInitialized)
            {
                MHO_FFTWTypes< floating_point_value_type >::destroy_plan_func(fPlanForward);
                fPlanForward = NULL;
                MHO_FFTWTypes< floating_point_value_type >::destroy_plan_func(fPlanBackward);
                fPlanBackward = NULL;
                MHO_FFTWTypes< floating_point_value_type >::destroy_plan_func(fPlanForwardInPlace);
                fPlanForwardInPlace = NULL;
                MHO_FFTWTypes< floating_point_value_type >::destroy_plan_func(fPlanBackwardInPlace);
                fPlanBackwardInPlace = NULL;
            }
        }

        /**
         * @brief Function ConstructPlan
         *
         * @return Return value (bool)
         */
        bool ConstructPlan()
        {
            if(fInPtr == NULL || fOutPtr == NULL || fInPlacePtr == NULL)
            {
                return false;
            }

            //then compute the rank of the transform (the number of axes selected)
            int rank = 0;
            for(size_t d = 0; d < XArgType::rank::value; d++)
            {
                if(this->fAxesToXForm[d])
                {
                    rank++;
                }
            }

            //then compute the howmany_rank of the transform
            //number of dimensions need to describe location of the starting point
            int howmany_rank = XArgType::rank::value - rank;

            //now figure out the dimensions of the transforms
            //note: unless we are transforming every dimension of the input,
            //we may not fill up this array completely (hence the count variable)
            int count = 0;
            for(size_t d = 0; d < XArgType::rank::value; d++)
            {
                if(this->fAxesToXForm[d])
                {
                    //figure out the dims parameter (length of the FFT and stride between elements)
                    fDims[count].n = this->fDimensionSize[d];
                    fDims[count].is =
                        MHO_NDArrayMath::StrideFromRowMajorIndex< XArgType::rank::value >(d, this->fDimensionSize);
                    fDims[count].os =
                        MHO_NDArrayMath::StrideFromRowMajorIndex< XArgType::rank::value >(d, this->fDimensionSize);
                    count++;
                }
            }

            count = 0;
            for(size_t d = 0; d < XArgType::rank::value; d++)
            {
                if(!this->fAxesToXForm[d])
                {
                    //figure out the dims parameter (length of the FFT and stride between elements)
                    fHowManyDims[count].n = this->fDimensionSize[d];
                    fHowManyDims[count].is =
                        MHO_NDArrayMath::StrideFromRowMajorIndex< XArgType::rank::value >(d, this->fDimensionSize);
                    fHowManyDims[count].os =
                        MHO_NDArrayMath::StrideFromRowMajorIndex< XArgType::rank::value >(d, this->fDimensionSize);
                    count++;
                }
            }

            fPlanForward = MHO_FFTWTypes< floating_point_value_type >::plan_guru_func(
                rank, fDims, howmany_rank, fHowManyDims, fInPtr, fOutPtr, FFTW_FORWARD, HOPS_FFTW_PLAN_ALGO);

            fPlanBackward = MHO_FFTWTypes< floating_point_value_type >::plan_guru_func(
                rank, fDims, howmany_rank, fHowManyDims, fInPtr, fOutPtr, FFTW_BACKWARD, HOPS_FFTW_PLAN_ALGO);

            fPlanForwardInPlace = MHO_FFTWTypes< floating_point_value_type >::plan_guru_func(
                rank, fDims, howmany_rank, fHowManyDims, fInPlacePtr, fInPlacePtr, FFTW_FORWARD, HOPS_FFTW_PLAN_ALGO);

            fPlanBackwardInPlace = MHO_FFTWTypes< floating_point_value_type >::plan_guru_func(
                rank, fDims, howmany_rank, fHowManyDims, fInPlacePtr, fInPlacePtr, FFTW_BACKWARD, HOPS_FFTW_PLAN_ALGO);

            if(fPlanForward != NULL && fPlanBackward != NULL && fPlanBackwardInPlace != NULL && fPlanForwardInPlace != NULL)
            {
                return true;
            }
            else
            {
                msg_warn("operators", "could not construct FFTW transform plan." << eom);
                return false;
            }
        }

        /**
         * @brief Checks if two pointers have the same memory alignment.
         *
         * @param ptr1 Pointer to first data block
         * @param ptr2 Pointer to second data block
         * @return Boolean indicating whether pointers have the same alignment
         */
        template< typename XPtrType1, typename XPtrType2 > bool HaveSameAlignment(XPtrType1 ptr1, XPtrType2 ptr2)
        {
            if(!fHaveAlignmentFuncs)
            {
                return false;
            }
            return (MHO_FFTWTypes< floating_point_value_type >::alignment_of_func(
                        reinterpret_cast< floating_point_value_type* >(ptr1)) ==
                    MHO_FFTWTypes< floating_point_value_type >::alignment_of_func(
                        reinterpret_cast< floating_point_value_type* >(ptr2)));
        }

        //data
        size_t fTotalArraySize;
        typename MHO_FFTWTypes< floating_point_value_type >::plan_type* fCurrentPlan;
        typename MHO_FFTWTypes< floating_point_value_type >::iodim_type fDims[XArgType::rank::value];
        typename MHO_FFTWTypes< floating_point_value_type >::iodim_type fHowManyDims[XArgType::rank::value];
        typename MHO_FFTWTypes< floating_point_value_type >::plan_type fPlanForward;
        typename MHO_FFTWTypes< floating_point_value_type >::plan_type fPlanBackward;
        typename MHO_FFTWTypes< floating_point_value_type >::plan_type fPlanForwardInPlace;
        typename MHO_FFTWTypes< floating_point_value_type >::plan_type fPlanBackwardInPlace;
        typename MHO_FFTWTypes< floating_point_value_type >::fftw_complex_type_ptr fInPtr;
        typename MHO_FFTWTypes< floating_point_value_type >::fftw_complex_type_ptr fOutPtr;
        typename MHO_FFTWTypes< floating_point_value_type >::fftw_complex_type_ptr fInPlacePtr;

        //detect if we have mem alignment functions
        bool fHaveAlignmentFuncs;
};

} // namespace hops

#endif /*! MHO_MultidimensionalFastFourierTransformFFTW_H__ */
