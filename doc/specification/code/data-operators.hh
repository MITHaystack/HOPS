//template parameters must inherit from MHO_NDArrayWrapper
class MHO_DataOperator
{
    public:
       virtual bool Initialize() = 0;
       virtual bool ExecuteOperation() = 0;
    //...impl...
};



template<class XInputArrayType, class XOutputArrayType>
class MHO_NDArrayOperator: public MHO_DataOperator
{
    public:
       virtual void SetInput(XInputArrayType* in){fInput = in;};
       virtual void SetOutput(XOutputArrayType* out){fOutput = out;};
       virtual XInputArrayType* GetInput(){return fInput;};
       virtual XOutputArrayType* GetOutput(){return fOutput;};
       virtual bool Initialize() override;
       virtual bool ExecuteOperation() override;
    //...impl...
};

 //template parameters must inherit from MHO_NDArrayWrapper
template<class XInputArrayType1, class XInputArrayType2, class XOutputArrayType>
class MHO_BinaryNDArrayOperator:  public MHO_DataOperator
{
    public:
        virtual void SetFirstInput(XInputArrayType1* in){fInput1 = in;};
        virtual void SetSecondInput(XInputArrayType2* in){fInput2 = in;};
        virtual void SetOutput(XOutputArrayType* out){fOutput = out;};
        virtual XInputArrayType1* GetFirstInput(){return fInput1;};
        virtual XInputArrayType2* GetSecondInput(){return fInput2;};
        virtual XOutputArrayType* GetOutput(){return fOutput;};
        virtual bool Initialize() override;
        virtual bool ExecuteOperation() override;
        //...impl...
};
