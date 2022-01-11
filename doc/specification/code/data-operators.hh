class MHO_Operator
{
    public:
        MHO_Operator();
        virtual ~MHO_Operator();
        virtual bool Initialize() = 0;
        virtual bool Execute() = 0;
};

template<class XArgType>
class MHO_UnaryOperator: public MHO_Operator
{
    public:
        MHO_UnaryOperator();
        virtual ~MHO_UnaryOperator();
        virtual void SetArgs(XArgType* in); //in-place
        virtual void SetArgs(const XArgType* in, XArgType* out); //out-of-place
};

template<class XArgType1, class XArgType2 = XArgType1, class XArgType3 = XArgType2>
class MHO_BinaryOperator: public MHO_Operator
{
   public:
       MHO_BinaryOperator();
       virtual ~MHO_BinaryOperator();
       virtual void SetArgs(const XArgType1* in1, const XArgType2* in2, XArgType3* out) //out-of-place
};