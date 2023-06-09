#ifndef MHO_OperatorToolbox_HH__
#define MHO_OperatorToolbox_HH__

#include <string>
#include <map>
#include <vector>
#include <algorithm>

#include "MHO_Operator.hh"

/*
*File: MHO_OperatorToolbox.hh
*Class: MHO_OperatorToolbox
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

namespace hops
{

class MHO_OperatorToolbox
{
    public:

        MHO_OperatorToolbox(){}
        virtual ~MHO_OperatorToolbox(){Clear();}

        //insertion
        void AddOperator(MHO_Operator* op, const std::string& name, bool replace_duplicate=true)
        {
            auto it = fOperators.find(name);
            if(replace_duplicate)
            {
                if(it != fOperators.end())
                {
                    delete it->second;
                }
                fOperators[name] = op;
            }
            else
            {
                if(it == fOperators.end()){fOperators[name] = op;}
            }
        }

        //retrieval as generic, returns nullptr if none matching name present
        MHO_Operator* GetOperator(const std::string& name)
        {
            MHO_Operator* ptr = nullptr;
            auto it = fOperators.find(name);
            if(it != fOperators.end()){ptr = it->second;}
            return ptr;
        }

        //retrieval, with case to specific type, is missing returns nullptr
        template < typename XOperatorType >
        XOperatorType* GetOperatorAs(const std::string& name)
        {
            XOperatorType* ptr = nullptr;
            MHO_Operator* gptr = GetOperator(name);
            if(gptr != nullptr)
            {
                ptr = dynamic_cast<XOperatorType*>(gptr);
            }
            return ptr;
        }

        std::vector< MHO_Operator* > GetAllOperators()
        {
            std::vector< MHO_Operator* > ops;
            for(auto it = fOperators.begin(); it != fOperators.end(); it++)
            {
                ops.push_back( it->second );
            }

            operator_predicate op_pred;
            std::sort(ops.begin(), ops.end(), op_pred);
            return ops;
        }


    private:

        void Clear()
        {
            //delete all the operators
            for(auto it = fOperators.begin(); it != fOperators.end(); it++)
            {
                delete it->second;
            }
            fOperators.clear();
        };


        std::map< std::string, MHO_Operator* > fOperators;

        //for sorting operator priorites
        class operator_predicate
        {
            public:
                operator_predicate(){};
                virtual ~operator_predicate(){};

            virtual bool operator()(const MHO_Operator* a, const MHO_Operator* b)
            {
                return a->Priority() < b->Priority();
            }
        };



};


}//end of namespace

#endif /* end of include guard: MHO_OperatorToolbox */
