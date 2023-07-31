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
                msg_error("operators", "Cannot transform axis with index: " << axis_index << "for array with rank: " << XArgType::rank::value << eom);
            }
        }

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

        bool fIsValid;
        bool fForward;
        bool fInitialized;

        size_t fDimensionSize[XArgType::rank::value];
        bool fAxesToXForm[XArgType::rank::value];

};


}

#endif /* MHO_OpenCLMultidimensionalFastFourierTransform_H__ */
