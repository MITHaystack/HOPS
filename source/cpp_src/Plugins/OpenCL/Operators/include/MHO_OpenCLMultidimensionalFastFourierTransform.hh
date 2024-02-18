#ifndef MHO_OpenCLMultidimensionalFastFourierTransform_HH__
#define MHO_OpenCLMultidimensionalFastFourierTransform_HH__

#include <cstring>

#include "MHO_Meta.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryInPlaceOperator.hh"

#include "MHO_FastFourierTransform.hh"
#include "MHO_FastFourierTransformWorkspace.hh"
#include "MHO_FastFourierTransformUtilities.hh"
#include "MHO_MultidimensionalFastFourierTransformInterface.hh"

#include "MHO_OpenCLInterface.hh"
#include "MHO_OpenCLKernelBuilder.hh"

#include "MHO_TableContainer.hh"

namespace hops
{

template< typename XArgType >
class MHO_OpenCLMultidimensionalFastFourierTransform:
    public MHO_UnaryInPlaceOperator< XArgType >,
    public MHO_MultidimensionalFastFourierTransformInterface< XArgType >
{
    public:

        static_assert( is_complex< typename XArgType::value_type >::value, "Array element type must be a complex floating point type." );
        using complex_value_type = typename XArgType::value_type;
        using floating_point_value_type = typename XArgType::value_type::value_type;

        MHO_OpenCLMultidimensionalFastFourierTransform():
            MHO_MultidimensionalFastFourierTransformInterface< XArgType >()
        {

            fContext = MHO_OpenCLInterface::GetInstance()->GetContext();

        };

        virtual ~MHO_OpenCLMultidimensionalFastFourierTransform()
        {
            DeallocateDeviceWorkspace();
        };

    protected:

        virtual bool InitializeInPlace(XArgType* in) override
        {
            if( in != nullptr ){fIsValid = true;}
            else{fIsValid = false;}

            if(fIsValid)
            {
                //check if the current transform sizes are the same as the input
                bool need_to_resize = false;
                for(std::size_t i=0; i<XArgType::rank::value; i++)
                {
                    if(fDimensionSize[i] != in->GetDimension(i)){need_to_resize = true; break;}
                }
                if(need_to_resize)
                {
                    in->GetDimensions(fDimensionSize);

                    InitHostWorkspace();
                    InitDeviceWorkspace();
                    // DeallocateDeviceWorkspace();
                    // AllocateDeviceWorkspace();
                }
                fInitialized = true;
            }
            return (fInitialized && fIsValid);
        }


        virtual bool ExecuteInPlace(XArgType* in) override
        {

        }


    private:


        // bool fIsValid;
        // bool fForward;
        // bool fInitialized;
        // bool fTransformAxisLabels;
        // size_t fDimensionSize[XArgType::rank::value];
        // bool fAxesToXForm[XArgType::rank::value];

        //host workspace
        unsigned int fTotalDataSize;
        MHO_FastFourierTransformWorkspace fHostPlans[XArgType::rank::value];

        //device related parameters ////////////////////////////////////////////
        cl::Context fContext; //access to the OpenCL device context
        unsigned int fNLocal;
        unsigned int fNGlobal;
        unsigned int fPreferredWorkgroupMultiple;
        mutable cl::Kernel* fFFTRadix2Kernel;
        mutable cl::Kernel* fFFTBluesteinKernel;
        std::string fOpenCLFlags;

        unsigned int fMaxBufferSize; //we use the same size for all of the buffers (max across all dimensions)
        cl::Buffer* fDimensionBufferCL; //buffer for the dimensions of the array
        cl::Buffer* fTwiddleBufferCL; //buffer for the FFT twiddle factors
        cl::Buffer* fConjugateTwiddleBufferCL; //buffer for the conjugate FFT twiddle factors
        cl::Buffer* fScaleBufferCL; //buffer for the bluestein scale factors
        cl::Buffer* fCirculantBufferCL; //buffer for the bluestein circulant vector
        cl::Buffer* fDataBufferCL; //buffer for the data to be transformed
        cl::Buffer* fPermuationArrayCL; //buffer for the permutation array
        cl::Buffer* fWorkspaceBufferCL; //buffer to global workspace

        void InitHostWorkspace()
        {
            fMaxBufferSize = 0;
            for(std::size_t i=0; i<XArgType::rank::value; i++)
            {
                if( fAxesToXForm[i] )
                {
                    fHostPlans[i].Resize(fDimensionSize[i]);
                    if(fMaxBufferSize < fHostPlans[i].GetN() ){ fMaxBufferSize = fHostPlans[i].GetN(); }
                    if(fMaxBufferSize < fHostPlans[i].GetM() ){ fMaxBufferSize = fHostPlans[i].GetM(); }
                }
            }
        }


        void InitDeviceWorkspace()
        {
            DeallocateDeviceWorkspace();
            AllocateDeviceWorkspace();
        }

        void AllocateDeviceWorkspace()
        {
            std::cout<<"building CL buffers"<<std::endl;
            fDimensionBufferCL = new cl::Buffer(fContext, CL_MEM_READ_ONLY, XArgType::rank::value * sizeof(unsigned int));
            fTwiddleBufferCL = new cl::Buffer(fContext, CL_MEM_READ_ONLY, fMaxBufferSize * sizeof(CL_TYPE2));
            fConjugateTwiddleBufferCL = new cl::Buffer(fContext, CL_MEM_READ_ONLY, fMaxBufferSize * sizeof(CL_TYPE2));
            fScaleBufferCL = new cl::Buffer(fContext, CL_MEM_READ_ONLY, fMaxBufferSize * sizeof(CL_TYPE2));
            fCirculantBufferCL = new cl::Buffer(fContext, CL_MEM_READ_ONLY, fMaxBufferSize * sizeof(CL_TYPE2));
            fDataBufferCL = new cl::Buffer(fContext, CL_MEM_READ_WRITE, fTotalDataSize * sizeof(CL_TYPE2));
            fPermuationArrayCL = new cl::Buffer(fContext, CL_MEM_READ_ONLY,fMaxBufferSize * sizeof(unsigned int));
        }

        void DeallocateDeviceWorkspace()
        {
            std::cout<<"deleting CL buffers"<<std::endl;
            delete fFFTRadix2Kernel;
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
            std::cout<<"building opencl kernels"<<std::endl;
            //Get name of kernel source file
            std::stringstream clFile;
            clFile << MHO_OpenCLInterface::GetInstance()->GetKernelPath() << "/MHO_MultidimensionalFastFourierTransform_kernel.cl";

            //set the build options
            std::stringstream options;
            options << GetOpenCLFlags();

            MHO_OpenCLKernelBuilder k_builder;
            fFFTRadix2Kernel = k_builder.BuildKernel(clFile.str(), std::string("MultidimensionalFastFourierTransform_Radix2Stage"), options.str());
            fFFTBluesteinKernel = k_builder.BuildKernel(clFile.str(), std::string("MultidimensionalFastFourierTransform_BluesteinStage"), options.str());

            //get n-local
            fNLocal = fFFTKernel->getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(MHO_OpenCLInterface::GetInstance()->GetDevice());
            fPreferredWorkgroupMultiple = fFFTKernel->getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>( MHO_OpenCLInterface::GetInstance()->GetDevice());
            if (fPreferredWorkgroupMultiple < fNLocal){fNLocal = fPreferredWorkgroupMultiple;}

            //determine the largest global worksize
            fMaxNWorkItems = 0;
            for (unsigned int D = 0; D < XArgType::rank::value; D++) {
                //compute number of 1d fft's needed (n-global)
                unsigned int n_global = fDimensionSize[0];
                unsigned int n_local_1d_transforms = 1;
                for (unsigned int i = 0; i < XArgType::rank::value-1; i++) {
                    if (i != D) {
                        n_global *= fSpatialDim[i];
                        n_local_1d_transforms *= fSpatialDim[i];
                    };
                };

                //pad out n-global to be a multiple of the n-local
                unsigned int nDummy = fNLocal - (n_global % fNLocal);
                if (nDummy == fNLocal) {
                    nDummy = 0;
                };
                n_global += nDummy;

                if (fMaxNWorkItems < n_global) {
                    fMaxNWorkItems = n_global;
                };
            }

        }




        //data



};


}

#endif /* MHO_OpenCLMultidimensionalFastFourierTransform_H__ */
