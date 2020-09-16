template< typename XValueType, typename XUnitType >
class ScalarContainer {
    public:
        ScalarContainer();
        virtual ~ScalarContainer();
        /*...TBD impl...*/
    private:
        std::string fName;
        XUnitType fUnit;
        XValueType fData;
};

template< typename XValueType, typename XUnitType >
class VectorContainer {
    public:
        std::string fName;
        VectorContainer();
        virtual ~VectorContainer();
        /*...TBD impl...*/
    private:
        std::string fName;
        XUnitType fUnit;
        std::vector< XValueType > fData;
};

template< typename XValueType, typename XUnitType, size_t RANK, typename... XAxisVectorContainerTypes >
class TensorContainer {
    public:
        TensorContainer();
        virtual ~TensorContainer();
        /*...TBD impl...*/
    private:
        std::string fName;
        XUnitType fUnit; //units of the data
        std::vector< XValueType > fData; //row-indexed block of data
        std::tuple< XAxisVectorContainerTypes > fAxes; //tuple length RANK of VectorContainers specifying each axis
};
