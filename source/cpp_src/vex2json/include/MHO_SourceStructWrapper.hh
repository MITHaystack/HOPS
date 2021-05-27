#ifndef MHO_SourceStructWrapper_HH__
#define MHO_SourceStructWrapper_HH__

#include "MHO_JSONOutputObject.hh"

extern "C"
{
#include "ovex.h"
}


namespace hops
{

class MHO_SourceStructWrapper: public MHO_JSONOutputObject
{
    public:

        MHO_SourceStructWrapper(source_struct aSource)
        {
            fSourceStruct = aSource;
        };

        MHO_SourceStructWrapper(){};

        virtual ~MHO_SourceStructWrapper(){};

        MHO_SourceStructWrapper(const MHO_SourceStructWrapper& copyObject)
        {
            fSourceStruct = copyObject.fSourceStruct;
        };

        MHO_SourceStructWrapper& operator=(const MHO_SourceStructWrapper& rhs)
        {
            if(&rhs != this)
            {
                fSourceStruct = rhs.fSourceStruct;
            }
            return *this;
        };

        virtual void DumpToJSON(json& json_obj);

        virtual const char* GetName() const {return "source_struct";};
        virtual const char* ClassName() const { return "source_struct"; };

    public:

        source_struct fSourceStruct;

};

}

#endif /* end of include guard: MHO_SourceStructWrapper_HH__ */
