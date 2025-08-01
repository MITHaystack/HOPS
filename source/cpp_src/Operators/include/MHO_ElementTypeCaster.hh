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

/**
 * @brief Class MHO_ElementTypeCaster
 */
template< class XArgType1, class XArgType2 > //args implicitly assumed to inherit from MHO_NDArrayWrapper
class MHO_ElementTypeCaster: public MHO_TransformingOperator< XArgType1, XArgType2 >
{
    public:
        MHO_ElementTypeCaster(){};
        virtual ~MHO_ElementTypeCaster(){};

    protected:
        /**
         * @brief Initializes implementation using input and output arguments.
         *
         * @param !in Pointer to constant input argument of type XArgType1
         * @param !out Pointer to output argument of type XArgType2
         * @return Boolean indicating success or failure of initialization
         * @note This is a virtual function.
         */
        virtual bool InitializeImpl(const XArgType1* /*!in*/, XArgType2* /*!out*/) override { return true; }; //no op

        /**
         * @brief Copies input array to output array and resizes it if necessary.
         *
         * @param in Input array to be copied.
         * @param out (XArgType2*)
         * @return True if copying was successful, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool ExecuteImpl(const XArgType1* in, XArgType2* out) override
        {
            if(in != nullptr && out != nullptr)
            {
                //resize the output array
                out->Resize(in->GetDimensions());

                //copy the contents
                std::size_t total_size = in->GetSize();
                for(std::size_t i = 0; i < total_size; i++)
                {
                    (*out)[i] = static_cast< typename XArgType2::value_type >((*in)[i]);
                }

                IfTableCopyAxesAndTags(in, out);
                return true;
            }
            return false;
        }

    private:
        //default...does nothing
        /**
         * @brief use SFINAE to generate specialization for MHO_TableContainer types
         *
         * @tparam XArrayType1 Template parameter XArrayType1
         * @tparam XArrayType2 Template parameter XArrayType2
         * @param !in Parameter description
         * @param !out Parameter description
         */
        template< class XArrayType1, class XArrayType2 >
        typename std::enable_if< !(std::is_base_of< MHO_TableContainerBase, XArrayType1 >::value &&
                                   std::is_base_of< MHO_TableContainerBase, XArrayType2 >::value),
                                 void >::type
        IfTableCopyAxesAndTags(const XArrayType1* /*!in*/, XArrayType2* /*!out*/){};

        /**
         * @brief Copies axes and tags from input XArrayType1 to output XArrayType2 if they inherit from MHO_TableContainer.
         *
         * @tparam XArrayType1 Template parameter XArrayType1
         * @tparam XArrayType2 Template parameter XArrayType2
         * @param in Input array of type XArrayType1
         * @param out Output array of type XArrayType2
         */
        template< class XArrayType1, class XArrayType2 >
        typename std::enable_if< std::is_base_of< MHO_TableContainerBase, XArrayType1 >::value &&
                                     std::is_base_of< MHO_TableContainerBase, XArrayType2 >::value,
                                 void >::type
        IfTableCopyAxesAndTags(const XArrayType1* in, XArrayType2* out)
        {
            //if it inherits from a table container, execute this specialization
            //copy the axes, their tags and labels
            //Note: this will fail if the axis types are not the same! --> type casting is for table elements only
            *(out->GetAxisPack()) = *(in->GetAxisPack());
            //copy the tags
            out->CopyTags(*in);
        }
};

} // namespace hops

#endif /*! end of include guard: MHO_ElementTypeCaster */
