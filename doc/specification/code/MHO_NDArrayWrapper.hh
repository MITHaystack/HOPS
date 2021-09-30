template< typename XValueType, std::size_t RANK>
class MHO_NDArrayWrapper: public MHO_ExtensibleElement
{
    public:
        using value_type = XValueType;
        typedef std::integral_constant< std::size_t, RANK > rank;

        MHO_NDArrayWrapper();
	virtual ~MHO_NDArrayWrapper();

	class iterator {...};
	class strided_iterator {...};

    protected:
        XValueType* fDataPtr;
        bool fExternallyManaged;
        std::vector< XValueType > fData; //internally managed data
        std::size_t fDimensions[RANK]; //size of each dimension
        std::size_t fTotalArraySize; //total size of array
};

