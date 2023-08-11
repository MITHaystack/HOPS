#ifndef MHO_EndZeroPadder_HH__
#define MHO_EndZeroPadder_HH__

#include <cstring>
#include <vector>
#include <bitset>
#include <set>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryOperator.hh"
#include "MHO_TableContainer.hh"

namespace hops
{

template< typename XArgType >
class MHO_EndZeroPadder:
    public MHO_UnaryOperator< XArgType >
{
    public:

        MHO_EndZeroPadder()
        {
            fPaddingFactor = 1;
            fPaddedSize = 0;
            for(std::size_t i=0; i<XArgType::rank::value; i++)
            {
                fInputDimensionSize[i] = 0;
                fOutputDimensionSize[i] = 0;
                fAxesToXForm[i] = true;
            }

            fIsValid = false;
            fFlipped = false; //chooses which end we pad
            fInitialized = false;
        };

        virtual ~MHO_EndZeroPadder(){};

        //factor M by which the new array will be extended (original array, length N, new array length NM)
        virtual void SetPaddingFactor(std::size_t factor){fPaddingFactor = factor;};

        //instead of a multiplicative factor, the original array, length N is padded out ot the new specified length M
        virtual void SetPaddedSize(std::size_t new_size){fPaddedSize = new_size; fPaddingFactor = 1;}

        virtual void SetEndPadded(){fFlipped = false;}; //zero padding from end of signal out to end of the array
        virtual void SetReverseEndPadded(){fFlipped = true;}; //place signal at end of array and zero pad out to start

        //sometimes we may want to select/deselect particular dimensions of the x-form
        //default is to transform along every dimension, but that may not always be needed
        void SelectAllAxes(){for(std::size_t i=0; i<XArgType::rank::value; i++){fAxesToXForm[i] = true;}}
        void DeselectAllAxes(){for(std::size_t i=0; i<XArgType::rank::value; i++){fAxesToXForm[i] = false;}}
        void SelectAxis(std::size_t axis_index)
        {
            if(axis_index < XArgType::rank::value){fAxesToXForm[axis_index] = true;}
            else
            {
                msg_error("operators", "Cannot transform axis with index: " <<
                          axis_index << "for array with rank: " << XArgType::rank::value << eom);
            }
        }

    protected:


        virtual bool InitializeInPlace(XArgType* in)
        {
            return InitializeOutOfPlace(in, &fTmpWorkspace);
        }

        virtual bool ExecuteInPlace(XArgType* in)
        {
            bool status = ExecuteOutOfPlace(in, &fTmpWorkspace);
            //"in-place" execution requires a copy from the workspace back to the object we are modifying
            in->Copy(fTmpWorkspace);
            return status;
        }


        virtual bool InitializeOutOfPlace(const XArgType* in, XArgType* out)
        {
            if(in != nullptr && out != nullptr && in != out)
            {
                fIsValid = true;
            }

            if( fIsValid )
            {
                //output dimensions must be factor of fPaddingFactor bigger than input dims
                in->GetDimensions(fInputDimensionSize);
                out->GetDimensions(fOutputDimensionSize);
                ConditionallyResizeOutput(in->GetDimensionArray(), out);
                fInitialized = true;
            }
            return (fInitialized && fIsValid);
        }

        virtual bool ExecuteOutOfPlace(const XArgType* in, XArgType* out)
        {
            if(fIsValid && fInitialized)
            {
                //zero out the output array
                out->ZeroArray();
                if(!fFlipped)
                {
                    //zero padding is placed at the end of the array
                    auto in_iter =  in->cbegin();
                    auto in_iter_end = in->cend();
                    std::array<std::size_t, XArgType::rank::value> in_indices;
                    while( in_iter != in_iter_end)
                    {
                        //copy the input data to the same 'index' location in the output array

                        //get the input indices for each dimension
                        MHO_NDArrayMath::RowMajorIndexFromOffset<XArgType::rank::value>(in_iter.GetOffset(), fInputDimensionSize, &(in_indices[0]) );
                        std::size_t out_loc = MHO_NDArrayMath::OffsetFromRowMajorIndex<XArgType::rank::value>(fOutputDimensionSize, &(in_indices[0]) );
                        (*(out))[out_loc] = *in_iter;
                        ++in_iter;
                    }
                }
                else
                {
                    //zero padding is placed at the start of the array
                    auto in_iter =  in->cbegin();
                    auto in_iter_end = in->cend();
                    std::array<std::size_t, XArgType::rank::value> in_index;
                    std::size_t out_index[XArgType::rank::value];
                    while( in_iter != in_iter_end)
                    {
                        //copy the input data to the flipped location in the output array
                        MHO_NDArrayMath::RowMajorIndexFromOffset<XArgType::rank::value>(in_iter.GetOffset(), in->GetDimensions(), &(in_index[0]) );
                        for(std::size_t i=0; i<XArgType::rank::value; i++)
                        {
                            if(fAxesToXForm[i])
                            {
                                //The way were are setting the indexes here
                                //(with n=0 mapping to N/2 is done for compatibility with norm_fx)
                                //TODO FIXME...figure out why it is done this way
                                if(in_index[i] != 0)
                                {
                                    out_index[i] = (fOutputDimensionSize[i]) - in_index[i];
                                }
                                else{out_index[i] = fOutputDimensionSize[i]/2;}
                            }
                            else{out_index[i] = in_index[i];}
                        }
                        std::size_t out_loc = MHO_NDArrayMath::OffsetFromRowMajorIndex<XArgType::rank::value>(fOutputDimensionSize, out_index);
                        (*(out))[out_loc] = *in_iter;
                        ++in_iter;
                    }
                }

                //IfTableTransformAxis(in, out, d);
                //successful
                return true;
            }
            else
            {
                //error
                msg_error("operators", "Array dimensions are not valid or intialization failed. Aborting zero padding." << eom);
                return false;
            }
        }

    private:

        void ConditionallyResizeOutput(const std::array<std::size_t, XArgType::rank::value>& dims, XArgType* out)
        {
            auto out_dim = out->GetDimensionArray();
            bool have_to_resize = false;
            for(std::size_t i=0; i<XArgType::rank::value; i++)
            {
                if(fAxesToXForm[i])
                {
                    if(dims[i]*fPaddingFactor != out_dim[i])
                    {
                        have_to_resize = true;
                        out_dim[i] = dims[i]*fPaddingFactor;
                    }
                }
                else
                {
                    if(dims[i] != out_dim[i])
                    {
                        have_to_resize = true;
                        out_dim[i] = dims[i];
                    }
                }
            }
            if(have_to_resize){ out->Resize( &(out_dim[0]) );}
            out->GetDimensions(fOutputDimensionSize);
        }

        bool fIsValid;
        bool fInitialized;
        bool fFlipped;

        std::size_t fPaddingFactor;
        std::size_t fPaddedSize;
        std::size_t fInputDimensionSize[XArgType::rank::value];
        std::size_t fOutputDimensionSize[XArgType::rank::value];
        bool fAxesToXForm[XArgType::rank::value];

        XArgType fTmpWorkspace;
};


}

#endif /* MHO_EndZeroPadder_H__ */
