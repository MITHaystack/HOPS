#ifndef MHO_OperatorToolbox_HH__
#define MHO_OperatorToolbox_HH__

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "MHO_Operator.hh"

namespace hops
{

/*!
 *@file MHO_OperatorToolbox.hh
 *@class MHO_OperatorToolbox
 *@author J. Barrett - barrettj@mit.edu
 *@date Sun Jun 4 17:43:54 2023 -0400
 *@brief The toolbox class stores all operator objects as points to the MHO_Operator base class
 */

/**
 * @brief Class MHO_OperatorToolbox
 */
class MHO_OperatorToolbox
{
    public:
        MHO_OperatorToolbox() {}

        virtual ~MHO_OperatorToolbox() { Clear(); }

        //insertion
        /**
         * @brief Adds an operator to the toolbox with optional replacement if duplicate name exists.
         * 
         * @param op Pointer to the MHO_Operator to be added
         * @param name Name of the operator (duplicate names can be replaced)
         * @param category Category under which the operator will be stored
         * @param replace_duplicate Flag indicating whether to replace duplicate operators by name
         */
        void AddOperator(MHO_Operator* op, const std::string& name, const std::string& category, bool replace_duplicate = true)
        {
            msg_debug("operators",
                      "adding an operator to the toolbox with name: " << name << " in category: " << category << eom);
            auto it = fNameToOperatorMap.find(name);
            if(it != fNameToOperatorMap.end() && replace_duplicate)
            {
                RemoveOperator(name);
            }

            fNameToOperatorMap[name] = op;
            fCategoryToOperatorMap.emplace(category, op);
            fOperators.insert(op);
        }

        /**
         * @brief Getter for operator - retrieval by name as generic operator, returns nullptr if missing
         * 
         * @param name Operator name to search for in the map
         * @return Pointer to MHO_Operator or nullptr if not found
         */
        MHO_Operator* GetOperator(const std::string& name)
        {
            MHO_Operator* ptr = nullptr;
            auto it = fNameToOperatorMap.find(name);
            if(it != fNameToOperatorMap.end())
            {
                ptr = it->second;
            }
            return ptr;
        }

        /**
         * @brief Getter for an operator by name
         * 
         * @param name Operator name to search for in the map
         * @return Pointer to MHO_Operator or nullptr if not found
         */
        MHO_Operator* GetOperator(const char* name)
        {
            std::string sname(name);
            return GetOperator(sname);
        }

        /**
         * @brief Getter for operator, retrieval by name, with cast to specified type (XOperatorType), if missing returns nullptr
         * 
         * @param name Operator name to retrieve
         * @return Pointer to operator cast as XOperatorType or nullptr if not found/cannot be cast
         */
        template< typename XOperatorType > XOperatorType* GetOperatorAs(const std::string& name)
        {
            XOperatorType* ptr = nullptr;
            MHO_Operator* gptr = GetOperator(name);
            if(gptr != nullptr)
            {
                ptr = dynamic_cast< XOperatorType* >(gptr);
            }
            return ptr;
        }

        /**
         * @brief Getter for number of operators
         * 
         * @return Size of fOperators vector as std::size_t
         */
        std::size_t GetNOperators() { return fOperators.size(); }

        //get all operators in the toolbox
        /**
         * @brief Getter for all operators (vector of pointers)
         * 
         * @return std::vector<MHO_Operator* sorted list of operators
         */
        std::vector< MHO_Operator* > GetAllOperators()
        {
            std::vector< MHO_Operator* > ops;
            for(auto it = fOperators.begin(); it != fOperators.end(); it++)
            {
                ops.push_back(*it);
            }

            operator_predicate op_pred;
            std::sort(ops.begin(), ops.end(), op_pred);
            return ops;
        }

        /**
         * @brief Getter for operators by priority range - get all operators within the priority range [low,high)
         * 
         * @param lower_limit Lower bound of priority range
         * @param upper_limit Upper bound of priority range
         * @return Vector of MHO_Operator pointers sorted by priority
         */
        std::vector< MHO_Operator* > GetOperatorsByPriorityRange(double lower_limit, double upper_limit)
        {
            std::vector< MHO_Operator* > ops;
            for(auto it = fOperators.begin(); it != fOperators.end(); it++)
            {
                double priority = (*it)->Priority();
                if(priority < upper_limit && lower_limit <= priority)
                {
                    ops.push_back(*it);
                }
            }
            //sort in order of priority
            operator_predicate op_pred;
            std::sort(ops.begin(), ops.end(), op_pred);
            return ops;
        }

        //get all operators by category
        /**
         * @brief Getter for operators by category
         * 
         * @param category The category of operators to retrieve.
         * @return A vector of MHO_Operator pointers sorted by priority.
         */
        std::vector< MHO_Operator* > GetOperatorsByCategory(const std::string& category)
        {
            std::vector< MHO_Operator* > ops;
            auto it1 = fCategoryToOperatorMap.lower_bound(category);
            auto it2 = fCategoryToOperatorMap.upper_bound(category);
            if(it1 != fCategoryToOperatorMap.end())
            {
                while(it1 != it2)
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
        /**
         * @brief Function RemoveOperator - removes operator from the toolbox
         * 
         * @param name (const std::string&)
         */
        void RemoveOperator(const std::string& name)
        {
            auto it = fNameToOperatorMap.find(name);
            if(it != fNameToOperatorMap.end())
            {
                auto op_ptr = it->second;
                //remove from the operator set
                auto op_iter = fOperators.find(op_ptr);
                if(op_iter != fOperators.end())
                {
                    fOperators.erase(op_iter);
                }

                //remove from the category map
                for(auto cat_iter = fCategoryToOperatorMap.begin(); cat_iter != fCategoryToOperatorMap.end(); cat_iter++)
                {
                    if(cat_iter->second == op_ptr)
                    {
                        fCategoryToOperatorMap.erase(cat_iter);
                        break;
                    }
                }

                //remove from the named operator map
                fNameToOperatorMap.erase(it);

                //finally delete the operator
                delete op_ptr;
            }
        }

        /**
         * @brief Clears the current collection of operators
         */
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
        /**
         * @brief Class operator_predicate
         */
        class operator_predicate
        {
            public:
                operator_predicate(){};
                virtual ~operator_predicate(){};

                virtual bool operator()(const MHO_Operator* a, const MHO_Operator* b) { return a->Priority() < b->Priority(); }
        };
};

} // namespace hops

#endif /*! end of include guard: MHO_OperatorToolbox */
