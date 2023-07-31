#ifndef MHO_OpenCLBatchedMultidimensionalFastFourierTransform_HH__
#define MHO_OpenCLBatchedMultidimensionalFastFourierTransform_HH__

#include <complex>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

//core (opencl)
#include "MHO_NDArrayWrapper.hh"
#include "MHO_FastFourierTransformUtilities.hh"
#include "MHO_UnaryOperator.hh"
#include "MHO_OpenCLInterface.hh"
#include "MHO_OpenCLKernelBuilder.hh"


#define ENFORCE_CL_FINISH

namespace hops
{


template< typename XArgType >
class MHO_OpenCLBatchedMultidimensionalFastFourierTransform :
    public MHO_Operator
{
  public:

    static_assert( is_complex< typename XArgType::value_type >::value, "Array element type must be a complex floating point type." );
    using complex_value_type = typename XArgType::value_type;
    using floating_point_value_type = typename XArgType::value_type::value_type;


    MHO_OpenCLBatchedMultidimensionalFastFourierTransform() :
        MHO_Operator(),
        fIsValid(false),
        fIsForward(true),
        fInitialized(false),
        fAllSpatialDimensionsAreEqual(true),
        fFillFromHostData(true),
        fReadOutDataToHost(true),
        fMaxBufferSize(0),
        fTotalDataSize(0),
        fOpenCLFlags(""),
        fFFTKernel(nullptr),
        fSpatialDimensionBufferCL(nullptr),
        fTwiddleBufferCL(nullptr),
        fConjugateTwiddleBufferCL(nullptr),
        fScaleBufferCL(nullptr),
        fCirculantBufferCL(nullptr),
        fDataBufferCL(nullptr),
        fPermuationArrayCL(nullptr),
        fWorkspaceBufferCL(nullptr),
        fNLocal(0)
    {
        ;
    };

    virtual ~MHO_OpenCLBatchedMultidimensionalFastFourierTransform()
    {
        delete fFFTKernel;
        delete fSpatialDimensionBufferCL;
        delete fTwiddleBufferCL;
        delete fConjugateTwiddleBufferCL;
        delete fScaleBufferCL;
        delete fCirculantBufferCL;
        delete fDataBufferCL;
        delete fPermuationArrayCL;
        delete fWorkspaceBufferCL;
    };

    virtual void SetInput(XArgType* in)
    {
        fInput = in;
    };
    virtual void SetOutput(XArgType* out)
    {
        fOutput = out;
    };

    private:
        XArgType* fInput;
        XArgType* fOutput;

    public:

    //control direction of FFT
    void SetForward()
    {
        fIsForward = true;
    }
    void SetBackward()
    {
        fIsForward = false;
    };

    //control whether GPU buffer is initialized from host data (fInput)
    void SetWriteOutHostDataTrue()
    {
        fFillFromHostData = true;
    };
    void SetWriteOutHostDataFalse()
    {
        fFillFromHostData = false;
    };

    //control whether result is read back to host (fOutput)
    void SetReadOutDataToHostTrue()
    {
        fReadOutDataToHost = true;
    };
    void SetReadOutDataToHostFalse()
    {
        fReadOutDataToHost = false;
    };

    ////////////////////////////////////////////////////////////////////////////////

    //opencl build flags used
    std::string GetOpenCLFlags() const
    {
        return fOpenCLFlags;
    };

    ////////////////////////////////////////////////////////////////////////////////

    //raw access to the OpenCL data buffer...use carefully
    cl::Buffer* GetDataBuffer()
    {
        return fDataBufferCL;
    };

    ////////////////////////////////////////////////////////////////////////////////

    //overides automatic determination of local workgroup size if set
    //do NOT provide any sanity checks
    void ForceLocalSize(unsigned int local)
    {
        fNLocal = local;
    };

    ////////////////////////////////////////////////////////////////////////////////

    virtual bool Initialize()
    {
        //input and output must be set before being called
        std::cout<<"initializing"<<std::endl;
        if (!fInitialized)  //can only be initialized once!
        {
            if (DoInputOutputDimensionsMatch()) {
                fIsValid = true;
                this->fInput->GetDimensions(fDimensionSize);
                for (unsigned int i = 0; i < XArgType::rank::value-1; i++) {
                    fSpatialDim[i] = fDimensionSize[i + 1];
                }
            }
            else {
                std::cout<<"dim mismatch"<<std::endl;
                fIsValid = false;
                fInitialized = false;
            }

            if (fIsValid) {
                std::cout<<"its valid"<<std::endl;
                ConstructWorkspace();
                ConstructOpenCLKernels();
                BuildBuffers();
                AssignBuffers();
                fInitialized = true;
            }
        }
        return fInitialized;
    }

    ////////////////////////////////////////////////////////////////////////////////

    virtual bool Execute()
    {
        if (fIsValid && fInitialized) {
            //set the basic arguments
            unsigned int n_multdim_ffts = fDimensionSize[0];
            fFFTKernel->setArg(0, n_multdim_ffts);  //number of complete multidimensional fft's to perform
            if (fIsForward) {
                fFFTKernel->setArg(2, 1);  //direction of FFT is forward
            }
            else {
                fFFTKernel->setArg(2, 0);  //direction of FFT is backward (inverse)
            }
            //arg 3 is dimensions, already written to GPU

            //write the data to the buffer if necessary
            FillDataBuffer();

            for (unsigned int D = 0; D < XArgType::rank::value; D++) {
                //compute number of 1d fft's needed (n-global)
                unsigned int n_global = fDimensionSize[0];
                unsigned int n_local_1d_transforms = 1;
                for (unsigned int i = 0; i < XArgType::rank::value -1; i++) {
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

                cl::NDRange global(n_global);
                cl::NDRange local(fNLocal);

                fFFTKernel->setArg(1, D);  //(index of the selected dimension) updated at each stage

                if (fAllSpatialDimensionsAreEqual) {
                    //no need to write the constants, as they are already on the gpu
                    //just fire up the kernel
                    MHO_OpenCLInterface::GetInstance()->GetQueue().enqueueNDRangeKernel(*fFFTKernel,
                                                                                     cl::NullRange,
                                                                                     global,
                                                                                     local);
#ifdef ENFORCE_CL_FINISH
                    MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
#endif
                }
                else {
                    //we enqueue write the needed constants for this dimension
                    MHO_OpenCLInterface::GetInstance()->GetQueue().enqueueWriteBuffer(*fTwiddleBufferCL,
                                                                                   CL_TRUE,
                                                                                   0,
                                                                                   fMaxBufferSize * sizeof(CL_TYPE2),
                                                                                   &(fTwiddle[D][0]));
#ifdef ENFORCE_CL_FINISH
                    MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
#endif
                    MHO_OpenCLInterface::GetInstance()->GetQueue().enqueueWriteBuffer(*fConjugateTwiddleBufferCL,
                                                                                   CL_TRUE,
                                                                                   0,
                                                                                   fMaxBufferSize * sizeof(CL_TYPE2),
                                                                                   &(fConjugateTwiddle[D][0]));
#ifdef ENFORCE_CL_FINISH
                    MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
#endif
                    MHO_OpenCLInterface::GetInstance()->GetQueue().enqueueWriteBuffer(*fScaleBufferCL,
                                                                                   CL_TRUE,
                                                                                   0,
                                                                                   fMaxBufferSize * sizeof(CL_TYPE2),
                                                                                   &(fScale[D][0]));
#ifdef ENFORCE_CL_FINISH
                    MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
#endif
                    MHO_OpenCLInterface::GetInstance()->GetQueue().enqueueWriteBuffer(*fCirculantBufferCL,
                                                                                   CL_TRUE,
                                                                                   0,
                                                                                   fMaxBufferSize * sizeof(CL_TYPE2),
                                                                                   &(fCirculant[D][0]));
#ifdef ENFORCE_CL_FINISH
                    MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
#endif
                    MHO_OpenCLInterface::GetInstance()->GetQueue().enqueueWriteBuffer(*fPermuationArrayCL,
                                                                                   CL_TRUE,
                                                                                   0,
                                                                                   fMaxBufferSize *
                                                                                       sizeof(unsigned int),
                                                                                   &(fPermuationArray[D][0]));
#ifdef ENFORCE_CL_FINISH
                    MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
#endif

                    //now enqueue the kernel
                    MHO_OpenCLInterface::GetInstance()->GetQueue().enqueueNDRangeKernel(*fFFTKernel,
                                                                                     cl::NullRange,
                                                                                     global,
                                                                                     local);
#ifdef ENFORCE_CL_FINISH
                    MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
#endif
                }
            }

            //read the data from the buffer if necessary
            ReadOutDataBuffer();
            return true;
        }
        else {
            std::cout
                << "MHO_OpenCLBatchedMultidimensionalFastFourierTransform::Execute: Not valid and initialized. Aborting."
                << std::endl;
            return false;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////

  private:
    void ConstructWorkspace()
    {
        //figure out the size of all the data
        fTotalDataSize = MHO_NDArrayMath::TotalArraySize<XArgType::rank::value>(fDimensionSize);

        //figure out the size of the private buffers needed
        fMaxBufferSize = 0;
        fAllSpatialDimensionsAreEqual = true;
        unsigned int previous_dim = fSpatialDim[0];
        for (unsigned int i = 0; i < XArgType::rank::value-1; i++) {
            if (previous_dim != fSpatialDim[i]) {
                fAllSpatialDimensionsAreEqual = false;
            };

            if (fSpatialDim[i] > fMaxBufferSize) {
                fMaxBufferSize = fSpatialDim[i];
            }

            if (!(MHO_BitReversalPermutation::IsPowerOfTwo(fSpatialDim[i]))) {
                if (MHO_FastFourierTransformUtilities< floating_point_value_type >::ComputeBluesteinArraySize(fSpatialDim[i]) > fMaxBufferSize) {
                    fMaxBufferSize = MHO_FastFourierTransformUtilities< floating_point_value_type >::ComputeBluesteinArraySize(fSpatialDim[i]);
                }
            }
        }

        //create the build flags
        std::stringstream ss;
        ss << " -D FFT_NDIM=" << XArgType::rank::value;
        ss << " -D FFT_BUFFERSIZE=" << fMaxBufferSize;

        // //determine the size of the device's constant memory buffer, if it is too small we do not
        // //use it, and use global memory instead
        // size_t const_mem_size =
        //     MHO_OpenCLInterface::GetInstance()->GetDevice().getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>();
        // if (fMaxBufferSize * sizeof(CL_TYPE2) < const_mem_size) {
        //     ss << " -D FFT_USE_CONST_MEM";
        // }

        ss << " -I " << MHO_OpenCLInterface::GetInstance()->GetKernelPath();
        fOpenCLFlags = ss.str();

        //now we need to compute all the auxiliary variables we need for the FFT
        for (unsigned int i = 0; i < XArgType::rank::value-1; i++) {
            unsigned int N = fSpatialDim[i];
            unsigned int M = N;

            if (!(MHO_BitReversalPermutation::IsPowerOfTwo(N))) {
                M = MHO_FastFourierTransformUtilities< floating_point_value_type >::ComputeBluesteinArraySize(N);
            }

            //resize to appropriate lengths
            fTwiddle[i].resize(M);
            fConjugateTwiddle[i].resize(M);
            fScale[i].resize(M);
            fCirculant[i].resize(M);
            fPermuationArray[i].resize(M);

            //compute the twiddle factors for this dimension
            MHO_FastFourierTransformUtilities< floating_point_value_type >::ComputeTwiddleFactors(M, &(fTwiddle[i][0]));

            //compute the conjugate twiddle factors
            MHO_FastFourierTransformUtilities< floating_point_value_type >::ComputeConjugateTwiddleFactors(M, &(fConjugateTwiddle[i][0]));

            //compute the bluestein scale factors for this dimension
            MHO_FastFourierTransformUtilities< floating_point_value_type >::ComputeBluesteinScaleFactors(N, &(fScale[i][0]));
            //compute the circulant vector for this dimension
            MHO_FastFourierTransformUtilities< floating_point_value_type >::ComputeBluesteinCirculantVector(N,
                                                                              M,
                                                                              &(fTwiddle[i][0]),
                                                                              &(fScale[i][0]),
                                                                              &(fCirculant[i][0]));

            MHO_BitReversalPermutation::ComputeBitReversedIndicesBaseTwo(M, &(fPermuationArray[i][0]));

            std::cout<<"build workspace"<<std::endl;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////

    void ConstructOpenCLKernels()
    {
        std::cout<<"opencl kernels"<<std::endl;
        //Get name of kernel source file
        std::stringstream clFile;
        clFile << MHO_OpenCLInterface::GetInstance()->GetKernelPath()
               << "/MHO_MultidimensionalFastFourierTransform_kernel.cl";

        //set the build options
        std::stringstream options;
        options << GetOpenCLFlags();

        MHO_OpenCLKernelBuilder k_builder;
        fFFTKernel = k_builder.BuildKernel(clFile.str(),
                                           std::string("MultidimensionalFastFourierTransform_Stage"),
                                           options.str());

        //get n-local
        if (fNLocal == 0)  //if fNLocal has already been set externally, do nothing
        {
            fNLocal =
                fFFTKernel->getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(MHO_OpenCLInterface::GetInstance()->GetDevice());

            fPreferredWorkgroupMultiple = fFFTKernel->getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(
                MHO_OpenCLInterface::GetInstance()->GetDevice());

            if (fPreferredWorkgroupMultiple < fNLocal) {
                fNLocal = fPreferredWorkgroupMultiple;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////

    void BuildBuffers()
    {
        std::cout<<"building buffers"<<std::endl;
        //buffer for the 'spatial' dimensions of the array
        fSpatialDimensionBufferCL = new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(),
                                                   CL_MEM_READ_ONLY,
                                                   XArgType::rank::value * sizeof(unsigned int));

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
                                            fMaxBufferSize * sizeof(unsigned int));

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

    ////////////////////////////////////////////////////////////////////////////////

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
        fFFTKernel->setArg(3, *fSpatialDimensionBufferCL);
        Q.enqueueWriteBuffer(*fSpatialDimensionBufferCL, CL_TRUE, 0, (XArgType::rank::value-1) * sizeof(unsigned int), fSpatialDim);
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

    ////////////////////////////////////////////////////////////////////////////////

    void FillDataBuffer()
    {
        if (fFillFromHostData) {
            //initialize data on the GPU from host memory
            cl::CommandQueue& Q = MHO_OpenCLInterface::GetInstance()->GetQueue();
            auto* ptr = (CL_TYPE2*) (&((this->fInput->GetData())[0]));
            std::cout<<"total data size = "<<fTotalDataSize<<std::endl;
            Q.enqueueWriteBuffer(*fDataBufferCL, CL_TRUE, 0, fTotalDataSize * sizeof(CL_TYPE2), ptr);
#ifdef ENFORCE_CL_FINISH
            MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
#endif
        }

        //otherwise assume data is already on GPU from previous result
    }

    ////////////////////////////////////////////////////////////////////////////////

    void ReadOutDataBuffer()
    {
        if (fReadOutDataToHost) {
            //read out data from the GPU to the host memory
            cl::CommandQueue& Q = MHO_OpenCLInterface::GetInstance()->GetQueue();
            auto* ptr = (CL_TYPE2*) (&((this->fInput->GetData())[0]));
            Q.enqueueReadBuffer(*fDataBufferCL, CL_TRUE, 0, fTotalDataSize * sizeof(CL_TYPE2), ptr);
#ifdef ENFORCE_CL_FINISH
            MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
#endif
        }
        //otherwise assume we want to leave the data on the GPU for further processing
    }

    ////////////////////////////////////////////////////////////////////////////////

    virtual bool DoInputOutputDimensionsMatch()
    {
        size_t in[XArgType::rank::value];
        size_t out[XArgType::rank::value];

        this->fInput->GetDimensions(in);
        this->fOutput->GetDimensions(out);

        for (unsigned int i = 0; i < XArgType::rank::value; i++) {
            if (in[i] != out[i]) {
                return false;
            }
        }
        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////

    bool fIsValid;
    bool fIsForward;
    bool fInitialized;
    bool fAllSpatialDimensionsAreEqual;
    bool fFillFromHostData;
    bool fReadOutDataToHost;
    size_t fDimensionSize[XArgType::rank::value];
    unsigned int fSpatialDim[XArgType::rank::value-1];
    unsigned int fMaxNWorkItems;

    unsigned int fMaxBufferSize;
    unsigned int fTotalDataSize;

    ////////////////////////////////////////////////////////////////////////
    //Workspace for needed coefficients
    std::vector<std::complex<double>> fTwiddle[XArgType::rank::value];
    std::vector<std::complex<double>> fConjugateTwiddle[XArgType::rank::value];
    std::vector<std::complex<double>> fScale[XArgType::rank::value];
    std::vector<std::complex<double>> fCirculant[XArgType::rank::value];
    std::vector<unsigned int> fPermuationArray[XArgType::rank::value];

    ////////////////////////////////////////////////////////////////////////

    std::string fOpenCLFlags;

    mutable cl::Kernel* fFFTKernel;

    //buffer for the spatial dimensions of each block to be transformed
    cl::Buffer* fSpatialDimensionBufferCL;

    //buffer for the FFT twiddle factors
    cl::Buffer* fTwiddleBufferCL;

    //buffer for the conjugate FFT twiddle factors
    cl::Buffer* fConjugateTwiddleBufferCL;

    //buffer for the bluestein scale factors
    cl::Buffer* fScaleBufferCL;

    //buffer for the bluestein circulant vector
    cl::Buffer* fCirculantBufferCL;

    //buffer for the data to be transformed
    cl::Buffer* fDataBufferCL;

    //buffer for the permutation array
    cl::Buffer* fPermuationArrayCL;

    //buffer to global workspace
    cl::Buffer* fWorkspaceBufferCL;

    unsigned int fNLocal;
    unsigned int fNGlobal;
    unsigned int fPreferredWorkgroupMultiple;

    ////////////////////////////////////////////////////////////////////////
};


}  // namespace hops

#endif /* MHO_OpenCLBatchedMultidimensionalFastFourierTransform_H__ */
