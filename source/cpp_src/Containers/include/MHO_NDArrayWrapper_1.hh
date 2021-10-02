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
            if(obj.fExternallyManaged){Construct(obj.fDataPtr,obj.fDims);}
            else
            {
                Construct(nullptr,obj.fDims);
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
                    Construct(rhs.fDataPtr, rhs.fDims);
                }
                else
                {
                    Construct(nullptr, rhs.fDims);
                    if(fSize != 0){std::copy(rhs.fData.begin(), rhs.fData.end(), rhs.begin() );}
                }
            }
            return *this;
        }

        //convenience functions
        void SetArray(const XValueType& obj){ for(std::size_t i=0; i < fSize; i++){fDataPtr[i] = obj; } }
        void ZeroArray(){ std::memset(fDataPtr, 0, fSize*sizeof(XValueType) ); }; //set all elements in the array to zero

        //linear offset into the array -- no real utility in 1-d case
        std::size_t GetOffsetForIndices(const std::size_t* index){return index[0];}

    protected:

        XValueType* fDataPtr;
        bool fExternallyManaged;
        std::vector< XValueType > fData; //used for internally managed data
        index_type fDims; //size of each dimension
        index_type fStrides; //strides between elements in each dimension
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
            ComputeStrides();

            if(ptr != nullptr) //using externally managed memory
            {
                fDataPtr = ptr;
                fExternallyManaged = true;
            }
            else //use internally managed memory
            {
                fData.resize(fSize);
                fDataPtr = &(fData[0]);
                fExternallyManaged = false;
            }
        }

        void ComputeStrides(){fStrides[0] = 1;}

    //the iterator definitions //////////////////////////////////////////////////
    public:

        using iterator = MHO_BidirectionalIterator<XValueType>;
        using stride_iterator = MHO_BidirectionalStrideIterator<XValueType>;

        iterator begin(){ return iterator(fDataPtr, fDataPtr, fSize);}
        iterator end(){ return iterator(fDataPtr, fDataPtr + fSize, fSize);}
        iterator iterator_at(std::size_t offset){return iterator(fDataPtr, fDataPtr + std::min(offset, fSize), fSize);}

        stride_iterator stride_begin(std::size_t stride){ return stride_iterator(fDataPtr, fDataPtr, fSize, stride);}
        stride_iterator stride_end(std::size_t stride){ return stride_iterator(fDataPtr, fDataPtr + fSize, fSize, stride);}
        stride_iterator stride_iterator_at(std::size_t offset, std::size_t stride)
        {
            return stride_iterator(fDataPtr, fDataPtr + std::min(offset, fSize), fSize, stride);
        }

};


}//end of namespace

#endif /* end of include guard: MHO_NDArrayWrapper_1 */
