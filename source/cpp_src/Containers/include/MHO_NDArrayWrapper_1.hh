#ifndef MHO_NDArrayWrapper_1_HH__
#define MHO_NDArrayWrapper_1_HH__

//this include file should not be used directly
#ifndef MHO_NDArrayWrapper_HH__
    #error "Do not include MHO_NDArrayWrapper_1.hh directly; use MHO_NDArrayWrapper.hh instead."
#endif

namespace hops
{

/**
 * @brief Class MHO_NDArrayWrapper<XValueType, 1> specialization for a RANK-1 (i.e. a vector)
 */
template< typename XValueType > class MHO_NDArrayWrapper< XValueType, 1 >: public MHO_ExtensibleElement
{
    public:
        using value_type = XValueType;
        using index_type = std::array< std::size_t, 1 >;
        typedef std::integral_constant< std::size_t, 1 > rank;

        //constructors
        MHO_NDArrayWrapper() { Construct(nullptr, nullptr); }; //empty constructor, to be configured later

        MHO_NDArrayWrapper(const std::size_t* dim) { Construct(nullptr, dim); }; //data is internally allocated

        MHO_NDArrayWrapper(XValueType* ptr, const std::size_t* dim)
        {
            Construct(ptr, dim);
        }; //data is externally allocated/managed

        MHO_NDArrayWrapper(std::size_t dim) { Construct(nullptr, &dim); }; //data is internally allocated

        MHO_NDArrayWrapper(XValueType* ptr, std::size_t dim) { Construct(ptr, &dim); }; //data is externally allocated/managed

        MHO_NDArrayWrapper(const MHO_NDArrayWrapper& obj)
        {
            Construct(nullptr, &(obj.fDims[0]));
            std::copy(obj.fData.begin(), obj.fData.end(), fData.begin());
        }

        //destructor
        virtual ~MHO_NDArrayWrapper(){};

        //resize functions
        /**
         * @brief Resize an externally managed array using provided dimensions.
         *
         * @param dim Input dimension for resizing the array.
         * @note This is a virtual function.
         */
        virtual void Resize(const std::size_t* dim) { Construct(nullptr, dim); }

        /**
         * @brief Sets dimensions for externally managed array.
         *
         * @param dim Pointer to dimension size_t array
         * @note This is a virtual function.
         */
        void Resize(std::size_t dim) { Resize(&dim); }

        //set pointer to externally managed array with associated dimension
        /**
         * @brief Setter for external data
         *
         * @param ptr Pointer to externally managed XValueType array
         * @param dim Pointer to size_t array representing dimensions of the external data
         */
        void SetExternalData(XValueType* ptr, const std::size_t* dim) { Construct(ptr, dim); }

        /**
         * @brief Setter for external data
         *
         * @param ptr Pointer to externally managed XValueType array
         * @param dim Pointer to size_t array representing dimensions of the array
         */
        void SetExternalData(XValueType* ptr, std::size_t dim) { Construct(ptr, &dim); }

        //access to underlying raw array pointer
        /**
         * @brief Getter for data
         *
         * @return Pointer to XValueType*
         */
        XValueType* GetData() { return fData.data(); };

        /**
         * @brief Getter for data
         *
         * @return Pointer to XValueType*
         */
        const XValueType* GetData() const { return fData.data(); };

        //get the total size of the array
        /**
         * @brief Getter for rank
         *
         * @return Total size of the array as std::size_t
         */
        std::size_t GetRank() const { return 1; }

        /**
         * @brief Getter for size
         *
         * @return Size along the first dimension as std::size_t.
         */
        std::size_t GetSize() const { return fDims[0]; };

        //get the dimensions/shape of the array
        /**
         * @brief Getter for dimensions
         *
         * @return Pointer to an array of std::size_t representing the dimensions
         */
        const std::size_t* GetDimensions() const { return &(fDims[0]); }

        /**
         * @brief Getter for dimensions
         *
         * @return Pointer to std::size_t array
         */
        void GetDimensions(std::size_t* dim) const { dim[0] = fDims[0]; }

        /**
         * @brief Getter for dimension array
         *
         * @return index_type&: Reference to the dimension array
         */
        index_type GetDimensionArray() const { return fDims; }

        std::size_t GetDimension(std::size_t idx) const
        {
            if(idx == 0)
            {
                return fDims[0];
            }
            else
            {
                throw std::out_of_range("MHO_NDArrayWrapper_1::GetDimension() index out of range.");
            }
        }

        //get element strides
        const std::size_t* GetStrides() const { return &(fStrides[0]); }

        void GetStrides(std::size_t* strd) const { strd[0] = fStrides[0]; }

        index_type GetStrideArray() const { return fStrides; }

        std::size_t GetStride(std::size_t idx) const { return fStrides[0]; }

        //access operators

        //access operator () -- no bounds checking
        inline XValueType& operator()(std::size_t idx) { return fData[idx]; }

        inline const XValueType& operator()(std::size_t idx) const { return fData[idx]; }

        //access via at() -- same as operator() but with bounds checking
        inline XValueType& at(std::size_t idx) { return fData.at(idx); }

        inline const XValueType& at(std::size_t idx) const { return fData.at(idx); }

        //in 1-d case, operator[] is same as operator()
        XValueType& operator[](std::size_t i) { return fData[i]; }

        const XValueType& operator[](std::size_t i) const { return fData[i]; }

        //assignment operator
        MHO_NDArrayWrapper& operator=(const MHO_NDArrayWrapper& rhs)
        {
            if(this != &rhs)
            {
                Construct(nullptr, &(rhs.fDims[0]));
                std::copy(rhs.fData.begin(), rhs.fData.end(), this->fData.begin());
            }
            return *this;
        }

        //convenience functions
        void SetArray(const XValueType& obj)
        {
            for(std::size_t i = 0; i < fDims[0]; i++)
            {
                fData[i] = obj;
            }
        }

        void ZeroArray() { std::memset(&(fData[0]), 0, fData.size() * sizeof(XValueType)); };

        //expensive copy (as opposed to the assignment operator,
        //pointers to exernally managed memory are not transferred)
        //but copied instead, inteally managed memory is also copied
        virtual void Copy(const MHO_NDArrayWrapper& rhs)
        {
            if(this != &rhs)
            {
                Construct(nullptr, &(rhs.fDims[0]));
                std::copy(rhs.fData.begin(), rhs.fData.end(), this->fData.begin());
            }
        }

        // //append the contents of the addition to the end of vector container
        // virtual int Append(const MHO_NDArrayWrapper& addition)
        // {
        //     if(this == &addition)
        //     {
        //         msg_error("containers", "cannot append 1d-array to itself." << eom);
        //         return 1;
        //     }
        //
        //         XValueType* &(fData[0]);
        //         bool fExternallyManaged;
        //         std::vector< XValueType > fData; //used for internally managed data
        //         index_type fDims; //size of each dimension
        //         index_type fStrides; //strides between elements in each dimension
        //         mutable index_type fTmp; //temp index workspace
        //         std::size_t fDims[0]; //total size of array
        //
        //     fData.reserve( fDims[0] + addition.fDims[0]);
        //     //in-case our re-allocation has triggered a move, we need to update the &(fData[0])
        //
        //     if(addition.fExternallyManaged)
        //     {
        //         fData.insert(addition.end(), addition.begin(), addition.end());
        //
        //         &(fData[0]) = &(fData[0]);
        //         fDims[0] = fData.size();
        //         fDims[0] = fDims[0];
        //         fStrides = 1;
        //     }
        //
        //     Construct(nullptr,  &(rhs.fDims[0]));
        //     if(fDims[0] != 0){std::copy(rhs.fData.begin(), rhs.fData.end(), this->fData.begin() );}
        //     return 0;
        //
        // }

        //linear offset into the array -- no real utility in 1-d case
        std::size_t GetOffsetForIndices(const std::size_t* index) { return index[0]; }

        index_type GetIndicesForOffset(std::size_t offset)
        {
            index_type val;
            val[0] = offset;
            return val;
        }

        //here mainly so table containers with rank 1 still work, in this case a sub-view just gets you a scalar
        template< typename... XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) < 1),
                                 MHO_NDArrayWrapper< XValueType, 1 - (sizeof...(XIndexTypeS)) > >::type
        SubView(XIndexTypeS... idx)
        {
            std::array< std::size_t, sizeof...(XIndexTypeS) > leading_idx = {{static_cast< size_t >(idx)...}};
            for(std::size_t i = 0; i < 1; i++)
            {
                fTmp[i] = 0;
            }
            for(std::size_t i = 0; i < leading_idx.size(); i++)
            {
                fTmp[i] = leading_idx[i];
            }
            std::size_t offset = MHO_NDArrayMath::OffsetFromRowMajorIndex< 1 >(&(fDims[0]), &(fTmp[0]));
            std::array< std::size_t, 1 - (sizeof...(XIndexTypeS)) > dim;
            for(std::size_t i = 0; i < dim.size(); i++)
            {
                dim[i] = fDims[i + (sizeof...(XIndexTypeS))];
            }
            return MHO_NDArrayWrapper< XValueType, 1 - (sizeof...(XIndexTypeS)) >(VPTR_AT(fData, offset), &(dim[0]));
        }

        //this function is mainly here to allow for 1-d table containers, there's not much utility
        //of a 'slice view' of a 1-d array (you just get the same array back...)
        MHO_NDArrayView< XValueType, 1 > SliceView(const char* /*! unused_arg */)
        {
            //just return a 1d array view of this 1-d array
            return MHO_NDArrayView< XValueType, 1 >(&(fData[0]), &(fDims[0]), &(fStrides[0]));
        }

        XValueType& ValueAt(const index_type& idx) { return fData[idx[0]]; }

        const XValueType& ValueAt(const index_type& idx) const { return fData[idx[0]]; }

        //in place multiplication by a scalar factor
        template< typename T >
        typename std::enable_if< std::is_same< XValueType, T >::value or std::is_integral< T >::value or
                                     std::is_floating_point< T >::value,
                                 MHO_NDArrayWrapper& >::type inline
        operator*=(T aScalar)
        {
            std::size_t length = fData.size();
            for(std::size_t i = 0; i < length; i++)
            {
                fData[i] *= aScalar;
            }
            return *this;
        }

        //in place addition by a scalar amount
        template< typename T >
        typename std::enable_if< std::is_same< XValueType, T >::value or std::is_integral< T >::value or
                                     std::is_floating_point< T >::value,
                                 MHO_NDArrayWrapper& >::type inline
        operator+=(T aScalar)
        {
            std::size_t length = fData.size();
            for(std::size_t i = 0; i < length; i++)
            {
                fData[i] += aScalar;
            }
            return *this;
        }

        //in place subraction by a scalar amount
        template< typename T >
        typename std::enable_if< std::is_same< XValueType, T >::value or std::is_integral< T >::value or
                                     std::is_floating_point< T >::value,
                                 MHO_NDArrayWrapper& >::type inline
        operator-=(T aScalar)
        {
            std::size_t length = fData.size();
            for(std::size_t i = 0; i < length; i++)
            {
                fData[i] -= aScalar;
            }
            return *this;
        }

        //in place point-wise multiplication by another array
        inline MHO_NDArrayWrapper& operator*=(const MHO_NDArrayWrapper& anArray)
        {
            if(!HaveSameNumberOfElements(this, &anArray))
            {
                throw std::out_of_range("MHO_NDArrayWrapper::*= size mismatch.");
            }
            std::size_t length = fData.size();
            for(std::size_t i = 0; i < length; i++)
            {
                fData[i] *= anArray.fData[i];
            }
            return *this;
        }

        //in place point-wise addition by another array of the same type
        inline MHO_NDArrayWrapper& operator+=(const MHO_NDArrayWrapper& anArray)
        {
            if(!HaveSameNumberOfElements(this, &anArray))
            {
                throw std::out_of_range("MHO_NDArrayWrapper::+= size mismatch.");
            }
            std::size_t length = fData.size();
            for(std::size_t i = 0; i < length; i++)
            {
                fData[i] += anArray.fData[i];
            }
            return *this;
        }

        //in place point-wise subtraction of another array
        inline MHO_NDArrayWrapper& operator-=(const MHO_NDArrayWrapper& anArray)
        {
            if(!HaveSameNumberOfElements(this, &anArray))
            {
                throw std::out_of_range("MHO_NDArrayWrapper::-= size mismatch.");
            }
            std::size_t length = fData.size();
            for(std::size_t i = 0; i < length; i++)
            {
                fData[i] -= anArray.fData[i];
            }
            return *this;
        }

    private:
        std::vector< XValueType > fData; //used for internally managed data
        index_type fDims;                //size of each dimension
        index_type fStrides;             //strides between elements in each dimension
        mutable index_type fTmp;         //temp index workspace

        void Construct(XValueType* ptr, const std::size_t* dim)
        {
            //default construction (empty)
            fDims[0] = 0;
            fStrides[0] = 0;
            if(ptr == nullptr && dim == nullptr)
            {
                return;
            }

            //dimensions known
            if(dim != nullptr)
            {
                fDims[0] = dim[0];
            }
            fStrides[0] = 1;

            fData.resize(fDims[0]);
            if(ptr != nullptr) //if given a ptr, copy in data from this location
            {
                std::memcpy(&(fData[0]), ptr, fData.size() * sizeof(XValueType));
            }
        }

        //the iterator definitions //////////////////////////////////////////////////
    public:
        using iterator = MHO_BidirectionalIterator< XValueType >;
        using stride_iterator = MHO_BidirectionalStrideIterator< XValueType >;

        using const_iterator = MHO_BidirectionalConstIterator< XValueType >;
        using const_stride_iterator = MHO_BidirectionalConstStrideIterator< XValueType >;

        iterator begin() { return iterator(&(fData[0]), &(fData[0]), fData.size()); }

        iterator end() { return iterator(&(fData[0]), &(fData[0]) + fData.size(), fData.size()); }

        iterator iterator_at(std::size_t offset)
        {
            return iterator(&(fData[0]), &(fData[0]) + std::min(offset, fData.size()), fData.size());
        }

        const_iterator cbegin() const { return const_iterator(&(fData[0]), &(fData[0]), fData.size()); }

        const_iterator cend() const { return const_iterator(&(fData[0]), &(fData[0]) + fData.size(), fData.size()); }

        const_iterator citerator_at(std::size_t offset) const
        {
            return const_iterator(&(fData[0]), &(fData[0]) + std::min(offset, fData.size()), fData.size());
        }

        stride_iterator stride_begin(std::size_t stride)
        {
            return stride_iterator(&(fData[0]), &(fData[0]), fData.size(), stride);
        }

        stride_iterator stride_end(std::size_t stride)
        {
            return stride_iterator(&(fData[0]), &(fData[0]) + fData.size(), fData.size(), stride);
        }

        stride_iterator stride_iterator_at(std::size_t offset, std::size_t stride)
        {
            return stride_iterator(&(fData[0]), &(fData[0]) + std::min(offset, fData.size()), fData.size(), stride);
        }

        const_stride_iterator cstride_begin(std::size_t stride) const
        {
            return const_stride_iterator(&(fData[0]), &(fData[0]), fData.size(), stride);
        }

        const_stride_iterator cstride_end(std::size_t stride) const
        {
            return const_stride_iterator(&(fData[0]), &(fData[0]) + fData.size(), fData.size(), stride);
        }

        const_stride_iterator cstride_iterator_at(std::size_t offset, std::size_t stride) const
        {
            return const_stride_iterator(&(fData[0]), &(fData[0]) + std::min(offset, fData.size()), fData.size(), stride);
        }
};

} // namespace hops

#endif /*! end of include guard: MHO_NDArrayWrapper_1 */
