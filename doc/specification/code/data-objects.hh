//implemenation omitted between {}
template< typename XValueType, std::size_t RANK>
class MHO_NDArrayWrapper
{
    //...impl omitted...
    protected:
        XValueType* fDataPtr; //raw pointer to memory
        bool fExternallyManaged; //flag for externally managed data
        std::vector< XValueType > fData; //used for internally managed data
        std::size_t fDimensions[RANK]; //size of each dimension
        std::size_t fTotalArraySize; //total size of array
    public:
        //STL style iterator
        class iterator {};
};

template< typename XValueType >
class MHO_ScalarContainer: public MHO_NDArrayWrapper< XValueType, 0>, public MHO_Named {};

template< typename XValueType >
class MHO_VectorContainer: public MHO_NDArrayWrapper< XValueType, 1>, public MHO_Named{};

template< typename XValueType >
class MHO_Axis: public MHO_VectorContainer< XValueType >, public MHO_IntervalLabelTree {};

template< typename...XAxisTypeS >
class MHO_AxisPack:  public std::tuple< XAxisTypeS... > {};

template< typename XValueType, typename XAxisPackType >
class MHO_TableContainer: public MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value>,
                          public XAxisPackType, public MHO_Named {};
