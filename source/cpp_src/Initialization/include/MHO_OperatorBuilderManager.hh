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
                                   MHO_ContainerStore* cstore = nullptr,
                                   MHO_ParameterStore* pstore = nullptr):
            fOperatorToolbox(toolbox),
            fContainerStore(cstore),
            fParameterStore(pstore)
        {
            CreateBuilders();
        };

        virtual ~MHO_OperatorBuilderManager()
        {
            for(auto it = fBuilderMap.begin(); it != fBuilderMap.end(); it++)
            {
                delete it->second;
            }
        }

        void SetControlStatements(const mho_json& statements){fControl = statements;};
        
        void BuildAll();

    private:
        
        void CreateBuilders();
        
        template<typename XBuilderType>
        void AddBuilderType(const char* builder_name)
        {
            std::string bn(builder_name);
            AddBuilderType<XBuilderType>(bn);
        };
        
        template<typename XBuilderType>
        void AddBuilderType(const std::string& builder_name)
        {
            auto it = fBuilderMap.find(builder_name);
            if( it == fBuilderMap.end()) //not found, so make one
            {
                fBuilderMap.emplace(builder_name, new XBuilderType(fOperatorToolbox, fContainerStore, fParameterStore) );
            }
        };
        
        mho_json fControl;

        //constructed operators all get stashed here
        MHO_OperatorToolbox* fOperatorToolbox;

        //data container and parameter stores
        MHO_ContainerStore* fContainerStore;
        MHO_ParameterStore* fParameterStore;
        
        //map to builders which do the actual work
        std::map< std::string, MHO_OperatorBuilder* > fBuilderMap;
};

}


#endif /* end of include guard: MHO_OperatorBuilderManager_HH__ */
