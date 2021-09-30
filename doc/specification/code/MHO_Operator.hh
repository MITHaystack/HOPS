class MHO_OperatorPrimitive
{
    public:
        MHO_OperatorPrimitive(){};
        virtual ~MHO_OperatorPrimitive(){};
        virtual int nargs() const {return 0;};
        virtual int init() = 0;
        virtual int exec() = 0;
};

class MHO_Operator
{
    public:
        MHO_Operator();
        virtual ~MHO_Operator();
        virtual int Initialize();
        virtual int Execute();
    protected:
        MHO_OperatorPrimitive* fOperator;
};

template< typename...XArgPtrTypeS >
class MHO_OperatorImpl: public MHO_OperatorPrimitive, public std::tuple< XArgPtrTypeS... >
{
    public:
        typedef std::integral_constant< std::size_t, sizeof...(XArgPtrTypeS) > NARGS;

        MHO_OperatorImpl():
            std::tuple< XArgPtrTypeS... >(){};

        MHO_OperatorImpl(XArgPtrTypeS...ptrs):
            std::tuple< XArgPtrTypeS... >( std::forward<XArgPtrTypeS>(ptrs)... ){}

        virtual ~MHO_OperatorImpl(){};

        void PassArgs(XArgPtrTypeS...ptrs)
        {
            *(static_cast< std::tuple< XArgPtrTypeS... >* >(this) ) =
                std::make_tuple( std::forward<XArgPtrTypeS>(ptrs)... );
        };

        virtual int nargs() const override {return NARGS::value;}
};


//operator specific
template< template <typename...> class XOperatorImplType >
class MHO_OperatorSpecific: public MHO_Operator
{
    public:

        MHO_OperatorSpecific(){};
        virtual ~MHO_OperatorSpecific(){};

        template< typename...XArgPtrTypeS >
        void SetArgs(XArgPtrTypeS... ptrs)
        {
            if(this->fOperator)
            {
                if( this->fOperator->nargs() == sizeof...(XArgPtrTypeS) )
                {
                    //pre-existing operator with the correct number of arguments already here
                    auto op = dynamic_cast< XOperatorImplType< XArgPtrTypeS... >* >(this->fOperator);
                    if(op)
                    {
                        op->PassArgs(std::forward<XArgPtrTypeS>(ptrs)...); return;
                    }
                }
                delete this->fOperator; this->fOperator = nullptr;
            }
            this->fOperator = new XOperatorImplType< XArgPtrTypeS... >( std::forward<XArgPtrTypeS>(ptrs)... );
        }

};



}

#endif /* end of include guard: MHO_Operator */
