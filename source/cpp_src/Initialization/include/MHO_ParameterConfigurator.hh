#ifndef MHO_ParameterConfigurator_HH__
#define MHO_ParameterConfigurator_HH__

#include <string>
#include <utility>
#include <vector>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"

namespace hops
{

/*!
 *@file MHO_ParameterConfigurator.hh
 *@class MHO_ParameterConfigurator
 *@date Mon Jun 12 12:51:32 2023 -0400
 *@brief
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_ParameterConfigurator
{

    public:
        MHO_ParameterConfigurator(MHO_ParameterStore* pstore, mho_json control_format): fParameterStore(pstore)
        {
            fFormat = control_format;
        };

        virtual ~MHO_ParameterConfigurator(){};

        //json config for this parameter (parsed from the control file)
        /**
         * @brief Setter for conditions
         * 
         * @param cond Input conditions of type mho_json
         * @note This is a virtual function.
         */
        virtual void SetConditions(const mho_json& cond) { fConditions = cond; } //conditional statements

        /**
         * @brief Setter for attributes
         * 
         * @param attr Input attribute object to set
         * @note This is a virtual function.
         */
        virtual void SetAttributes(const mho_json& attr) { fAttributes = attr; }; //configuration parameters

        /**
         * @brief Configures weight channel frequencies by parsing attributes and determining parameter types.
         * 
         * @return bool indicating success (true) or failure (false)
         * @note This is a virtual function.
         */
        virtual bool Configure();

    protected:
        /*! data */
        enum class ParamType
        {
            config,
            global,
            station,
            baseline,
            fit,
            plot,
            unknown
        };

        enum class ParamValueType
        {
            int_type,         //single integer parameter
            real_type,        //single float parameter
            bool_type,        //single boolean parameter
            string_type,      //single string parameter
            list_int_type,    //list of ints with arbitrary length
            list_real_type,   //list of floats with arbitrary length
            list_string_type, //list of strings with arbitrary length
            compound_type,    //multiple elements of different types
            logical_intersection_list_string_type, //list of strings, if multiple lists are encountered take the logical intersection (e.g. freqs)
            unknown
        };

        typedef ParamType param_t;
        typedef ParamValueType paramv_t;

        /**
         * @brief Determines parameter type from string input.
         * 
         * @param par_type Input parameter type as string.
         * @return Parameter type enum based on input string.
         */
        ParamType DetermineParamType(const std::string& par_type) const;
        /**
         * @brief Determines the parameter value type from a given string representation.
         * 
         * @param par_value_type Input string representing the parameter value type.
         * @return Enum value of ParamValueType corresponding to the input string.
         */
        ParamValueType DetermineParamValueType(const std::string& par_value_type) const;

        /**
         * @brief Setter for scalar parameter
         * 
         * @param path Parameter path as std::string
         * @param value Scalar value to set, type XValueType
         * @return void
         */
        template< typename XValueType > void SetScalarParameter(std::string path, const XValueType& value);
        /**
         * @brief Getter for scalar parameter
         * 
         * @tparam XValueType Template parameter XValueType
         * @param path Path to the parameter as a std::string
         * @param value Reference to the scalar parameter value of type XValueType
         * @return void, with success indicated by modifying 'value'
         */
        template< typename XValueType > void GetScalarParameter(std::string path, XValueType& value);

        /**
         * @brief Setter for vector parameter
         * 
         * @tparam XValueType Template parameter XValueType
         * @param path Parameter path as std::string
         * @param values Vector of values to set at given path
         * @return void
         */
        template< typename XValueType > void SetVectorParameter(std::string path, const std::vector< XValueType >& values);
        /**
         * @brief Getter for vector parameter
         * 
         * @tparam XValueType Template parameter XValueType
         * @tparam std::vector< Template parameter std::vector<
         * @param path The path to the parameter in the parameter store.
         * @param values (std::vector< XValueType )&
         * @return void
         */
        /**
         * @brief Setter for compound parameter
         * 
         * @tparam XValueType Template parameter XValueType
         * @tparam XValueType Template parameter XValueType
         * @param path Path to the compound parameter as a std::string.
         * @param values JSON values to set for the compound parameter.
         */
        template< typename XValueType > void GetVectorParameter(std::string path, std::vector< XValueType >& values);

        void SetCompoundParameter(std::string path, const mho_json& values);

        /**
         * @brief Calculates logical intersection of two sorted string vectors using a labeler.
         * 
         * @param values1 First vector of strings to find intersection.
         * @param values2 Second vector of strings to find intersection.
         * @return Vector of strings representing the logical intersection.
         */
        std::vector< std::string > LogicalIntersection(std::vector< std::string >& values1, std::vector< std::string>& values2) const;

        MHO_ParameterStore* fParameterStore;
        mho_json fFormat; //format description for each parameter

        //provided for the configuration of the parameter that is to be set
        mho_json fConditions;
        mho_json fAttributes;
};

/**
 * @brief Sets a scalar parameter in the MHO system using a given path and value.
 * 
 * @param path The path to the parameter as a string.
 * @param value The new value for the parameter of type XValueType.
 * @return No return value (void).
 */
template< typename XValueType > void MHO_ParameterConfigurator::SetScalarParameter(std::string path, const XValueType& value)
{
    bool ok = fParameterStore->Set(path, value);
    if(!ok)
    {
        msg_warn("initialization", "could not set parameter: " << path << eom);
    }
}

/**
 * @brief Retrieves a scalar parameter value from the parameter store by its path.
 * 
 * @param path The path to the desired parameter.
 * @param value (XValueType&)
 * @return void
 */
template< typename XValueType > void MHO_ParameterConfigurator::GetScalarParameter(std::string path, XValueType& value)
{
    bool ok = fParameterStore->Get(path, value);
    if(!ok)
    {
        msg_info("initialization", "could not get parameter, using default value for: " << path << eom);
    }
}

/**
 * @brief Sets a vector parameter at the specified path in the parameter store.
 * 
 * @tparam XValueType Template parameter XValueType
 * @param path The path to set the vector parameter at.
 * @param values A reference to the vector of XValueType values to be set.
 */
template< typename XValueType >
void MHO_ParameterConfigurator::SetVectorParameter(std::string path, const std::vector< XValueType >& values)
{
    bool ok = fParameterStore->Set(path, values);
    if(!ok)
    {
        msg_warn("initialization", "could not set parameter vector: " << path << eom);
    }
}

/**
 * @brief Retrieves a vector parameter from the parameter store using the given path.
 * 
 * @tparam XValueType Template parameter XValueType
 * @param path The path to the desired parameter.
 * @param values (std::vector< XValueType )&
 */
template< typename XValueType >
void MHO_ParameterConfigurator::GetVectorParameter(std::string path, std::vector< XValueType >& values)
{
    bool ok = fParameterStore->Get(path, values);
    if(!ok)
    {
        msg_info("initialization", "could not get parameter vector, using default value for: " << path << eom);
    }
}

} // namespace hops

#endif /*! end of include guard: MHO_ParameterConfigurator_HH__ */
