#ifndef MHO_OperatorBuilderManager_HH__
#define MHO_OperatorBuilderManager_HH__

#include "MHO_ContainerStore.hh"
#include "MHO_FringeData.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_OperatorBuilder.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_ParameterStore.hh"

namespace hops
{

/*!
 *@file MHO_OperatorBuilderManager.hh
 *@class MHO_OperatorBuilderManager
 *@date Thu Jun 8 17:05:29 2023 -0400
 *@brief Manages all the various operator builders
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_OperatorBuilderManager
{
    public:
        MHO_OperatorBuilderManager(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata, mho_json control_format)
            : fOperatorToolbox(toolbox), fFringeData(fdata), fContainerStore(fdata->GetContainerStore()),
              fParameterStore(fdata->GetParameterStore())
        {
            fFormat = control_format;
        };

        virtual ~MHO_OperatorBuilderManager()
        {
            for(std::size_t i = 0; i < fAllBuilders.size(); i++)
            {
                delete fAllBuilders[i];
            }
            fAllBuilders.clear();
            fNameToBuilderMap.clear();
            fCategoryToBuilderMap.clear();
        }

        /**
         * @brief pass in parsed control file elements
         *
         * @param statements Input mho_json object containing control file statements
         */
        void SetControlStatements(mho_json* statements) { fControl = statements; };

        /**
         * @brief Registers default operator builders for various purposes such as channel labeling,
         * phase and delay corrections, pol-product summation, flagging operators, etc.
         */
        void CreateDefaultBuilders();

        /**
         * @brief Builds operator category from input string and calls BuildOperatorCategory with it.
         *
         * @param cat Input operator category as a C-style string.
         */
        void BuildOperatorCategory(const char* cat)
        {
            std::string scat(cat);
            BuildOperatorCategory(scat);
        };

        /**
         * @brief Builds operator category from input string and calls BuildOperatorCategory with it.
         *
         * @param cat Input operator category as a C-style string.
         */
        void BuildOperatorCategory(const std::string& cat);

        /**
         * @brief return the number of operator builders in the specified category
         *
         * @param cat operator category
         */
        std::size_t GetNBuildersInCategory(std::string cat);

        /**
         * @brief Adds a new builder type with specified format, inserted into (builder) map for later use.
         *
         * @param builder_name Name of the builder type as string
         * @param format_key (const std::string&)
         * @return void
         */
        template< typename XBuilderType > void AddBuilderType(const std::string& builder_name, const std::string& format_key)
        {
            auto format_it = fFormat.find(format_key);
            if(format_it != fFormat.end())
            {
                auto it = fNameToBuilderMap.find(builder_name);
                if(it == fNameToBuilderMap.end()) //not found, so make one
                {
                    auto builder = new XBuilderType(fOperatorToolbox, fFringeData);
                    builder->SetFormat(fFormat[format_key]);

                    //the builder's operator category comes from the format specification
                    std::string category = "unknown"; //default's to unknown
                    if(format_it->contains("operator_category"))
                    {
                        category = (*format_it)["operator_category"].get< std::string >();
                    }
                    fAllBuilders.push_back(builder);
                    fNameToBuilderMap.emplace(builder_name, builder);
                    fCategoryToBuilderMap.emplace(category, builder);
                }
            }
            else
            {
                msg_error("initialization", "cannot add builder for operator with format key: " << format_key << eom);
            }
        };

    private:
        /**
         * @brief Registers default operator builders for MHO_OperatorBuilderManager.
         */
        void CreateNullFormatBuilders();

        /**
         * @brief Adds a new builder type with specified format and maps it by name and category.
         *
         * @param builder_name Name of the builder to be added
         * @param format Format specification for the builder
         * @return void
         */
        template< typename XBuilderType > void AddBuilderTypeWithFormat(const std::string& builder_name, const mho_json& format)
        {
            auto it = fNameToBuilderMap.find(builder_name);
            if(it == fNameToBuilderMap.end()) //not found, so make one
            {
                // auto builder = new XBuilderType(fOperatorToolbox, fContainerStore, fParameterStore);
                auto builder = new XBuilderType(fOperatorToolbox, fFringeData);
                builder->SetFormat(format);

                //the builder's operator category comes from the format specification
                std::string category = "unknown"; //default's to unknown
                if(format.contains("operator_category"))
                {
                    category = format["operator_category"].get< std::string >();
                }
                fAllBuilders.push_back(builder);
                fNameToBuilderMap.emplace(builder_name, builder);
                fCategoryToBuilderMap.emplace(category, builder);
            }
        };

        //internal data
        mho_json fFormat;   //control file statement formats
        mho_json* fControl; //control file statements

        //constructed operators all get stashed here
        MHO_OperatorToolbox* fOperatorToolbox;

        //data container and parameter stores
        MHO_FringeData* fFringeData;
        MHO_ContainerStore* fContainerStore;
        MHO_ParameterStore* fParameterStore;

        //container to store all of the builders, for memory management
        std::vector< MHO_OperatorBuilder* > fAllBuilders;

        //name -> builder map for lookup by name
        std::map< std::string, MHO_OperatorBuilder* > fNameToBuilderMap;

        //operator category -> builder multimap for lookup by category
        std::multimap< std::string, MHO_OperatorBuilder* > fCategoryToBuilderMap;
};

} // namespace hops

#endif /*! end of include guard: MHO_OperatorBuilderManager_HH__ */
