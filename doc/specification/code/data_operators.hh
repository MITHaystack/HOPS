class DataOperator {
{
  public:
    DataOperator(){};
    virtual ~DataOperator(){};
    virtual void Initialize(){};
    virtual void ExecuteOperation() = 0;
};

template< typename XInputType, typename XOutputType >
class UnaryDataOperator: public DataOperator {
    public:
        UnaryDataOperator();
        virtual ~DataOperator();
        void SetInput(const XInputType& input );
        void SetOutput( XOutputType& output);
    //....impl...
}

template< typename XFirstInputType, typename XSecondInputType, typename XOutputType >
class BinaryDataOperator: public DataOperator {
    public:
        BinaryDataOperator();
        virtual ~DataOperator();
        void SetFirstInput(const XInputType& input1 );
        void SetSecondInput(const XInputType& input2 );
        void SetOutput( XOutputType& output);
    //....impl...
}
