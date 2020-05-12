
template< typename XValueType, typename XUnitType, size_t RANK, typename... XAxisTypes >
class TensorContainer {
    public:
        TensorContainer();
        virtual ~TensorContainer();
        //...TBD impl...
    private:
        std::string fName;
        XUnitType fUnit; //units of the data
        std::vector< XValueType > fData; //row-indexed block of data
        std::tuple< XAxisTypes > fAxes; //tuple of length RANK of VectorContainers
};
