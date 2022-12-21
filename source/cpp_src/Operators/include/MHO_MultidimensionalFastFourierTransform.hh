#ifndef MHO_MultidimensionalFastFourierTransform_HH__
#define MHO_MultidimensionalFastFourierTransform_HH__

#include <cstring>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryOperator.hh"
#include "MHO_FastFourierTransform.hh"

#include "MHO_TableContainer.hh"

namespace hops
{

template< typename XFloatType, size_t RANK>
class MHO_MultidimensionalFastFourierTransform:
    public MHO_UnaryOperator< MHO_NDArrayWrapper< std::complex<XFloatType>, RANK > >
{
    public:

        using XArrayType = MHO_NDArrayWrapper< std::complex< XFloatType>, RANK >;

        MHO_MultidimensionalFastFourierTransform()
        {
            for(size_t i=0; i<RANK; i++)
            {
                fDimensionSize[i] = 0;
                fAxesToXForm[i] = true;
                fWorkspaceWrapper[i] = NULL;
                fTransformCalculator[i] = NULL;
            }

            fIsValid = false;
            fInitialized = false;
            fForward = true;
        };

        virtual ~MHO_MultidimensionalFastFourierTransform()
        {
            DealocateWorkspace();
        };

        virtual void SetForward(){fForward = true;}
        virtual void SetBackward(){fForward = false;};

        //sometimes we may want to select/deselect particular dimensions of the x-form
        //default is to transform along every dimension, but that may not always be needed
        void SelectAllAxes(){for(std::size_t i=0; i<RANK; i++){fAxesToXForm[i] = true;}}
        void DeselectAllAxes(){for(std::size_t i=0; i<RANK; i++){fAxesToXForm[i] = false;}}
        void SelectAxis(std::size_t axis_index)
        {
            if(axis_index < RANK){fAxesToXForm[axis_index] = true;}
            else
            {
                msg_error("operators", "Cannot transform axis with index: " <<
                          axis_index << "for array with rank: " << RANK << eom);
            }
        }

    protected:

        virtual bool InitializeInPlace(XArrayType* in) override
        {
            if( in != nullptr ){fIsValid = true;}
            else{fIsValid = false;}

            if(fIsValid)
            {
                //check if the current transform sizes are the same as input
                bool need_to_resize = false;
                for(std::size_t i=0; i<RANK; i++)
                {
                    if(fDimensionSize[i] != in->GetDimension(i)){need_to_resize = true; break;}
                }
                if(need_to_resize)
                {
                    in->GetDimensions(fDimensionSize);
                    DealocateWorkspace();
                    AllocateWorkspace();
                }
                fInitialized = true;
            }
            return (fInitialized && fIsValid);
        }

        virtual bool InitializeOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            if( in != nullptr && out != nullptr ){fIsValid = true;}
            else{fIsValid = false;}

            if(fIsValid)
            {
                //check if the arrays have the same dimensions
                if( !HaveSameDimensions(in, out) )
                {
                    //resize the output array to match input
                    out->Resize( in->GetDimensions() );
                }
                //check if the current transform sizes are the same as input
                bool need_to_resize = false;
                for(std::size_t i=0; i<RANK; i++)
                {
                    if(fDimensionSize[i] != in->GetDimension(i)){need_to_resize = true; break;}
                }
                if(need_to_resize)
                {
                    in->GetDimensions(fDimensionSize);
                    DealocateWorkspace();
                    AllocateWorkspace();
                }
                fInitialized = true;
            }
            return (fInitialized && fIsValid);
        }



        virtual bool ExecuteInPlace(XArrayType* in) override
        {
            if(fIsValid && fInitialized)
            {
                size_t total_size = 1;
                for(size_t i=0; i<RANK; i++)
                {
                    total_size *= fDimensionSize[i];
                    if(fForward)
                    {
                        fTransformCalculator[i]->SetForward();
                    }
                    else
                    {
                        fTransformCalculator[i]->SetBackward();
                    }
                }

                size_t index[RANK];
                size_t non_active_dimension_size[RANK-1];
                size_t non_active_dimension_value[RANK-1];
                size_t non_active_dimension_index[RANK-1];

                //select the dimension on which to perform the FFT
                for(size_t d = 0; d < RANK; d++)
                {
                    if(fAxesToXForm[d])
                    {
                        //now we loop over all dimensions not specified by d
                        //first compute the number of FFTs to perform
                        size_t n_fft = 1;
                        size_t count = 0;
                        for(size_t i = 0; i < RANK; i++)
                        {
                            if(i != d)
                            {
                                n_fft *= fDimensionSize[i];
                                non_active_dimension_index[count] = i;
                                non_active_dimension_size[count] = fDimensionSize[i];
                                count++;
                            }
                        }

                        //loop over the number of FFTs to perform
                        for(size_t n=0; n<n_fft; n++)
                        {
                            //invert place in list to obtain indices of block in array
                            MHO_NDArrayMath::RowMajorIndexFromOffset<RANK-1>(n, non_active_dimension_size, non_active_dimension_value);

                            //copy the value of the non-active dimensions in to index
                            for(size_t i=0; i<RANK-1; i++)
                            {
                                index[ non_active_dimension_index[i] ] = non_active_dimension_value[i];
                            }

                            size_t data_location;
                            //copy the row selected by the other dimensions
                            for(size_t i=0; i<fDimensionSize[d]; i++)
                            {
                                index[d] = i;
                                data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensionSize, index);
                                (*(fWorkspaceWrapper[d]))[i] = (*(in))[data_location];
                            }

                            //compute the FFT of the row selected
                            fTransformCalculator[d]->Execute();

                            //copy the row selected back
                            for(size_t i=0; i<fDimensionSize[d]; i++)
                            {
                                index[d] = i;
                                data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensionSize, index);
                                (*(in))[data_location] = (*(fWorkspaceWrapper[d]))[i];
                            }
                        }

                        std::cout<<"weeeeee "<<d<<std::endl;
                        IfTableTransformAxis(in,d);
                    }
                }
                //successful
                return true;
            }
            else
            {
                //error
                msg_error("math", "FFT input/output array dimensions are not valid or intialization failed. Aborting transform." << eom);
                return false;
            }
        }


        virtual bool ExecuteOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            //if input and output point to the same array, don't bother copying data over
            if(in != out)
            {
                std::memcpy( (void*) out->GetData(),
                             (void*) in->GetData(),
                             (in->GetSize() )*sizeof(std::complex<XFloatType>) );
            }

            // TODO FIXME FOR TABLE CONTAINERS

            return ExecuteInPlace(out);
        }

    private:

        virtual void AllocateWorkspace()
        {
            for(size_t i=0; i<RANK; i++)
            {
                fWorkspaceWrapper[i] = new MHO_NDArrayWrapper< std::complex<XFloatType>, 1 >(fDimensionSize[i]);
                fTransformCalculator[i] = new MHO_FastFourierTransform<XFloatType>();
                fTransformCalculator[i]->SetArgs(fWorkspaceWrapper[i]);
                fTransformCalculator[i]->Initialize();
            }
        }

        virtual void DealocateWorkspace()
        {
            for(size_t i=0; i<RANK; i++)
            {
                delete fWorkspaceWrapper[i]; fWorkspaceWrapper[i] = NULL;
                delete fTransformCalculator[i]; fTransformCalculator[i] = NULL;
            }
        }


        //default...does nothing
        template< typename XCheckType = XArrayType >
        typename std::enable_if< !std::is_base_of<MHO_TableContainerBase, XCheckType>::value, void >::type
        IfTableTransformAxis(XArrayType* /*in*/, std::size_t /*axis_index*/){std::cout<<"DOH!"<<std::endl;};

        //use SFINAE to generate specialization for MHO_TableContainer types
        template< typename XCheckType = XArrayType >
        typename std::enable_if< std::is_base_of<MHO_TableContainerBase, XCheckType>::value, void >::type
        IfTableTransformAxis(XArrayType* in, std::size_t axis_index)
        {
            std::cout<<"Booo: "<<axis_index<<std::endl;
            TransformAxis axis_xformer;
            apply_at< typename XArrayType::axis_pack_tuple_type, TransformAxis >( *in, axis_index, axis_xformer);
        }


        class TransformAxis
        {
            public:
                TransformAxis(){};
                ~TransformAxis(){};

                template< typename XAxisType >
                void operator()(const XAxisType& axis1)
                {
                    //this is under the expectation that all axis labels are equi-spaced 
                    //this should be a safe assumption since we are doing DFT anyway
                    std::size_t N = axis1.GetSize();
                    double length = N;
                    if(N > 1)
                    {
                        double delta = axis1(1) - axis1(0);
                        double spacing = (1.0/delta)*(1.0/length);
                        double start = -1*N/2;
                        for(std::size_t i=0; i<N; i++)
                        {
                            double x = i;
                            double value = (i+start)*spacing;
                            axis1(i) = value;
                        }
                    }
                }

            private:
                std::vector< std::size_t > fSelection;

        };



        //data

        bool fIsValid;
        bool fForward;
        bool fInitialized;

        size_t fDimensionSize[RANK];
        bool fAxesToXForm[RANK];

        MHO_FastFourierTransform<XFloatType>* fTransformCalculator[RANK];
        MHO_NDArrayWrapper<std::complex<XFloatType>, 1>* fWorkspaceWrapper[RANK];


};


}

#endif /* MHO_MultidimensionalFastFourierTransform_H__ */
