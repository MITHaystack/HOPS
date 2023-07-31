#ifndef MHO_MultidimensionalFastFourierTransformFFTW_HH__
#define MHO_MultidimensionalFastFourierTransformFFTW_HH__

#include <cstring>
#include <fftw3.h>

#include "MHO_Meta.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryOperator.hh"
#include "MHO_FFTWTypes.hh"

#include "MHO_TableContainer.hh"

#define HOPS_FFTW_PLAN_ALGO FFTW_ESTIMATE
//could use FFTW_MEASURE instead


namespace hops
{


template< typename XArgType >
class MHO_MultidimensionalFastFourierTransformFFTW:
    public MHO_UnaryOperator< XArgType >
{
    public:

        static_assert( is_complex< typename XArgType::value_type >::value, "Array element type must be a complex floating point type." );

        using complex_value_type = typename XArgType::value_type;
        using floating_point_value_type = typename XArgType::value_type::value_type;

        MHO_MultidimensionalFastFourierTransformFFTW()
        {
            fTotalArraySize = 0;
            fInPtr = NULL;
            fOutPtr = NULL;
            fInPlacePtr = NULL;
            for(size_t i=0; i<XArgType::rank::value; i++)
            {
                fDimensionSize[i] = 0;
                fAxesToXForm[i] = true;
            }

            fIsValid = false;
            fInitialized = false;
            fForward = true;

            fPlanForward = NULL;
            fPlanBackward = NULL;
            fPlanForwardInPlace = NULL;
            fPlanBackwardInPlace = NULL;
        };

        virtual ~MHO_MultidimensionalFastFourierTransformFFTW()
        {
            DealocateWorkspace();
            DestructPlan();
        };

        virtual void SetForward(){fForward = true;}
        virtual void SetBackward(){fForward = false;};

        //sometimes we may want to select/deselect particular dimensions of the x-form
        //default is to transform along every dimension, but that may not always be needed
        void SelectAllAxes(){for(std::size_t i=0; i<XArgType::rank::value; i++){fAxesToXForm[i] = true;} fInitialized = false;}
        void DeselectAllAxes(){for(std::size_t i=0; i<XArgType::rank::value; i++){fAxesToXForm[i] = false;} fInitialized = false;}
        void SelectAxis(std::size_t axis_index)
        {
            fInitialized = false;
            if(axis_index < XArgType::rank::value){fAxesToXForm[axis_index] = true;}
            else
            {
                msg_error("operators", "Cannot transform axis with index: " <<
                          axis_index << "for array with rank: " << XArgType::rank::value << eom);
            }
        }

    protected:

        virtual bool InitializeInPlace(XArgType* in) override
        {
            if( in != nullptr ){fIsValid = true;}
            else{fIsValid = false;}

            if(fIsValid)
            {
                //check if the current transform sizes are the same as input
                bool need_to_resize = false;
                for(std::size_t i=0; i<XArgType::rank::value; i++)
                {
                    if(fDimensionSize[i] != in->GetDimension(i)){need_to_resize = true; break;}
                }
                if(need_to_resize || !fInitialized ) 
                {
                    in->GetDimensions(fDimensionSize);
                    fTotalArraySize = MHO_NDArrayMath::TotalArraySize<XArgType::rank::value>(fDimensionSize);
                    DealocateWorkspace();
                    AllocateWorkspace();
                    DestructPlan();
                    fInitialized = ConstructPlan();
                }
                else
                {
                    fInitialized = true;
                }
            }
            return (fInitialized && fIsValid);
        }


        virtual bool InitializeOutOfPlace(const XArgType* in, XArgType* out) override
        {
            if( in != nullptr && out != nullptr ){fIsValid = true;}
            else{fIsValid = false;}

            if(fIsValid)
            {
                //check if the arrays have the same dimensions
                if( !HaveSameDimensions(in, out) )
                {
                    //resize the output array to match input
                    out->Resize( in->GetDimensions() );
                }
                //check if the current transform sizes are the same as input
                bool need_to_resize = false;
                for(std::size_t i=0; i<XArgType::rank::value; i++)
                {
                    if(fDimensionSize[i] != in->GetDimension(i)){need_to_resize = true; break;}
                }
                if(need_to_resize || !fInitialized )
                {
                    in->GetDimensions(fDimensionSize);
                    fTotalArraySize = MHO_NDArrayMath::TotalArraySize<XArgType::rank::value>(fDimensionSize);
                    DealocateWorkspace();
                    AllocateWorkspace();
                    DestructPlan();
                    fInitialized = ConstructPlan();
                }
                else
                {
                    fInitialized = true;
                }
            }
            return (fInitialized && fIsValid);
        }



        virtual bool ExecuteInPlace(XArgType* in) override
        {
            if(fIsValid && fInitialized)
            {
                //check memory alignment to determine if we can avoid copying the data around
                if( HaveSameAlignment(in->GetData(), fInPtr) )
                {
                    //we have to execute an in-place transform
                    if(fForward)
                    {
                        MHO_FFTWTypes<floating_point_value_type>::execute_dft_func(fPlanForwardInPlace,
                        reinterpret_cast<typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr>(in->GetData() ),
                        reinterpret_cast<typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr>(in->GetData() ) );
                    }
                    else
                    {
                        MHO_FFTWTypes<floating_point_value_type>::execute_dft_func(fPlanBackwardInPlace,
                        reinterpret_cast<typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr>(in->GetData() ),
                        reinterpret_cast<typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr>(in->GetData() ) );
                    }
                }
                else
                {
                    //alignment doesn't match so we need to use memcpy
                    std::memcpy( fInPtr, in->GetData() , fTotalArraySize*sizeof(typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type) );
                    if(fForward)
                    {
                        MHO_FFTWTypes<floating_point_value_type>::execute_func(fPlanForward);
                    }
                    else
                    {
                        MHO_FFTWTypes<floating_point_value_type>::execute_func(fPlanBackward);
                    }
                    std::memcpy(in->GetData(), fOutPtr, fTotalArraySize*sizeof(typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type) );
                }

                for(size_t d=0; d<XArgType::rank::value; d++)
                {
                    if(fAxesToXForm[d])
                    {
                        IfTableTransformAxis(in,d);
                    }
                };

                return true;
            }
            else
            {
                //error
                if(!fIsValid){msg_error("operators", "FFT input/output array dimensions/pointers are not valid. Aborting transform." << eom);}
                if(!fInitialized){msg_error("operators", "FFT intialization failed. Aborting transform." << eom);}
                return false;
            }
        }


        virtual bool ExecuteOutOfPlace(const XArgType* in, XArgType* out) override
        {
            if(fIsValid && fInitialized)
            {
                auto in_ptr = const_cast< complex_value_type* >( in->GetData() );// we promised not to modify it, so be careful
                //check memory alignment to determine if we can avoid copying the data around
                if( HaveSameAlignment( in_ptr, fInPtr) &&
                    HaveSameAlignment(out->GetData(), fOutPtr) )
                {
                    //transform is out-of-place
                    if(fForward)
                    {
                        MHO_FFTWTypes<floating_point_value_type>::execute_dft_func(fPlanForward,
                        reinterpret_cast<typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr>( in_ptr ),
                        reinterpret_cast<typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr>(out->GetData() ) );
                    }
                    else
                    {
                        MHO_FFTWTypes<floating_point_value_type>::execute_dft_func(fPlanBackward,
                        reinterpret_cast<typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr>( in_ptr ),
                        reinterpret_cast<typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr>(out->GetData() ) );
                    }
                }
                else
                {
                    //alignment doesn't match so we need to use memcpy
                    std::memcpy( fInPtr, in->GetData() , fTotalArraySize*sizeof(typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type) );
                    if(fForward)
                    {
                        MHO_FFTWTypes<floating_point_value_type>::execute_func(fPlanForward);
                    }
                    else
                    {
                        MHO_FFTWTypes<floating_point_value_type>::execute_func(fPlanBackward);
                    }
                    std::memcpy(out->GetData(), fOutPtr, fTotalArraySize*sizeof(typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type) );
                }

                
                for(size_t d=0; d<XArgType::rank::value; d++)
                {
                    if(fAxesToXForm[d])
                    {
                        IfTableTransformAxis(out,d);
                    }
                };


                return true;
            }
            else
            {
                //error
                if(!fIsValid){msg_error("operators", "FFT input/output array dimensions/pointers are not valid. Aborting transform." << eom);}
                if(!fInitialized){msg_error("operators", "FFT intialization failed. Aborting transform." << eom);}
                return false;
            }
        }



    private:

        virtual void AllocateWorkspace()
        {
            fInPtr = MHO_FFTWTypes<floating_point_value_type>::alloc_func(fTotalArraySize);
            fOutPtr = MHO_FFTWTypes<floating_point_value_type>::alloc_func(fTotalArraySize);
            fInPlacePtr = MHO_FFTWTypes<floating_point_value_type>::alloc_func(fTotalArraySize);
        }

        virtual void DealocateWorkspace()
        {
            MHO_FFTWTypes<floating_point_value_type>::free_func(fInPtr);
            MHO_FFTWTypes<floating_point_value_type>::free_func(fOutPtr);
            MHO_FFTWTypes<floating_point_value_type>::free_func(fInPlacePtr);
        }


        void DestructPlan()
        {
            if(fInitialized)
            {
                MHO_FFTWTypes<floating_point_value_type>::destroy_plan_func(fPlanForward); fPlanForward = NULL;
                MHO_FFTWTypes<floating_point_value_type>::destroy_plan_func(fPlanBackward); fPlanBackward = NULL;
                MHO_FFTWTypes<floating_point_value_type>::destroy_plan_func(fPlanForwardInPlace); fPlanForwardInPlace = NULL;
                MHO_FFTWTypes<floating_point_value_type>::destroy_plan_func(fPlanBackwardInPlace); fPlanBackwardInPlace = NULL;
            }
        }

        bool ConstructPlan()
        {
            if(fInPtr == NULL || fOutPtr == NULL || fInPlacePtr == NULL){return false;}

            //then compute the rank of the transform (the number of axes selected)
            int rank = 0;
            for(size_t d = 0; d < XArgType::rank::value; d++){if(fAxesToXForm[d]){rank++;}}

            //then compute the howmany_rank of the transform
            //number of dimensions need to describe location of the starting point
            int howmany_rank = XArgType::rank::value - rank;

            //now figure out the dimensions of the transforms
            //note: unless we are transforming every dimension of the input,
            //we may not fill up this array completely (hence the count variable)
            int count = 0; 
            for(size_t d = 0; d < XArgType::rank::value; d++)
            {
                if(fAxesToXForm[d])
                {
                    //figure out the dims parameter (length of the FFT and stride between elements)
                    fDims[count].n = fDimensionSize[d];
                    fDims[count].is = MHO_NDArrayMath::StrideFromRowMajorIndex<XArgType::rank::value>(d,fDimensionSize);
                    fDims[count].os = MHO_NDArrayMath::StrideFromRowMajorIndex<XArgType::rank::value>(d,fDimensionSize);
                    count++;
                }
            }

            count=0;
            for(size_t d = 0; d < XArgType::rank::value; d++)
            {
                if(!fAxesToXForm[d])
                {
                    //figure out the dims parameter (length of the FFT and stride between elements)
                    fHowManyDims[count].n = fDimensionSize[d];
                    fHowManyDims[count].is = MHO_NDArrayMath::StrideFromRowMajorIndex<XArgType::rank::value>(d,fDimensionSize);
                    fHowManyDims[count].os = MHO_NDArrayMath::StrideFromRowMajorIndex<XArgType::rank::value>(d,fDimensionSize);
                    count++;
                }

            }


            fPlanForward = MHO_FFTWTypes<floating_point_value_type>::plan_guru_func(rank, fDims, howmany_rank, fHowManyDims,
                                       fInPtr, fOutPtr, FFTW_FORWARD, HOPS_FFTW_PLAN_ALGO);

            fPlanBackward = MHO_FFTWTypes<floating_point_value_type>::plan_guru_func(rank, fDims, howmany_rank, fHowManyDims,
                                       fInPtr, fOutPtr, FFTW_BACKWARD, HOPS_FFTW_PLAN_ALGO);

            fPlanForwardInPlace = MHO_FFTWTypes<floating_point_value_type>::plan_guru_func(rank, fDims, howmany_rank, fHowManyDims,
                                       fInPlacePtr, fInPlacePtr, FFTW_FORWARD, HOPS_FFTW_PLAN_ALGO);

            fPlanBackwardInPlace = MHO_FFTWTypes<floating_point_value_type>::plan_guru_func(rank, fDims, howmany_rank, fHowManyDims,
                                       fInPlacePtr, fInPlacePtr, FFTW_BACKWARD, HOPS_FFTW_PLAN_ALGO);

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

        template< typename XPtrType1, typename XPtrType2>
        bool HaveSameAlignment(XPtrType1 ptr1, XPtrType2 ptr2)
        {
            return ( MHO_FFTWTypes<floating_point_value_type>::alignment_of_func( reinterpret_cast<floating_point_value_type*>(ptr1) ) ==
                     MHO_FFTWTypes<floating_point_value_type>::alignment_of_func( reinterpret_cast<floating_point_value_type*>(ptr2) ) );
        }


        //axis handling for table containers 


        //default...does nothing
        template< typename XCheckType = XArgType >
        typename std::enable_if< !std::is_base_of<MHO_TableContainerBase, XCheckType>::value, void >::type
        IfTableTransformAxis(XArgType* /*in*/, std::size_t /*axis_index*/){};

        //use SFINAE to generate specialization for MHO_TableContainer types
        template< typename XCheckType = XArgType >
        typename std::enable_if< std::is_base_of<MHO_TableContainerBase, XCheckType>::value, void >::type
        IfTableTransformAxis(XArgType* in, std::size_t axis_index)
        {
            if(fAxesToXForm[axis_index]) //only xform axis if this dimension was transformed
            {
                TransformAxis axis_xformer;
                apply_at< typename XArgType::axis_pack_tuple_type, TransformAxis >( *in, axis_index, axis_xformer);
            }
        }


        class TransformAxis
        {
            public:
                TransformAxis(){};
                ~TransformAxis(){};

                //generic axis, do nothing
                template< typename XAxisType >
                void operator()(XAxisType& /*axis1*/){};

                //overload for doubles
                void operator()(MHO_Axis<double>& axis1)
                {
                    //this is under the expectation that all axis labels are equi-spaced
                    //this should be a safe assumption since we are doing DFT anyway
                    //one issue here is that we are not taking into account units (e.g. nanosec or MHz)
                    std::size_t N = axis1.GetSize();
                    double length = N;
                    if(N > 1)
                    {
                        double delta = axis1(1) - axis1(0);
                        double spacing = (1.0/delta)*(1.0/length);
                        double start = 0;//-1*length/2;
                        for(std::size_t i=0; i<N; i++)
                        {
                            double x = i;
                            if(i<N/2)
                            {
                                start = 0;
                                double value = (x+start)*spacing;
                                axis1(i) = value;
                            }
                            else
                            {
                                start = -1*length;
                                double value = (x+start)*spacing;
                                axis1(i) = value;
                            }
                        }
                    }
                }

                //overload for floats
                void operator()(MHO_Axis<float>& axis1)
                {
                    //this is under the expectation that all axis labels are equi-spaced
                    //this should be a safe assumption since we are doing DFT anyway
                    //one issue here is that we are not taking into account units (e.g. nanosec or MHz)
                    std::size_t N = axis1.GetSize();
                    float length = N;
                    if(N > 1)
                    {
                        float delta = axis1(1) - axis1(0);
                        float spacing = (1.0/delta)*(1.0/length);
                        float start = 0;//-1*length/2;
                        for(std::size_t i=0; i<N; i++)
                        {
                            float x = i;
                            if(i<N/2)
                            {
                                start = 0;
                                float value = (x+start)*spacing;
                                axis1(i) = value;
                            }
                            else
                            {
                                start = -1*length;
                                float value = (x+start)*spacing;
                                axis1(i) = value;
                            }
                        }
                    }
                }

        };



        //data
        bool fIsValid;
        bool fForward;
        bool fInitialized;

        size_t fTotalArraySize;
        size_t fDimensionSize[XArgType::rank::value];
        bool fAxesToXForm[XArgType::rank::value];

        typename MHO_FFTWTypes<floating_point_value_type>::iodim_type fDims[XArgType::rank::value];
        typename MHO_FFTWTypes<floating_point_value_type>::iodim_type fHowManyDims[XArgType::rank::value];
        typename MHO_FFTWTypes<floating_point_value_type>::plan_type fPlanForward;
        typename MHO_FFTWTypes<floating_point_value_type>::plan_type fPlanBackward;
        typename MHO_FFTWTypes<floating_point_value_type>::plan_type fPlanForwardInPlace;
        typename MHO_FFTWTypes<floating_point_value_type>::plan_type fPlanBackwardInPlace;
        typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr fInPtr;
        typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr fOutPtr;
        typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr fInPlacePtr;
};


}

#endif /* MHO_MultidimensionalFastFourierTransformFFTW_H__ */
