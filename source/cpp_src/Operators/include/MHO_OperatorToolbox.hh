#ifndef MHO_OperatorToolbox_HH__
#define MHO_OperatorToolbox_HH__

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "MHO_Message.hh"
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

        MHO_OperatorToolbox(const MHO_OperatorToolbox&) = delete;
        MHO_OperatorToolbox& operator=(const MHO_OperatorToolbox&) = delete;

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
        void AddOperator(std::unique_ptr< MHO_Operator > op, const std::string& name, const std::string& category,
                         bool replace_duplicate = true)
        {
            msg_debug("operators",
                      "adding an operator to the toolbox with name: " << name << " in category: " << category << eom);
            auto it = fNameToOperatorMap.find(name);
            if(it != fNameToOperatorMap.end() && replace_duplicate)
            {
                RemoveOperator(name);
            }

            MHO_Operator* raw = op.get();
            fNameToOperatorMap[name] = raw;
            fCategoryToOperatorMap.emplace(category, raw);
            fOperators.push_back(std::move(op));
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
         * @brief Getter for all operators with the given name - returns all operators whose name matches,
         * sorted by priority. Use this when multiple operators may share the same name.
         *
         * @param name Operator name to search for
         * @return Vector of MHO_Operator pointers sorted by priority
         */
        std::vector< MHO_Operator* > GetAllOperatorsByName(const std::string& name)
        {
            std::vector< MHO_Operator* > ops;
            for(const auto& uptr : fOperators)
            {
                if(uptr->GetName() == name)
                {
                    ops.push_back(uptr.get());
                }
            }
            operator_predicate op_pred;
            std::sort(ops.begin(), ops.end(), op_pred);
            return ops;
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
            for(const auto& uptr : fOperators)
            {
                ops.push_back(uptr.get());
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
            for(const auto& uptr : fOperators)
            {
                double priority = uptr->Priority();
                if(priority < upper_limit && lower_limit <= priority)
                {
                    ops.push_back(uptr.get());
                }
            }
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

        std::size_t GetNOperatorsInCategory(const std::string& cat) { return fCategoryToOperatorMap.count(cat); }

        //debugging
        void PrintOperatorNames()
        {
            for(auto it = fNameToOperatorMap.begin(); it != fNameToOperatorMap.end(); it++)
            {
                std::string op_name = it->first;
                MHO_Operator* op_ptr = it->second;

                //brute force search for category
                std::string op_category = "none";
                for(auto it2 = fCategoryToOperatorMap.begin(); it2 != fCategoryToOperatorMap.end(); it2++)
                {
                    if(it->second == op_ptr)
                    {
                        op_category = it->first;
                        break;
                    }
                }

                std::cout << "operator: " << op_name << ", category: " << op_category << std::endl;
            }
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
                MHO_Operator* op_ptr = it->second;

                fNameToOperatorMap.erase(it);

                for(auto cat_iter = fCategoryToOperatorMap.begin(); cat_iter != fCategoryToOperatorMap.end(); ++cat_iter)
                {
                    if(cat_iter->second == op_ptr)
                    {
                        fCategoryToOperatorMap.erase(cat_iter);
                        break;
                    }
                }

                // erase from owned storage last — unique_ptr destructor deletes the object
                auto owned_it =
                    std::find_if(fOperators.begin(), fOperators.end(),
                                 [op_ptr](const std::unique_ptr< MHO_Operator >& uptr) { return uptr.get() == op_ptr; });
                if(owned_it != fOperators.end())
                {
                    fOperators.erase(owned_it);
                }
            }
        }

        /**
         * @brief Clears the current collection of operators
         */
        void Clear()
        {
            fOperators.clear(); // unique_ptrs delete their objects
            fNameToOperatorMap.clear();
            fCategoryToOperatorMap.clear();
        }

        // owned storage — unique_ptrs are the sole owners
        std::vector< std::unique_ptr< MHO_Operator > > fOperators;

        //look up operators by name (non-owning)
        std::map< std::string, MHO_Operator* > fNameToOperatorMap;

        //look up operators by category (non-owning)
        std::multimap< std::string, MHO_Operator* > fCategoryToOperatorMap;

        struct operator_predicate
        {
                bool operator()(const MHO_Operator* a, const MHO_Operator* b) const { return a->Priority() < b->Priority(); }
        };
};

} // namespace hops

#endif /*! end of include guard: MHO_OperatorToolbox */
