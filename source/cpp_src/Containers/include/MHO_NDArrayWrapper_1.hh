#ifndef MHO_NDArrayWrapper_1_HH__
#define MHO_NDArrayWrapper_1_HH__

//this include file should not be used directly
#ifndef MHO_NDArrayWrapper_HH__
#error "Do not include MHO_NDArrayWrapper_1.hh directly; use MHO_NDArrayWrapper.hh instead."
#endif

namespace hops
{

template< typename XValueType >
class MHO_NDArrayWrapper<XValueType, 1>:
    public MHO_ExtensibleElement
{
    public:

        using value_type = XValueType;
        using index_type = std::array<std::size_t, 1>;
        typedef std::integral_constant< std::size_t, 1 > rank;

        //constructors
        MHO_NDArrayWrapper(){Construct(nullptr, nullptr);}; //empty constructor, to be configured later
        MHO_NDArrayWrapper(const std::size_t* dim){Construct(nullptr, dim);}; //data is internally allocated
        MHO_NDArrayWrapper(XValueType* ptr, const std::size_t* dim){Construct(ptr,dim);}; //data is externally allocated/managed
        MHO_NDArrayWrapper(std::size_t dim){Construct(nullptr, &dim);}; //data is internally allocated
        MHO_NDArrayWrapper(XValueType* ptr, std::size_t dim){Construct(ptr,&dim);}; //data is externally allocated/managed

        MHO_NDArrayWrapper(const MHO_NDArrayWrapper& obj)
        {
            if(obj.fExternallyManaged){Construct(obj.fDataPtr, &(obj.fDims[0]) );}
            else
            {
                Construct(nullptr, &(obj.fDims[0]) );
                if(fSize != 0){std::copy(obj.fData.begin(), obj.fData.end(), fData.begin() );}
            }
        }

        //destructor
        virtual ~MHO_NDArrayWrapper(){};

        //resize functions
        virtual void Resize(const std::size_t* dim)
        {
            if(fExternallyManaged)
            {
                msg_warn("containers", "Resize operation called on a wrapper pointing to " <<
                          "an exernally managed array will replace it with internally " <<
                          "managed memory. This may result in unexpected behavior." << eom);
            }
            Construct(nullptr, dim);
        }

        void Resize(std::size_t dim){ Resize(&dim); }

        //set pointer to externally managed array with associated dimension
        void SetExternalData(XValueType* ptr, const std::size_t* dim){Construct(ptr, dim);}
        void SetExternalData(XValueType* ptr, std::size_t dim){Construct(ptr, &dim);}

        //access to underlying raw array pointer
        XValueType* GetData(){return fDataPtr;};
        const XValueType* GetData() const {return fDataPtr;};

        //get the total size of the array
        std::size_t GetRank() const {return 1;}
        std::size_t GetSize() const {return fSize;};

        //get the dimensions/shape of the array
        const std::size_t* GetDimensions() const {return &(fDims[0]);}
        void GetDimensions(std::size_t* dim) const { dim[0] = fDims[0]; }
        index_type GetDimensionArray() const {return fDims;}
        std::size_t GetDimension(std::size_t idx) const {return fDims[idx];}

        //get element strides
        const std::size_t* GetStrides() const {return &(fStrides[0]);}
        void GetStrides(std::size_t* strd) const { strd[0] = fStrides[0];}
        index_type GetStrideArray() const {return fStrides;}
        std::size_t GetStride(std::size_t idx) const {return fStrides[0];}

        //access operators

        //access operator () -- no bounds checking
        XValueType& operator()(std::size_t idx) {return fDataPtr[idx];}
        const XValueType& operator()(std::size_t idx) const {return fDataPtr[idx];}

        //access via at() -- same as operator() but with bounds checking
        XValueType& at(std::size_t idx)
        {
            if( idx < fSize ){ return fDataPtr[idx]; }
            else{ throw std::out_of_range("MHO_NDArrayWrapper::at() indices out of range.");}
        }

        //const at()
        const XValueType& at(std::size_t idx) const
        {
            if( idx < fSize ){ return fDataPtr[idx]; }
            else{ throw std::out_of_range("MHO_NDArrayWrapper::at() indices out of range.");}
        }

        //in 1-d case, operator[] is same as operator()
        XValueType& operator[](std::size_t i){return fDataPtr[i];}
        const XValueType& operator[](std::size_t i) const {return fDataPtr[i];}

        //assignment operator
        MHO_NDArrayWrapper& operator=(const MHO_NDArrayWrapper& rhs)
        {
            if(this != &rhs)
            {
                if(rhs.fExternallyManaged)
                {
                    //effectively de-allocate anything we might have had before
                    std::vector< XValueType >().swap(fData);
                    Construct(rhs.fDataPtr, &(rhs.fDims[0]));
                }
                else
                {
                    Construct(nullptr,  &(rhs.fDims[0]));
                    if(fSize != 0){std::copy(rhs.fData.begin(), rhs.fData.end(), this->fData.begin() );}
                }
            }
            return *this;
        }

        //convenience functions
        void SetArray(const XValueType& obj)
        {
            for(std::size_t i=0; i < fSize; i++){fDataPtr[i] = obj; } 
        }
        void ZeroArray()
        {
            std::memset(fDataPtr, 0, fSize*sizeof(XValueType) );
        };

        //expensive copy (as opposed to the assignment operator,
        //pointers to exernally managed memory are not transfer)
        virtual void Copy(const MHO_NDArrayWrapper& rhs)
        {
            if(this != &rhs)
            {
                Construct(nullptr,  &(rhs.fDims[0]));
                if(fSize != 0){std::copy(rhs.fData.begin(), rhs.fData.end(), this->fData.begin() );}
            }
        }


        //linear offset into the array -- no real utility in 1-d case
        std::size_t GetOffsetForIndices(const std::size_t* index){return index[0];}

        //here mainly so table containers with rank 1 still work, in this case a sub-view just gets you a scalar 
        template <typename ...XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) < 1), MHO_NDArrayWrapper<XValueType, 1 - ( sizeof...(XIndexTypeS) ) > >::type
        SubView(XIndexTypeS...idx)
        {
            std::array<std::size_t, sizeof...(XIndexTypeS) > leading_idx = {{static_cast<size_t>(idx)...}};
            for(std::size_t i=0; i<1; i++){fTmp[i] = 0;}
            for(std::size_t i=0; i<leading_idx.size(); i++){fTmp[i] = leading_idx[i];}
            std::size_t offset = MHO_NDArrayMath::OffsetFromRowMajorIndex<1>(&(fDims[0]), &(fTmp[0]));
            std::array<std::size_t, 1 - (sizeof...(XIndexTypeS)) > dim;
            for(std::size_t i=0; i<dim.size(); i++){dim[i] = fDims[i + (sizeof...(XIndexTypeS) )];}
            return MHO_NDArrayWrapper<XValueType, 1 - ( sizeof...(XIndexTypeS) ) >(&(fDataPtr[offset]) , &(dim[0]) );
        }

        //this function is mainly here to allow for 1-d table containers, there's not much utility 
        //of a 'slice view' of a 1-d array (you just get the same array back...)
        MHO_NDArrayView< XValueType, 1>
        SliceView(const char* /* unused_arg */) 
        {
            //just return a 1d array view of this 1-d array
            return  MHO_NDArrayView<XValueType, 1>(&(fDataPtr[0]), &(fDims[0]), &(fStrides[0]) );
        }

        XValueType& ValueAt(const index_type& idx)
        {
            return fDataPtr[idx];
        }

        const XValueType& ValueAt(const index_type& idx) const
        {
            return fDataPtr[idx];
        }

    protected:

        XValueType* fDataPtr;
        bool fExternallyManaged;
        std::vector< XValueType > fData; //used for internally managed data
        index_type fDims; //size of each dimension
        index_type fStrides; //strides between elements in each dimension
        mutable index_type fTmp; //temp index workspace
        std::size_t fSize; //total size of array

    private:

        void Construct(XValueType* ptr, const std::size_t* dim)
        {
            //default construction (empty)
            fDims[0] = 0; fStrides[0] = 0;
            fSize = 0;
            fDataPtr = nullptr;
            fExternallyManaged = false;
            if(ptr == nullptr && dim == nullptr){return;}

            //dimensions known
            if(dim != nullptr){fDims[0] = dim[0];}
            fSize = fDims[0];
            fStrides[0] = 1;

            if(ptr != nullptr) //using externally managed memory
            {
                fDataPtr = ptr;
                fExternallyManaged = true;
            }
            else //use internally managed memory
            {
                fData.resize(fSize);
                //this concept does not work with std::vector<bool>, as 
                //boolean vectors use packed bitsets, while bool variables are 
                //the size of a char, should write a specialization for boolean
                //containers, but at the moment they aren't needed
                fDataPtr = &(fData[0]);
                fExternallyManaged = false;
            }
        }

    //the iterator definitions //////////////////////////////////////////////////
    public:

        using iterator = MHO_BidirectionalIterator<XValueType>;
        using stride_iterator = MHO_BidirectionalStrideIterator<XValueType>;

        using const_iterator = MHO_BidirectionalIterator<XValueType>;
        using const_stride_iterator = MHO_BidirectionalStrideIterator<XValueType>;

        iterator begin(){ return iterator(fDataPtr, fDataPtr, fSize);}
        iterator end(){ return iterator(fDataPtr, fDataPtr + fSize, fSize);}
        iterator iterator_at(std::size_t offset){return iterator(fDataPtr, fDataPtr + std::min(offset, fSize), fSize);}

        const_iterator cbegin() const{ return const_iterator(fDataPtr, fDataPtr, fSize);}
        const_iterator cend() const { return const_iterator(fDataPtr, fDataPtr + fSize, fSize);}
        const_iterator citerator_at(std::size_t offset) const {return const_iterator(fDataPtr, fDataPtr + std::min(offset, fSize), fSize);}

        stride_iterator stride_begin(std::size_t stride){ return stride_iterator(fDataPtr, fDataPtr, fSize, stride);}
        stride_iterator stride_end(std::size_t stride){ return stride_iterator(fDataPtr, fDataPtr + fSize, fSize, stride);}
        stride_iterator stride_iterator_at(std::size_t offset, std::size_t stride)
        {
            return stride_iterator(fDataPtr, fDataPtr + std::min(offset, fSize), fSize, stride);
        }

        const_stride_iterator cstride_begin(std::size_t stride) const { return stride_iterator(fDataPtr, fDataPtr, fSize, stride);}
        const_stride_iterator cstride_end(std::size_t stride) const { return stride_iterator(fDataPtr, fDataPtr + fSize, fSize, stride);}
        const_stride_iterator cstride_iterator_at(std::size_t offset, std::size_t stride) const
        {
            return stride_iterator(fDataPtr, fDataPtr + std::min(offset, fSize), fSize, stride);
        }
};


}//end of namespace

#endif /* end of include guard: MHO_NDArrayWrapper_1 */
