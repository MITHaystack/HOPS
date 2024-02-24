#ifndef MHO_OperatorToolbox_HH__
#define MHO_OperatorToolbox_HH__

#include <string>
#include <map>
#include <vector>
#include <algorithm>

#include "MHO_Operator.hh"



namespace hops
{

/*!
*@file MHO_OperatorToolbox.hh
*@class MHO_OperatorToolbox
*@author J. Barrett - barrettj@mit.edu
*@date
*@brief
*/

class MHO_OperatorToolbox
{
    public:

        MHO_OperatorToolbox(){}
        virtual ~MHO_OperatorToolbox(){Clear();}

        //insertion
        void AddOperator(MHO_Operator* op, const std::string& name, const std::string& category, bool replace_duplicate=true)
        {
            msg_debug("operators", "adding an operator to the toolbox with name: "<< name <<" in category: "<< category << eom);
            auto it = fNameToOperatorMap.find(name);
            if(it != fNameToOperatorMap.end() && replace_duplicate)
            {
                RemoveOperator(name);
            }

            fNameToOperatorMap[name] = op;
            fCategoryToOperatorMap.emplace(category,op);
            fOperators.insert(op);
        }

        //retrieval by name as generic operator, returns nullptr if missing
        MHO_Operator* GetOperator(const std::string& name)
        {
            MHO_Operator* ptr = nullptr;
            auto it = fNameToOperatorMap.find(name);
            if(it != fNameToOperatorMap.end()){ptr = it->second;}
            return ptr;
        }

        MHO_Operator* GetOperator(const char* name)
        {
            std::string sname(name);
            return GetOperator(sname);
        }

        //retrieval by name, with cast to specific type, if missing returns nullptr
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

        std::size_t GetNOperators(){return fOperators.size();}

        //get all operators in the toolbox
        std::vector< MHO_Operator* > GetAllOperators()
        {
            std::vector< MHO_Operator* > ops;
            for(auto it = fOperators.begin(); it != fOperators.end(); it++)
            {
                ops.push_back( *it);
            }

            operator_predicate op_pred;
            std::sort(ops.begin(), ops.end(), op_pred);
            return ops;
        }

        //get all operators within the priority range [low,high)
        std::vector< MHO_Operator* > GetOperatorsByPriorityRange(double lower_limit, double upper_limit)
        {
            std::vector< MHO_Operator* > ops;
            for(auto it = fOperators.begin(); it != fOperators.end(); it++)
            {
                double priority = (*it)->Priority();
                if(priority < upper_limit && lower_limit <= priority )
                {
                    ops.push_back( *it );
                }
            }
            //sort in order of priority
            operator_predicate op_pred;
            std::sort(ops.begin(), ops.end(), op_pred);
            return ops;
        }

        //get all operators by category
        std::vector< MHO_Operator* > GetOperatorsByCategory(const std::string& category)
        {
            std::vector< MHO_Operator* > ops;
            auto it1 = fCategoryToOperatorMap.lower_bound(category);
            auto it2 = fCategoryToOperatorMap.upper_bound(category);
            if(it1 != fCategoryToOperatorMap.end() )
            {
                while (it1 != it2)
                {
                    ops.push_back(it1->second);
                    it1++;
                }
            }
            //sort in order of priority
            operator_predicate op_pred;
            std::sort(ops.begin(), ops.end(), op_pred);
            return ops;
        }


    private:

        void RemoveOperator(const std::string& name)
        {
            auto it = fNameToOperatorMap.find(name);
            if(it != fNameToOperatorMap.end())
            {
                auto op_ptr = it->second;
                //remove from the operator set
                auto op_iter = fOperators.find(op_ptr);
                if(op_iter != fOperators.end()){fOperators.erase(op_iter);}

                //remove from the category map
                for(auto cat_iter = fCategoryToOperatorMap.begin(); cat_iter != fCategoryToOperatorMap.end(); cat_iter++)
                {
                    if(cat_iter->second == op_ptr){fCategoryToOperatorMap.erase(cat_iter); break;}
                }

                //remove from the named operator map
                fNameToOperatorMap.erase(it);

                //finally delete the operator
                delete op_ptr;
            }
        }

        void Clear()
        {
            //delete all the operators
            for(auto it = fOperators.begin(); it != fOperators.end(); it++)
            {
                delete *it;
            }
            fOperators.clear();
            fNameToOperatorMap.clear();
            fCategoryToOperatorMap.clear();
        };

        //store operator pointers for memory management
        std::set< MHO_Operator* > fOperators;

        //look up operators by name
        std::map< std::string, MHO_Operator* > fNameToOperatorMap;

        //look up operators by category
        std::multimap< std::string, MHO_Operator* > fCategoryToOperatorMap;

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

#endif /*! end of include guard: MHO_OperatorToolbox */
