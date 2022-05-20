#ifndef MHO_NDArrayWrapper_0_HH__
#define MHO_NDArrayWrapper_0_HH__


//this include file should not be used directly
#ifndef MHO_NDArrayWrapper_HH__
#error "Do not include MHO_NDArrayWrapper_0.hh directly; use MHO_NDArrayWrapper.hh instead."
#endif

namespace hops
{


//specialization for a RANK-0 (i.e. a scalar)
template< typename XValueType >
class MHO_NDArrayWrapper<XValueType, 0>:
    public MHO_ExtensibleElement //any and all extensions are purely a runtime concept and do NOT get streamed for I/O
{
    public:

        using value_type = XValueType;
        typedef std::integral_constant< std::size_t, 0 > rank;

        MHO_NDArrayWrapper(){};
        MHO_NDArrayWrapper(const XValueType& data){fData = data;}
        MHO_NDArrayWrapper(const MHO_NDArrayWrapper& obj)
        {
            fData = obj.fData;
        }

        virtual ~MHO_NDArrayWrapper(){};

        //directly set/get the only value
        void SetData(const XValueType& value){fData = value;}
        XValueType GetData(){return fData;};

        std::size_t GetRank() const {return 0;}
        std::size_t GetSize() const {return 1;};

        MHO_NDArrayWrapper& operator=(const MHO_NDArrayWrapper& rhs)
        {
            if(this != &rhs)
            {
                fData = rhs.fData;
            }
            return *this;
        }

        void SetArray(const XValueType& obj){fData = obj;}
        void ZeroArray(){std::memset(&fData, 0, sizeof(XValueType) );}

    protected:

        XValueType fData; //single value
};


}//end of namespace

#endif /* end of include guard: MHO_NDArrayWrapper_0 */
