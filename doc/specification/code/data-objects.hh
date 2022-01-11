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
