#ifndef MHO_OpenCLNDArrayBuffer_HH__
#define MHO_OpenCLNDArrayBuffer_HH__

/*
*File: MHO_OpenCLNDArrayBuffer.hh
*Class: MHO_OpenCLNDArrayBuffer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include "MHO_OpenCLInterface.hh"
#include "MHO_ExtensibleElement.hh"

namespace hops
{

template< typename XArrayType >
class MHO_OpenCLNDArrayBuffer
{
    public:

        MHO_OpenCLNDArrayBuffer(MHO_ExtensibleElement* element):
            fElement(element),
            fDimensionBufferCL(nullptr),
            fDataBufferCL(nullptr)
        {
            fRank = XArrayType::rank::value;
            fNDArray = dynamic_cast< XArrayType* >(element);
        };


        virtual ~MHO_OpenCLNDArrayBuffer()
        {
            delete fDimensionBufferCL;
            delete fDataBufferCL;
        };

        void ConstructDimensionBuffer()
        {
            //buffer for the dimensions of the array
            unsigned int n_bytes = (fRank)*sizeof(unsigned int);
            CL_ERROR_TRY
            fDimensionBufferCL =
            new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(), CL_MEM_READ_ONLY, n_bytes);
            CL_ERROR_CATCH
        }

        void ConstructDataBuffer()
        {
            //buffer for the data
            unsigned int n_bytes = (static_cast< unsigned int >( fNDArray->GetSize() ) )*sizeof( typename MHO_OpenCLTypeMap< typename XArrayType::value_type  >::mapped_type );
            CL_ERROR_TRY
            fDataBufferCL =
            new cl::Buffer(MHO_OpenCLInterface::GetInstance()->GetContext(), CL_MEM_READ_WRITE, n_bytes);
            CL_ERROR_CATCH
        }

        cl::Buffer* GetDimensionBuffer(){return fDimensionBufferCL;};
        cl::Buffer* GetDataBuffer(){return fDataBufferCL;}

        void WriteDimensionBuffer()
        {
            //writes the dimensions of the array from the host to the device
            cl::CommandQueue& Q = MHO_OpenCLInterface::GetInstance()->GetQueue();
            unsigned int n_bytes = fRank*sizeof(unsigned int);
            for(unsigned int i=0; i<fRank; i++)
            {
                fDimensions[i] = static_cast< unsigned int >( fNDArray->GetDimension(i) );
            }
            //we enqueue write the needed constants for this dimension
            CL_ERROR_TRY
            Q.enqueueWriteBuffer(*fDimensionBufferCL, CL_TRUE, 0, n_bytes, &(fDimensions[0]) );
            CL_ERROR_CATCH
            #ifdef ENFORCE_CL_FINISH
            Q.finish();
            #endif
        }

        void WriteDataBuffer()
        {
            //Writes data from the host to device
            cl::CommandQueue& Q = MHO_OpenCLInterface::GetInstance()->GetQueue();
            unsigned int n_bytes = (static_cast< unsigned int >( fNDArray->GetSize() ) )*sizeof( typename MHO_OpenCLTypeMap< typename XArrayType::value_type  >::mapped_type );
            typename MHO_OpenCLTypeMap< typename XArrayType::value_type  >::mapped_type* ptr;
            ptr = reinterpret_cast< typename MHO_OpenCLTypeMap< typename XArrayType::value_type  >::mapped_type* >(fNDArray->GetData() );
            CL_ERROR_TRY
            Q.enqueueWriteBuffer(*fDataBufferCL, CL_TRUE, 0, n_bytes, ptr );
            CL_ERROR_CATCH
            #ifdef ENFORCE_CL_FINISH
            Q.finish();
            #endif
        }

        void ReadDataBuffer()
        {
            //TODO FIXME -- need to make sure  typename XArrayType::value_type   is the same size as equivalent OpenCL type we are mapping to
            //read out data from the GPU to the host memory
            cl::CommandQueue& Q = MHO_OpenCLInterface::GetInstance()->GetQueue();
            unsigned int n_bytes = (static_cast< unsigned int >( fNDArray->GetSize() ) )*sizeof( typename MHO_OpenCLTypeMap< typename XArrayType::value_type  >::mapped_type );
            typename MHO_OpenCLTypeMap< typename XArrayType::value_type  >::mapped_type* ptr;
            ptr = reinterpret_cast< typename MHO_OpenCLTypeMap< typename XArrayType::value_type  >::mapped_type* >(fNDArray->GetData() );
            CL_ERROR_TRY
            Q.enqueueReadBuffer(*fDataBufferCL, CL_TRUE, 0, n_bytes, ptr);
            CL_ERROR_CATCH
            #ifdef ENFORCE_CL_FINISH
            Q.finish();
            #endif
        }

    protected:

        MHO_ExtensibleElement* fElement;
        XArrayType* fNDArray;
        unsigned int fRank;

        //The OpenCL buffers associated with this table are:
        //(1) A buffer containing the dimensions of the table + total size
        //(2) A buffer for the ND-data array

        //buffer for the dimensions of the array
        //this buffer must be made of unsigned int (fixed width), as opencl doesnt support size_t
        unsigned int fDimensions[XArrayType::rank::value];

        //buffer for the array dimensions
        cl::Buffer* fDimensionBufferCL;

        //buffer for the array data
        cl::Buffer* fDataBufferCL;
};


}//end of hops namespace

#endif /* end of include guard: MHO_OpenCLNDArrayBuffer */
