#ifndef MHO_OpenCLScalarMultiply_HH__
#define MHO_OpenCLScalarMultiply_HH__

#include <algorithm>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryNDArrayOperator.hh"

#include "MHO_OpenCLInterface.hh"
#include "MHO_OpenCLKernelBuilder.hh"
#include "MHO_OpenCLNDArrayBuffer.hh"

/*
*File: MHO_OpenCLScalarMultiply.hh
*Class: MHO_OpenCLScalarMultiply
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

namespace hops
{

template< typename XFactorType, class XInputArrayType >
class MHO_OpenCLScalarMultiply: public MHO_UnaryNDArrayOperator< XInputArrayType >
{
    public:

        MHO_OpenCLScalarMultiply():
            fInitialized(false),
            fFactor(0),
            fNLocal(0),
            fNGlobal(0),
            fKernel(nullptr),
            fWriteOut(true), //default is always to write host -> device
            fReadBack(true) //default is always to read device -> host
        {
            BuildKernel();
        };

        virtual ~MHO_OpenCLScalarMultiply()
        {
            delete fKernel;
        };

        void SetFactor(XFactorType factor){fFactor = factor;};
        XFactorType GetFactor() const {return fFactor;};

        //sometimes there is no need to read the data back from device -> host
        //for example, if there is another kernel using the same data that
        //is running immediately afterwards, we can just leave the data there
        void SetReadTrue(){fReadBack = true;};
        void SetReadFalse(){fReadBack = false;};

        //sometimes there is no need to write the data from host -> device
        //for example, if another kernel has just run, and the data is already
        //present on the device, we can use it without doing a transferr
        void SetWriteTrue(){fWriteOut = true;};
        void SetWriteFalse(){fWriteOut = false;};

        virtual bool Initialize() override
        {
            if(this->fInput != nullptr)
            {
                if( this->fInput->template HasExtension< MHO_OpenCLNDArrayBuffer< XInputArrayType > >() )
                {
                    fArrayBuffer = this->fInput->template AsExtension< MHO_OpenCLNDArrayBuffer< XInputArrayType > >();
                }
                else
                {
                    fArrayBuffer = this->fInput->template MakeExtension< MHO_OpenCLNDArrayBuffer< XInputArrayType > >();
                }

                unsigned int array_size = this->fInput->GetSize();

                fKernel->setArg(0,array_size);
                fKernel->setArg(1,fFactor);
                fKernel->setArg(2, *(fArrayBuffer->GetDataBuffer()) );

                //pad out n-global to be a multiple of the n-local
                fNGlobal = array_size;
                unsigned int dummy = fNLocal - (array_size%fNLocal);
                if(dummy == fNLocal){dummy = 0;};
                fNGlobal += dummy;

                fInitialized = true;
                return true;
            }
            return false;
        }

        virtual bool ExecuteOperation() override
        {
            if(fInitialized)
            {
                //write out the data to the device if we must, otherwise assume it is already on device
                if(fWriteOut)
                {
                    fArrayBuffer->WriteDataBuffer();
                }

                //now fire up the kernel
                MHO_OpenCLInterface::GetInstance()->GetQueue().enqueueNDRangeKernel(*fKernel, cl::NullRange, fNGlobal, fNLocal);
                #ifdef ENFORCE_CL_FINISH
                MHO_OpenCLInterface::GetInstance()->GetQueue().finish();
                #endif

                //read back data to the host if we must, otherwise, leave it on the device for the next kernel
                if(fReadBack)
                {
                    fArrayBuffer->ReadDataBuffer();
                }
                return true;
            }
            return false;
        }


    private:

        void BuildKernel()
        {
            //Get name of kernel source file
            std::stringstream clFile;
            clFile << MHO_OpenCLInterface::GetInstance()->GetKernelPath() << "/MHO_VectorScale_kernel.cl";

            std::string flags = GetOpenCLFlags();

            MHO_OpenCLKernelBuilder k_builder;
            fKernel = k_builder.BuildKernel(clFile.str(), std::string("VectorScale"), flags );

            fNLocal = fKernel->getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(MHO_OpenCLInterface::GetInstance()->GetDevice());
            unsigned int preferredWorkgroupMultiple = fKernel->getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(MHO_OpenCLInterface::GetInstance()->GetDevice() );
            if(preferredWorkgroupMultiple < fNLocal)
            {
                fNLocal = preferredWorkgroupMultiple;
            }

        }


        std::string GetOpenCLFlags()
        {
            //set the build options
            std::stringstream options;
            options << " -I " << MHO_OpenCLInterface::GetInstance()->GetKernelPath();

            std::string factor_type = MHO_ClassName< XFactorType >();
            std::string data_type = MHO_ClassName< typename XInputArrayType::value_type >();

            //pass flag that we have to do complex multiply
            if( factor_type.find( std::string("complex") ) != std::string::npos &&
                data_type.find( std::string("complex") ) != std::string::npos )
            {
                options << " -D COMPLEX_COMPLEX";
            }

            //figure out the data type defines to insert in the OpenCL kernel
            options << " -D CL_FACTOR_TYPE=" << MHO_ClassName<  typename MHO_OpenCLTypeMap< XFactorType >::mapped_type >();
            options << " -D CL_DATA_TYPE=" << MHO_ClassName<  typename MHO_OpenCLTypeMap< typename XInputArrayType::value_type  >::mapped_type >();

            return options.str();
        }

        bool fInitialized;
        XFactorType fFactor;
        unsigned int fNLocal;
        unsigned int fNGlobal;
        cl::Kernel* fKernel;
        bool fWriteOut;
        bool fReadBack;

        MHO_OpenCLNDArrayBuffer< XInputArrayType >* fArrayBuffer;


};

}


#endif /* MHO_OpenCLScalarMultiply_H__ */
