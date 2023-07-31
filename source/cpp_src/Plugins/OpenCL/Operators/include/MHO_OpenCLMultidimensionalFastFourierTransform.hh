#ifndef MHO_OpenCLMultidimensionalFastFourierTransform_HH__
#define MHO_OpenCLMultidimensionalFastFourierTransform_HH__

#include <cstring>

#include "MHO_Meta.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryInPlaceOperator.hh"

#include "MHO_FastFourierTransform.hh"
#include "MHO_FastFourierTransformUtilities.hh"

#include "MHO_OpenCLInterface.hh"
#include "MHO_OpenCLKernelBuilder.hh"



#include "MHO_TableContainer.hh"

namespace hops
{

template< typename XArgType >
class MHO_OpenCLMultidimensionalFastFourierTransform:
    public MHO_UnaryInPlaceOperator< XArgType >
{
    public:

        static_assert( is_complex< typename XArgType::value_type >::value, "Array element type must be a complex floating point type." );
        using complex_value_type = typename XArgType::value_type;
        using floating_point_value_type = typename XArgType::value_type::value_type;

        MHO_OpenCLMultidimensionalFastFourierTransform()
        {
            for(size_t i=0; i<XArgType::rank::value; i++)
            {
                fDimensionSize[i] = 0;
                fAxesToXForm[i] = true;
                fWorkspaceWrapper[i] = NULL;
                fTransformCalculator[i] = NULL;
            }

            fIsValid = false;
            fInitialized = false;
            fForward = true;
        };

        virtual ~MHO_OpenCLMultidimensionalFastFourierTransform()
        {
            DeallocateWorkspace();
        };

        virtual void SetForward(){fForward = true;}
        virtual void SetBackward(){fForward = false;};

        //sometimes we may want to select/deselect particular dimensions of the x-form
        //default is to transform along every dimension, but that may not always be needed
        void SelectAllAxes(){for(std::size_t i=0; i<XArgType::rank::value; i++){fAxesToXForm[i] = true;}}
        void DeselectAllAxes(){for(std::size_t i=0; i<XArgType::rank::value; i++){fAxesToXForm[i] = false;}}
        void SelectAxis(std::size_t axis_index)
        {
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
                if(need_to_resize)
                {
                    in->GetDimensions(fDimensionSize);
                    DeallocateWorkspace();
                    AllocateWorkspace();
                }
                fInitialized = true;
            }
            return (fInitialized && fIsValid);
        }

    


        virtual bool ExecuteInPlace(XArgType* in) override
        {
            if(fIsValid && fInitialized)
            {
                size_t total_size = 1;
                for(size_t i=0; i<XArgType::rank::value; i++)
                {
                    total_size *= fDimensionSize[i];
                    if(fForward)
                    {
                        fTransformCalculator[i]->SetForward();
                    }
                    else
                    {
                        fTransformCalculator[i]->SetBackward();
                    }
                }

                size_t index[XArgType::rank::value];
                size_t non_active_dimension_size[XArgType::rank::value-1];
                size_t non_active_dimension_value[XArgType::rank::value-1];
                size_t non_active_dimension_index[XArgType::rank::value-1];

                //select the dimension on which to perform the FFT
                for(size_t d = 0; d < XArgType::rank::value; d++)
                {
                    if(fAxesToXForm[d])
                    {
                        //now we loop over all dimensions not specified by d
                        //first compute the number of FFTs to perform
                        size_t n_fft = 1;
                        size_t count = 0;
                        for(size_t i = 0; i < XArgType::rank::value; i++)
                        {
                            if(i != d)
                            {
                                n_fft *= fDimensionSize[i];
                                non_active_dimension_index[count] = i;
                                non_active_dimension_size[count] = fDimensionSize[i];
                                count++;
                            }
                        }

                        //loop over the number of FFTs to perform
                        for(size_t n=0; n<n_fft; n++)
                        {
                            //invert place in list to obtain indices of block in array
                            MHO_NDArrayMath::RowMajorIndexFromOffset<XArgType::rank::value-1>(n, non_active_dimension_size, non_active_dimension_value);

                            //copy the value of the non-active dimensions in to index
                            for(size_t i=0; i<XArgType::rank::value-1; i++)
                            {
                                index[ non_active_dimension_index[i] ] = non_active_dimension_value[i];
                            }

                            size_t data_location;
                            //copy the row selected by the other dimensions
                            for(size_t i=0; i<fDimensionSize[d]; i++)
                            {
                                index[d] = i;
                                data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<XArgType::rank::value>(fDimensionSize, index);
                                (*(fWorkspaceWrapper[d]))[i] = (*(in))[data_location];
                            }

                            //compute the FFT of the row selected
                            fTransformCalculator[d]->Execute();

                            //copy the row selected back
                            for(size_t i=0; i<fDimensionSize[d]; i++)
                            {
                                index[d] = i;
                                data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<XArgType::rank::value>(fDimensionSize, index);
                                (*(in))[data_location] = (*(fWorkspaceWrapper[d]))[i];
                            }
                        }
                    }

                    //IfTableTransformAxis(in,d);
                }
                //successful
                return true;
            }
            else
            {
                //error
                msg_error("math", "FFT input/output array dimensions are not valid or intialization failed. Aborting transform." << eom);
                return false;
            }
        }


    private:






        void BuildBuffers()
        {
            std::cout<<"building buffers"<<std::endl;
            //buffer for the 'spatial' dimensions of the array
            fDimensionBufferCL = new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(),
                                                       CL_MEM_READ_ONLY,
                                                       XArgType::rank::value * sizeof(size_t));

            //buffer for the FFT twiddle factors
            fTwiddleBufferCL = new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(),
                                              CL_MEM_READ_ONLY,
                                              fMaxBufferSize * sizeof(CL_TYPE2));

            //buffer for the conjugate FFT twiddle factors
            fConjugateTwiddleBufferCL = new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(),
                                                       CL_MEM_READ_ONLY,
                                                       fMaxBufferSize * sizeof(CL_TYPE2));

            //buffer for the bluestein scale factors
            fScaleBufferCL = new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(),
                                            CL_MEM_READ_ONLY,
                                            fMaxBufferSize * sizeof(CL_TYPE2));

            //buffer for the bluestein circulant vector
            fCirculantBufferCL = new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(),
                                                CL_MEM_READ_ONLY,
                                                fMaxBufferSize * sizeof(CL_TYPE2));

            //buffer for the data to be transformed
            fDataBufferCL = new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(),
                                           CL_MEM_READ_WRITE,
                                           fTotalDataSize * sizeof(CL_TYPE2));

            //buffer for the permutation_array
            fPermuationArrayCL = new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(),
                                                CL_MEM_READ_ONLY,
                                                fMaxBufferSize * sizeof(size_t));

            //determine the largest global worksize
            fMaxNWorkItems = 0;
            for (size_t D = 0; D < XArgType::rank::value; D++) {
                //compute number of 1d fft's needed (n-global)
                size_t n_global = fDimensionSize[0];
                size_t n_local_1d_transforms = 1;
                for (size_t i = 0; i < XArgType::rank::value-1; i++) {
                    if (i != D) {
                        n_global *= fSpatialDim[i];
                        n_local_1d_transforms *= fSpatialDim[i];
                    };
                };

                //pad out n-global to be a multiple of the n-local
                size_t nDummy = fNLocal - (n_global % fNLocal);
                if (nDummy == fNLocal) {
                    nDummy = 0;
                };
                n_global += nDummy;

                if (fMaxNWorkItems < n_global) {
                    fMaxNWorkItems = n_global;
                };
            }
        }




        void AssignBuffers()
        {
            cl::CommandQueue& Q = MHO_OpenCLInterface::GetInstance()->GetQueue();

            //assign buffers and set kernel arguments
            unsigned int n_multdim_ffts = fDimensionSize[0];
            //small arguments that do not need to be enqueued/written
            fFFTKernel->setArg(0, n_multdim_ffts);  //number of multidimensional fft's to perform


            //assign buffers and set kernel arguments
            fFFTKernel->setArg(1, 0);  //(index of the selected dimension) updated at each stage

            //assign buffers and set kernel arguments
            fFFTKernel->setArg(2, 0);  //updated at each execution

            //array dimensionality written once
            fFFTKernel->setArg(3, *fDimensionBufferCL);
            Q.enqueueWriteBuffer(*fDimensionBufferCL, CL_TRUE, 0, (XArgType::rank::value-1) * sizeof(unsigned int), fSpatialDim);
    #ifdef ENFORCE_CL_FINISH
            MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
    #endif


            //following are updated at each stage when necessary
            //however in the special case where all spatial dimensions are the same
            //we can write them to the GPU now and need not re-send them during execution
            fFFTKernel->setArg(4, *fTwiddleBufferCL);
            fFFTKernel->setArg(5, *fConjugateTwiddleBufferCL);
            fFFTKernel->setArg(6, *fScaleBufferCL);
            fFFTKernel->setArg(7, *fCirculantBufferCL);
            fFFTKernel->setArg(8, *fPermuationArrayCL);


            if (fAllSpatialDimensionsAreEqual) {
                //write the constant buffers
                Q.enqueueWriteBuffer(*fTwiddleBufferCL, CL_TRUE, 0, fMaxBufferSize * sizeof(CL_TYPE2), &(fTwiddle[0][0]));
    #ifdef ENFORCE_CL_FINISH
                MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
    #endif
                Q.enqueueWriteBuffer(*fConjugateTwiddleBufferCL,
                                     CL_TRUE,
                                     0,
                                     fMaxBufferSize * sizeof(CL_TYPE2),
                                     &(fConjugateTwiddle[0][0]));
    #ifdef ENFORCE_CL_FINISH
                MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
    #endif
                Q.enqueueWriteBuffer(*fScaleBufferCL, CL_TRUE, 0, fMaxBufferSize * sizeof(CL_TYPE2), &(fScale[0][0]));
    #ifdef ENFORCE_CL_FINISH
                MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
    #endif
                Q.enqueueWriteBuffer(*fCirculantBufferCL,
                                     CL_TRUE,
                                     0,
                                     fMaxBufferSize * sizeof(CL_TYPE2),
                                     &(fCirculant[0][0]));
    #ifdef ENFORCE_CL_FINISH
                MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
    #endif
                Q.enqueueWriteBuffer(*fPermuationArrayCL,
                                     CL_TRUE,
                                     0,
                                     fMaxBufferSize * sizeof(unsigned int),
                                     &(fPermuationArray[0][0]));
    #ifdef ENFORCE_CL_FINISH
                MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
    #endif
            }

            //the data is updated once per execution
            fFFTKernel->setArg(9, *fDataBufferCL);
        }














        
        //data

        bool fIsValid;
        bool fForward;
        bool fInitialized;

        size_t fDimensionSize[XArgType::rank::value];
        bool fAxesToXForm[XArgType::rank::value];

};


}

#endif /* MHO_OpenCLMultidimensionalFastFourierTransform_H__ */
