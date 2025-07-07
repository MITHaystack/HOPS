#ifndef MHO_NDArrayWrapper_0_HH__
#define MHO_NDArrayWrapper_0_HH__

//this include file should not be used directly
#ifndef MHO_NDArrayWrapper_HH__
    #error "Do not include MHO_NDArrayWrapper_0.hh directly; use MHO_NDArrayWrapper.hh instead."
#endif

namespace hops
{


/**
 * @brief Class MHO_NDArrayWrapper<XValueType, 0> specialization for a RANK-0 (i.e. a scalar)
 */
template< typename XValueType >
class MHO_NDArrayWrapper< XValueType, 0 >
    : public MHO_ExtensibleElement //any and all extensions are purely a runtime concept and do NOT get streamed for I/O
{
    public:
        using value_type = XValueType;
        typedef std::integral_constant< std::size_t, 0 > rank;

        MHO_NDArrayWrapper(){};

        MHO_NDArrayWrapper(const XValueType& data) { fData = data; }

        MHO_NDArrayWrapper(const MHO_NDArrayWrapper& obj) { fData = obj.fData; }

        virtual ~MHO_NDArrayWrapper(){};

        //directly set/get the only value
        /**
         * @brief Setter for data
         * 
         * @param value New value to set as const XValueType&
         */
        void SetData(const XValueType& value) { fData = value; }

        /**
         * @brief Getter for data
         * 
         * @return Current value of type XValueType
         */
        XValueType GetData() { return fData; };

        /**
         * @brief Getter for rank
         * 
         * @return Current rank as std::size_t
         */
        std::size_t GetRank() const { return 0; }

        /**
         * @brief Getter for size
         * 
         * @return Size as std::size_t
         */
        std::size_t GetSize() const { return 1; };

        MHO_NDArrayWrapper& operator=(const MHO_NDArrayWrapper& rhs)
        {
            if(this != &rhs)
            {
                fData = rhs.fData;
            }
            return *this;
        }

        /**
         * @brief Setter for array
         * 
         * @param obj Reference to a constant XValueType object used to update internal data
         */
        void SetArray(const XValueType& obj) { fData = obj; }

        /**
         * @brief Sets all elements in the array to zero.
         */
        void ZeroArray() { std::memset(&fData, 0, sizeof(XValueType)); }

    protected:
        XValueType fData; //single value
};

} // namespace hops

#endif /*! end of include guard: MHO_NDArrayWrapper_0 */
