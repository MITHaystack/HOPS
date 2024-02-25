#ifndef MHO_ElementTypeCaster_HH__
#define MHO_ElementTypeCaster_HH__


#include "MHO_NDArrayWrapper.hh"
#include "MHO_TransformingOperator.hh"

#include "MHO_ContainerDefinitions.hh"

namespace hops
{

/*!
*@file MHO_ElementTypeCaster.hh
*@class MHO_ElementTypeCaster
*@author J. Barrett - barrettj@mit.edu
*@date Thu Apr 13 15:19:20 2023 -0400
*@brief operator class which casts the element type of one array type to another
* (e.g float -> double or double -> etc.)
*/


template<class XArgType1, class XArgType2> //args implicitly assumed to inherit from MHO_NDArrayWrapper
class MHO_ElementTypeCaster: public MHO_TransformingOperator< XArgType1, XArgType2>
{
    public:
        MHO_ElementTypeCaster(){};
        virtual ~MHO_ElementTypeCaster(){};

    protected:

        virtual bool InitializeImpl(const XArgType1* /*!in*/, XArgType2* /*!out*/) override {return true;}; //no op

        virtual bool ExecuteImpl(const XArgType1* in, XArgType2* out) override
        {
            if(in != nullptr && out != nullptr)
            {
                //resize the output array
                out->Resize( in->GetDimensions() );

                //copy the contents
                std::size_t total_size = in->GetSize();
                for(std::size_t i=0; i<total_size; i++)
                {
                    (*out)[i] = static_cast< typename XArgType2::value_type >( (*in)[i] );
                }

                IfTableCopyAxesAndTags(in,out);
                return true;
            }
            return false;
        }

    private:


        //default...does nothing
        template<class XArrayType1, class XArrayType2>
        typename std::enable_if< !(std::is_base_of<MHO_TableContainerBase, XArrayType1>::value &&
                                   std::is_base_of<MHO_TableContainerBase, XArrayType2>::value), void >::type
        IfTableCopyAxesAndTags(const XArrayType1* /*!in*/, XArrayType2* /*!out*/){};



        //use SFINAE to generate specialization for MHO_TableContainer types
        template<class XArrayType1, class XArrayType2>
        typename std::enable_if< std::is_base_of<MHO_TableContainerBase, XArrayType1>::value &&
                                 std::is_base_of<MHO_TableContainerBase, XArrayType2>::value, void >::type
        IfTableCopyAxesAndTags(const XArrayType1* in, XArrayType2* out)
        {
            //if it inherits from a table container, execute this specialization
            //copy the axes, their tags and labels
            //Note: this will fail if the axis types are not the same! --> type casting is for table elements only
            *( out->GetAxisPack() ) = *( in->GetAxisPack() );
            //copy the tags
            out->CopyTags(*in);
        }



};

}

#endif /*! end of include guard: MHO_ElementTypeCaster */
