#ifndef MHO_OpenCLFastFourierTransformRadix2Stage_HH__
#define MHO_OpenCLFastFourierTransformRadix2Stage_HH__

#include <cstring>

#include "MHO_Message.hh"
#include "MHO_Meta.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryInPlaceOperator.hh"

#include "MHO_FastFourierTransform.hh"
#include "MHO_FastFourierTransformUtilities.hh"
#include "MHO_FastFourierTransformWorkspace.hh"
#include "MHO_MultidimensionalFastFourierTransformInterface.hh"

#include "MHO_OpenCLInterface.hh"
#include "MHO_OpenCLKernelBuilder.hh"
#include "MHO_OpenCLNDArrayBuffer.hh"

#include "MHO_TableContainer.hh"

namespace hops
{

template< typename XArgType >
class MHO_OpenCLFastFourierTransformRadix2Stage: public MHO_UnaryInPlaceOperator< XArgType >,
                                                 public MHO_MultidimensionalFastFourierTransformInterface< XArgType >
{
    public:
        static_assert(is_complex< typename XArgType::value_type >::value,
                      "Array element type must be a complex floating point type.");
        using complex_value_type = typename XArgType::value_type;
        using floating_point_value_type = typename XArgType::value_type::value_type;

        MHO_OpenCLScalarMultiply()
            : fInitialized(false), fNLocal(0), fNGlobal(0), fKernel(nullptr),
              fWriteOut(true), //default is always to write host -> device
              fReadBack(true)  //default is always to read device -> host
        {
            BuildKernel();
        };

        virtual ~MHO_OpenCLScalarMultiply()
        {
            delete fKernel;
            DeallocateDeviceWorkspace();
        };

        //only one axis can be selected at a time
        virtual void SelectAxis(std::size_t axis_index) override
        {
            fInitialized = false;
            fActiveAxisIndex = axis_index;
            for(std::size_t i = 0; i < XArgType::rank::value; i++)
            {
                fAxesToXForm[i] = false;
            }
            if(axis_index < XArgType::rank::value)
            {
                fAxesToXForm[axis_index] = true;
            }
        }

        //sometimes there is no need to read the data back from device -> host
        //for example, if there is another kernel using the same data that
        //is running immediately afterwards, we can just leave the data there
        void SetReadTrue() { fReadBack = true; };

        void SetReadFalse() { fReadBack = false; };

        //sometimes there is no need to write the data from host -> device
        //for example, if another kernel has just run, and the data is already
        //present on the device, we can use it without doing a transferr
        void SetWriteTrue() { fWriteOut = true; };

        void SetWriteFalse() { fWriteOut = false; };

    protected:
        virtual bool InitializeInPlace(XArgType* in) override
        {
            fIsValid = false;
            if(in != nullptr)
            {
                fIsValid = true;
            }

            if(fIsValid)
            {
                //check if the current transform sizes are the same as the input
                bool need_to_resize = false;
                if(fDimensionSize[fActiveAxisIndex] != in->GetDimension(fActiveAxisIndex))
                {
                    need_to_resize = true;
                }
                if(need_to_resize)
                {
                    in->GetDimensions(fDimensionSize);
                    InitHostWorkspace();
                    InitDeviceWorkspace();
                }
                fInitialized = true;
            }
            return (fInitialized && fIsValid);
        }

        virtual bool ExecuteInPlace(XArgType* in) override {}

        // virtual bool InitializeInPlace(XArrayType* in)
        // {
        //     if(in != nullptr)
        //     {
        //         if( in->template HasExtension< MHO_OpenCLNDArrayBuffer< XArrayType > >() )
        //         {
        //             fArrayBuffer = in->template AsExtension< MHO_OpenCLNDArrayBuffer< XArrayType > >();
        //         }
        //         else
        //         {
        //             fArrayBuffer = in->template MakeExtension< MHO_OpenCLNDArrayBuffer< XArrayType > >();
        //         }
        //
        //         unsigned int array_size = in->GetSize();
        //
        //         fKernel->setArg(0,array_size);
        //         fKernel->setArg(1,fFactor);
        //         fKernel->setArg(2, *(fArrayBuffer->GetDataBuffer()) );
        //
        //         //pad out n-global to be a multiple of the n-local
        //         fNGlobal = array_size;
        //         unsigned int dummy = fNLocal - (array_size%fNLocal);
        //         if(dummy == fNLocal){dummy = 0;};
        //         fNGlobal += dummy;
        //
        //         fInitialized = true;
        //         return true;
        //     }
        //     return false;
        // }
        //
        //
        //
        // virtual bool ExecuteInPlace(XArrayType* in)
        // {
        //     if(fInitialized)
        //     {
        //         //write out the data to the device if we must, otherwise assume it is already on device
        //         if(fWriteOut)
        //         {
        //             fArrayBuffer->WriteDataBuffer();
        //         }
        //
        //         //now fire up the kernel
        //         MHO_OpenCLInterface::GetInstance()->GetQueue().enqueueNDRangeKernel(*fKernel, cl::NullRange, fNGlobal, fNLocal);
        //         #ifdef ENFORCE_CL_FINISH
        //         MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
        //         #endif
        //
        //         //read back data to the host if we must, otherwise, leave it on the device for the next kernel
        //         if(fReadBack)
        //         {
        //             fArrayBuffer->ReadDataBuffer();
        //         }
        //         return true;
        //     }
        //     return false;
        // }

        // virtual bool InitializeOutOfPlace(const XArrayType* in, XArrayType* out)
        // {
        //     ConditionallyResizeOutput(in->GetDimensionArray(), out);
        //     return InitializeInPlace(out);
        // }
        //
        //
        // virtual bool ExecuteOutOfPlace(const XArrayType* in, XArrayType* out)
        // {
        //     //This may not be the most efficient way to do this
        //     out->Copy(*in);
        //     bool cached_value = fWriteOut;
        //     fWriteOut = true;
        //     bool ret_val = ExecuteInPlace(out);
        //     fWriteOut = cached_value;
        //     return ret_val;
        // }

    private:
        void BuildKernel()
        {
            std::cout << "building opencl kernel" << std::endl;
            //Get name of kernel source file
            std::stringstream clFile;
            clFile << MHO_OpenCLInterface::GetInstance()->GetKernelPath()
                   << "/MHO_MultidimensionalFastFourierTransform_kernel.cl";

            //set the build options
            std::stringstream options;
            options << GetOpenCLFlags();

            MHO_OpenCLKernelBuilder k_builder;
            fKernel = k_builder.BuildKernel(clFile.str(), std::string("MultidimensionalFastFourierTransform_Radix2Stage"),
                                            options.str());

            //get n-local
            fNLocal =
                fFFTKernel->getWorkGroupInfo< CL_KERNEL_WORK_GROUP_SIZE >(MHO_OpenCLInterface::GetInstance()->GetDevice());
            fPreferredWorkgroupMultiple = fFFTKernel->getWorkGroupInfo< CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE >(
                MHO_OpenCLInterface::GetInstance()->GetDevice());
            if(fPreferredWorkgroupMultiple < fNLocal)
            {
                fNLocal = fPreferredWorkgroupMultiple;
            }
        }

        std::string GetOpenCLFlags()
        {
            //set the build options
            std::stringstream options;
            options << " -I " << MHO_OpenCLInterface::GetInstance()->GetKernelPath();
            return options.str();
        }

        bool fInitialized;
        XFactorType fFactor;
        unsigned int fNLocal;
        unsigned int fNGlobal;
        cl::Kernel* fKernel;
        bool fWriteOut;
        bool fReadBack;

        MHO_OpenCLNDArrayBuffer< XArrayType >* fArrayBuffer;

        //host workspace
        unsigned int fTotalDataSize;
        MHO_FastFourierTransformWorkspace fHostPlan;

        //device related parameters ////////////////////////////////////////////
        cl::Context fContext; //access to the OpenCL device context
        unsigned int fNLocal;
        unsigned int fNGlobal;
        unsigned int fPreferredWorkgroupMultiple;
        mutable cl::Kernel* fKernel;
        std::string fOpenCLFlags;

        unsigned int fMaxBufferSize;           //we use the same size for all of the buffers (max across all dimensions)
        cl::Buffer* fTwiddleBufferCL;          //buffer for the FFT twiddle factors
        cl::Buffer* fConjugateTwiddleBufferCL; //buffer for the conjugate FFT twiddle factors
        cl::Buffer* fScaleBufferCL;            //buffer for the bluestein scale factors
        cl::Buffer* fCirculantBufferCL;        //buffer for the bluestein circulant vector
        cl::Buffer* fPermuationArrayCL;        //buffer for the permutation array
        cl::Buffer* fWorkspaceBufferCL;        //buffer to global workspace

        void InitHostWorkspace()
        {
            fMaxBufferSize = 0;
            fHostPlan[i].Resize(fDimensionSize[i]);
            if(fMaxBufferSize < fHostPlans[i].GetN())
            {
                fMaxBufferSize = fHostPlans[i].GetN();
            }
            if(fMaxBufferSize < fHostPlans[i].GetM())
            {
                fMaxBufferSize = fHostPlans[i].GetM();
            }
        }

        void InitDeviceWorkspace()
        {
            DeallocateDeviceWorkspace();
            AllocateDeviceWorkspace();
        }

        void AllocateDeviceWorkspace()
        {
            std::cout << "building CL buffers" << std::endl;
            fDimensionBufferCL = new cl::Buffer(fContext, CL_MEM_READ_ONLY, XArgType::rank::value * sizeof(unsigned int));
            fTwiddleBufferCL = new cl::Buffer(fContext, CL_MEM_READ_ONLY, fMaxBufferSize * sizeof(CL_TYPE2));
            fConjugateTwiddleBufferCL = new cl::Buffer(fContext, CL_MEM_READ_ONLY, fMaxBufferSize * sizeof(CL_TYPE2));
            fScaleBufferCL = new cl::Buffer(fContext, CL_MEM_READ_ONLY, fMaxBufferSize * sizeof(CL_TYPE2));
            fCirculantBufferCL = new cl::Buffer(fContext, CL_MEM_READ_ONLY, fMaxBufferSize * sizeof(CL_TYPE2));
            fDataBufferCL = new cl::Buffer(fContext, CL_MEM_READ_WRITE, fTotalDataSize * sizeof(CL_TYPE2));
            fPermuationArrayCL = new cl::Buffer(fContext, CL_MEM_READ_ONLY, fMaxBufferSize * sizeof(unsigned int));
        }

        void DeallocateDeviceWorkspace()
        {
            std::cout << "deleting CL buffers" << std::endl;
            delete fKernel;
            delete fFFTBluesteinKernel;
            delete fDimensionBufferCL;
            delete fTwiddleBufferCL;
            delete fConjugateTwiddleBufferCL;
            delete fScaleBufferCL;
            delete fCirculantBufferCL;
            delete fDataBufferCL;
            delete fPermuationArrayCL;
            delete fWorkspaceBufferCL;
        }

        void ConstructOpenCLKernels()
        {
            std::cout << "building opencl kernels" << std::endl;
            //Get name of kernel source file
            std::stringstream clFile;
            clFile << MHO_OpenCLInterface::GetInstance()->GetKernelPath()
                   << "/MHO_MultidimensionalFastFourierTransform_kernel.cl";

            //set the build options
            std::stringstream options;
            options << GetOpenCLFlags();

            MHO_OpenCLKernelBuilder k_builder;
            fKernel = k_builder.BuildKernel(clFile.str(), std::string("MultidimensionalFastFourierTransform_Radix2Stage"),
                                            options.str());
            fFFTBluesteinKernel = k_builder.BuildKernel(
                clFile.str(), std::string("MultidimensionalFastFourierTransform_BluesteinStage"), options.str());

            //get n-local
            fNLocal =
                fFFTKernel->getWorkGroupInfo< CL_KERNEL_WORK_GROUP_SIZE >(MHO_OpenCLInterface::GetInstance()->GetDevice());
            fPreferredWorkgroupMultiple = fFFTKernel->getWorkGroupInfo< CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE >(
                MHO_OpenCLInterface::GetInstance()->GetDevice());
            if(fPreferredWorkgroupMultiple < fNLocal)
            {
                fNLocal = fPreferredWorkgroupMultiple;
            }

            //determine the largest global worksize
            fMaxNWorkItems = 0;
            for(unsigned int D = 0; D < XArgType::rank::value; D++)
            {
                //compute number of 1d fft's needed (n-global)
                unsigned int n_global = fDimensionSize[0];
                unsigned int n_local_1d_transforms = 1;
                for(unsigned int i = 0; i < XArgType::rank::value - 1; i++)
                {
                    if(i != D)
                    {
                        n_global *= fSpatialDim[i];
                        n_local_1d_transforms *= fSpatialDim[i];
                    };
                };

                //pad out n-global to be a multiple of the n-local
                unsigned int nDummy = fNLocal - (n_global % fNLocal);
                if(nDummy == fNLocal)
                {
                    nDummy = 0;
                };
                n_global += nDummy;

                if(fMaxNWorkItems < n_global)
                {
                    fMaxNWorkItems = n_global;
                };
            }
        }
};

} // namespace hops

#endif /*! end of include guard: MHO_OpenCLFastFourierTransformRadix2Stage_HH__ */
