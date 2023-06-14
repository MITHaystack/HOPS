#ifndef MHO_OperatorBuilderManager_HH__
#define MHO_OperatorBuilderManager_HH__


#include "MHO_Message.hh"
#include "MHO_OperatorBuilder.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{
    
class MHO_OperatorBuilderManager
{
    public:
        
        MHO_OperatorBuilderManager(MHO_OperatorToolbox* toolbox, 
                                   MHO_ContainerStore* cstore,
                                   MHO_ParameterStore* pstore,
                                   mho_json control_format
                                  ):
            fOperatorToolbox(toolbox),
            fContainerStore(cstore),
            fParameterStore(pstore)
        {
            fFormat = control_format;
            CreateBuilders();
        };

        virtual ~MHO_OperatorBuilderManager()
        {
            for(std::size_t i = 0; i<fAllBuilders.size(); i++)
            {
                delete fAllBuilders[i];
            }
            fAllBuilders.clear();
            fNameToBuilderMap.clear();
            fCategoryToBuilderMap.clear();
        }

        //pass in parsed control file elements 
        void SetControlStatements(mho_json* statements){fControl = statements;};

        void BuildOperatorCategory(const char* cat){std::string scat(cat); BuildOperatorCategory(scat);};
        void BuildOperatorCategory(const std::string& cat);

    private:
        
        void CreateBuilders();
        
        template<typename XBuilderType>
        void AddBuilderType(const char* builder_name, const mho_json& format)
        {
            std::string bn(builder_name);
            AddBuilderType<XBuilderType>(bn, format);
        };
        
        template<typename XBuilderType>
        void AddBuilderType(const std::string& builder_name, const mho_json& format)
        {
            auto it = fNameToBuilderMap.find(builder_name);
            if( it == fNameToBuilderMap.end()) //not found, so make one
            {
                auto builder = new XBuilderType(fOperatorToolbox, fContainerStore, fParameterStore);
                builder->SetFormat(format);
                
                //the builder's operator category comes from the format specification
                std::string category = "unknown";
                if(format.contains("operator_category"))
                {
                    category = format["operator_category"].get<std::string>(); 
                }
                
                std::cout<<"ADDING A BUILDER: "<<builder_name<<" - "<<category<<std::endl;
                
                fAllBuilders.push_back(builder);
                fNameToBuilderMap.emplace(builder_name, builder);
                fCategoryToBuilderMap.emplace(category, builder);
            }
        };
        
        //internal data
        mho_json fFormat; //control file statement formats
        mho_json* fControl; //control file statements

        //constructed operators all get stashed here
        MHO_OperatorToolbox* fOperatorToolbox;

        //data container and parameter stores
        MHO_ContainerStore* fContainerStore;
        MHO_ParameterStore* fParameterStore;
        
        //container to store all of the builders, for memory management
        std::vector< MHO_OperatorBuilder* > fAllBuilders;

        //name -> builder map for lookup by name 
        std::map< std::string, MHO_OperatorBuilder* > fNameToBuilderMap;

        //operator category -> builder multimap for lookup by category
        std::multimap< std::string, MHO_OperatorBuilder* > fCategoryToBuilderMap;

};

}


#endif /* end of include guard: MHO_OperatorBuilderManager_HH__ */
