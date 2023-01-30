#ifndef MHO_SelectIndexesByAxisLabel_HH__
#define MHO_SelectIndexesByAxisLabel_HH__

#include <set>

#include "MHO_Meta.hh"
#include "MHO_Message.hh"
#include "MHO_InspectingOperator.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TableContainer.hh"

/*
*File: MHO_SelectIndexesByAxisLabel.hh
*Class: MHO_SelectIndexesByAxisLabel
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:

*/


namespace hops
{

template< class XArgType, std::size_t XAxisIndex >
class MHO_SelectIndexesByAxisLabel:
    public MHO_InspectingOperator< XArgType >
{
    public:

        //for a table container, this returns the type associated with
        //the axis labels of XAxisIndex-th axis
        using XAxisType = typename std::tuple_element<XAxisIndex, XArgType>::type;
        using XAxisLabelType = typename XAxisType::value_type;

        MHO_SelectIndexesByAxisLabel(){};
        virtual ~MHO_SelectIndexesByAxisLabel(){};

        void Reset(){ fLabelValues.clear(); fSelectedIndexes.clear();};
        void AddAxisLabelToSelect(XAxisLabelType label_value){fLabelValues.push_back(label_value);}
        std::vector< std::size_t > GetSelectedIndexes(){return fSelectedIndexes;};

    protected:

        virtual bool InitializeImpl(const XArgType* /*in*/) override {return true;};
        virtual bool ExecuteImpl(const XArgType* in) override { Search(in); return true;}

    private:

        std::set< XAxisLabelType > fLabelValues;
        std::vector< std::size_t > fSelectedIndexes;

        void Search(const XArgType* in)
        {
            fSelectedIndexes.clear();
            //dumb brute force search, for each label value, check all the axis element labels for a match 
            for(auto label_it = fLabelValues.begin(); label_it != fLabelValues.end(); label_it++)
            {
                const XAxisType* ax = &( std::get<XAxisIndex>(*in));
                for(std::size_t i = 0; i < ax->GetSize(); i++)
                {
                    if( (*ax)[i] == *label_it )
                    {
                        fSelectedIndexes.push_back(i);
                    }
                }
            }
        };



};







};




#endif /* MHO_SelectIndexesByAxisLabel_H__ */
