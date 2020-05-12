template< typename XValueType, typename XUnitType >
class ScalarContainer {
    public:
        ScalarContainer();
        virtual ~ScalarContainer();
        //...TBD impl...
    private:
        std::string fName;
        XUnitType fUnit;
        XValueType fData;
};
