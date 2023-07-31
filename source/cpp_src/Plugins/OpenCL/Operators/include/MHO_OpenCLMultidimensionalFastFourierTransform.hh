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

        };

        virtual ~MHO_OpenCLMultidimensionalFastFourierTransform()
        {
            DeallocateWorkspace();
        };

    protected:

        virtual bool InitializeInPlace(XArgType* in) override
        {
            // if( in != nullptr ){fIsValid = true;}
            // else{fIsValid = false;}
            // 
            // if(fIsValid)
            // {
            //     //check if the current transform sizes are the same as input
            //     bool need_to_resize = false;
            //     for(std::size_t i=0; i<XArgType::rank::value; i++)
            //     {
            //         if(fDimensionSize[i] != in->GetDimension(i)){need_to_resize = true; break;}
            //     }
            //     if(need_to_resize)
            //     {
            //         in->GetDimensions(fDimensionSize);
            //         DeallocateWorkspace();
            //         AllocateWorkspace();
            //     }
            //     fInitialized = true;
            // }
            // return (fInitialized && fIsValid);
        }


        virtual bool ExecuteInPlace(XArgType* in) override
        {
            
        }


    private:


















        
        //data



};


}

#endif /* MHO_OpenCLMultidimensionalFastFourierTransform_H__ */
