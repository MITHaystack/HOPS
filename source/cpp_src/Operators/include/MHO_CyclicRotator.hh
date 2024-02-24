#ifndef MHO_CyclicRotator_HH__
#define MHO_CyclicRotator_HH__



#include <algorithm>
#include <cstdint>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryOperator.hh"
#include "MHO_TableContainer.hh"

namespace hops
{

/*!
*@file MHO_CyclicRotator.hh
*@class MHO_CyclicRotator
*@author J. Barrett - barrettj@mit.edu
*@date
*@brief
* Applies a cyclic rotation on the contents on a multidimensional array
* by some specified offset for each dimension.
*/


template< class XArrayType >
class MHO_CyclicRotator: public MHO_UnaryOperator< XArrayType >
{
    public:

        MHO_CyclicRotator()
        {
            fInitialized = false;
            for(std::size_t i=0; i<XArrayType::rank::value; i++)
            {
                fDimensionSize[i] = 0;
                fOffsets[i]=0;
            }
        };

        virtual ~MHO_CyclicRotator(){};

        //set the offset for the cyclic rotation in each dimension (default is zero...do nothing)
        void SetOffset(std::size_t dimension_index, int64_t offset_value)
        {
            //A negative offset_value results in a "right" rotation: rot by  1: [0 1 2 3] -> [3 0 1 2]
            //A positive offset_value results in a "left" rotation: rot by -1: [0 1 2 3] -> [1 2 3 0]
            if(dimension_index < XArrayType::rank::value)
            {
                fOffsets[dimension_index] = offset_value;
            }
            else
            {
                msg_error("operators", "error, rotation offset for dimension: "<<dimension_index<<", exceeds array rank." << eom);
            }
            fInitialized = false;
        }


    protected:


        virtual bool InitializeInPlace(XArrayType* in) override
        {
            if(in != nullptr )
            {
                in->GetDimensions(fDimensionSize);
                fInitialized = true;
            }
            return fInitialized;
        }

        virtual bool ExecuteInPlace(XArrayType* in) override
        {
            if(!fInitialized)
            {
                return false;
            }
            else
            {
                for(std::size_t i=0; i<XArrayType::rank::value; i++)
                {
                    if(fOffsets[i] != 0)
                    {
                        fModuloOffsets[i] = positive_modulo(fOffsets[i], fDimensionSize[i]);
                    }
                    else{fModuloOffsets[i] = 0;}
                    IfTableRotateAxis(in, in, i);
                }

                size_t index[XArrayType::rank::value];
                size_t non_active_dimension_size[XArrayType::rank::value-1];
                size_t non_active_dimension_value[XArrayType::rank::value-1];
                size_t non_active_dimension_index[XArrayType::rank::value-1];

                //select the dimension on which to perform the rotation
                for(size_t d = 0; d < XArrayType::rank::value; d++)
                {
                    if(fOffsets[d] !=0)
                    {
                        //now we loop over all dimensions not specified by d
                        //first compute the number of arrays we need to rotate
                        size_t n_rot = 1;
                        size_t count = 0;
                        size_t stride = in->GetStride(d);
                        for(size_t i = 0; i < XArrayType::rank::value; i++)
                        {
                            if(i != d)
                            {
                                n_rot *= fDimensionSize[i];
                                non_active_dimension_index[count] = i;
                                non_active_dimension_size[count] = fDimensionSize[i];
                                count++;
                            }
                        }

                        //loop over the number of rotations to perform
                        for(size_t n=0; n<n_rot; n++)
                        {
                            //invert place in list to obtain indices of block in array
                            MHO_NDArrayMath::RowMajorIndexFromOffset<XArrayType::rank::value-1>(n, non_active_dimension_size, non_active_dimension_value);

                            //copy the value of the non-active dimensions in to index
                            for(size_t i=0; i<XArrayType::rank::value-1; i++)
                            {
                                index[ non_active_dimension_index[i] ] = non_active_dimension_value[i];
                            }

                            //locate the start of this row
                            size_t data_location;
                            index[d] = 0;
                            data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<XArrayType::rank::value>(fDimensionSize, index);

                            //now rotate the array by the specified amount
                            auto it_first = in->stride_iterator_at(data_location, stride);
                            auto it_nfirst = it_first + fModuloOffsets[d];
                            auto it_end = it_first + fDimensionSize[d];
                            std::rotate(it_first, it_nfirst, it_end);
                        }
                    }
                }
                return true;
            }

        }

        virtual bool InitializeOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            if(in != nullptr && out != nullptr)
            {
                in->GetDimensions(fDimensionSize);
                //only need to change output size if in != out and size is different
                std::size_t in_dim[XArrayType::rank::value];
                std::size_t out_dim[XArrayType::rank::value];
                in->GetDimensions(in_dim);
                out->GetDimensions(out_dim);

                bool have_to_resize = false;
                for(std::size_t i=0; i<XArrayType::rank::value; i++)
                {
                    if(out_dim[i] != in_dim[i]){have_to_resize = true; break;}
                }

                if(have_to_resize){out->Resize(in_dim);}
                fInitialized = true;
            }
            else{ fInitialized = false; }
            return fInitialized;
        }

        virtual bool ExecuteOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            if(fInitialized)
            {
                for(std::size_t i=0; i<XArrayType::rank::value; i++)
                {
                    if(fOffsets[i] != 0)
                    {
                        fModuloOffsets[i] = positive_modulo(fOffsets[i], fDimensionSize[i]);
                    }
                    IfTableRotateAxis(in, out, i);
                }
                //first get the indices of the input iterator
                auto in_iter =  in->cbegin();
                auto in_iter_end = in->cend();
                const std::size_t* out_dim = out->GetDimensions();
                const std::size_t* in_dim = in->GetDimensions();
                std::array<std::size_t, XArrayType::rank::value > in_loc;
                while( in_iter != in_iter_end)
                {
                    MHO_NDArrayMath::RowMajorIndexFromOffset< XArrayType::rank::value >(in_iter.GetOffset(), in_dim, &(in_loc[0]) );
                    for(std::size_t i=0; i<XArrayType::rank::value;i++)
                    {
                        fWorkspace[i] = positive_modulo( in_loc[i] - fModuloOffsets[i], out_dim[i]);
                    }
                    std::size_t out_loc = out->GetOffsetForIndices(fWorkspace);
                    (*(out))[out_loc] = *in_iter;
                    in_iter++;
                }
                return true;
            }
            return false;
        }

    private:


        //default...does nothing
        template< typename XCheckType = XArrayType >
        typename std::enable_if< !std::is_base_of<MHO_TableContainerBase, XCheckType>::value, void >::type
        IfTableRotateAxis(const XArrayType* /*!in*/, XArrayType* /*!out*/, std::size_t /*!dim*/){};

        //use SFINAE to generate specialization for MHO_TableContainer types
        template< typename XCheckType = XArrayType >
        typename std::enable_if< std::is_base_of<MHO_TableContainerBase, XCheckType>::value, void >::type
        IfTableRotateAxis(const XArrayType* in, XArrayType* out, std::size_t dim)
        {
            RotateAxis axis_rot(fOffsets[dim]);
            apply_at2< typename XArrayType::axis_pack_tuple_type, RotateAxis >( *in, *out, dim, axis_rot);
        }

        class RotateAxis
        {
            public:
                RotateAxis(std::size_t offset):
                    fOffset(offset)
                {};
                ~RotateAxis(){};

                template< typename XAxisType >
                void operator()(const XAxisType& axis1, XAxisType& axis2)
                {
                    if(fOffset != 0)
                    {
                        XAxisType tmp;
                        tmp.Copy(axis1);
                        axis2.Copy(axis1); //copy the axis first to get tags, etc.
                        //now rotate the elements by the offset
                        std::size_t a, b;
                        int64_t i, j;
                        int64_t n = axis2.GetSize();
                        for(i=0; i<n; i++)
                        {
                            j = ( (i-fOffset) % n + n) % n;
                            a = i;
                            b = j;
                            axis2(a) = tmp(b);
                        }
                    }
                    else{ axis2.Copy(axis1); }
                }

            private:
                int64_t fOffset;
        };



        //note: using the modulo is a painfully slow to do this
        //TODO FIXME...we ought to check for uint64_t -> int64_t overflows!
        inline int64_t positive_modulo(int64_t i, int64_t n)
        {
            return (i % n + n) % n;
        }

        //offsets to for cyclic rotation
        bool fInitialized;
        std::size_t  fDimensionSize[XArrayType::rank::value];
        int64_t fOffsets[XArrayType::rank::value];
        int64_t fModuloOffsets[XArrayType::rank::value];
        std::size_t fWorkspace[XArrayType::rank::value];
};

};




#endif /*! MHO_CyclicRotator_H__ */
