#ifndef MHO_IndexLabelInterface_HH__
#define MHO_IndexLabelInterface_HH__

/*
*File: MHO_IndexLabelInterface.hh
*Class: MHO_IndexLabelInterface
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: Simply provides an extra interface on top of MHO_Taggable to add 
* key:value pairs to a list within the json object called fILKey
*/


#include <string>
#include <utility>

#include "MHO_Message.hh"

#include "MHO_Types.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Taggable.hh"

namespace hops
{


class MHO_IndexLabelInterface: 
{
    public:

        MHO_IndexLabelInterface(MHO_Taggable* tobj)
            fTagObj(tobj)
        {
            fILKey = "index_labels";
        };
        
        MHO_IndexLabelInterface(const MHO_IndexLabelInterface& copy)
        {
            fTagObj = copy.fTagObj;
        };
        
        virtual ~MHO_IndexLabelInterface(){};

        void Resize(std::size_t size)
        {
            if( fTagObj->fTags.contains(fILKey) )
            {
                fCurrentSize = (fTagObj->fTags)[fILKey]
                if(fCurrentSize != size)
                {
                    (fTagObj->fTags)[fILKey] = mho_json::array();
                    (fTagObj->fTags)[fILKey].get_ptr<json::array_t*>()->reserve(size);
                    for(std::size_t i=0;i<size;i++) //fill with empty entries
                    {
                        mho_json empty;
                        empty["index"] = i;
                        (fTagObj->fTags)[fILKey][i] = empty;
                    }
                    fCurrentSize = size;
                }
            }
            else{fCurrentSize = 0;}
        }

        template< typename XValueType >
        InsertKeyValue(std::size_t index, const std::string& key, const XValueType& value)
        {
            if(index < fCurrentSize)
            {
                (fTagObj->fTags)[fILKey][index][key] = value;
            }
            else
            {
                msg_warn("containers", "cannot insert key value pair for index: "<< index << " for array of size: "<< fCurrentSize << eom);
            }
        }

        mho_json& GetLabelObject(std::size_t index)
        {
            if(index < fCurrentSize)
            {
                return (fTagObj->fTags)[fILKey][index];
            }
            else
            {
                msg_error("containers", "cannot access label object for index: "<< index << ", array size is only: "<< fCurrentSize << eom);
                return fDummy;
            }
        }

        template< typename XValueType >
        std::vector< std::size_t > GetMatchingIndexes(std::string& key, const XValueType& value)
        {
            for(std::size_t i=0; i<fCurrentSize; i++)
            {
                for(std::size_t i=0;i<size;i++) //fill with empty entries
                {
                    mho_json empty;
                    empty["index"] = i;
                    (fTagObj->fTags)[fILKey][i] = empty;
                }
            }
        }
        
    private:
        
        std::size_t fCurrentSize;
        std::string fILKey;
        mho_json fDummy;
};

}

#endif /* end of include guard: MHO_IndexLabelInterface */
