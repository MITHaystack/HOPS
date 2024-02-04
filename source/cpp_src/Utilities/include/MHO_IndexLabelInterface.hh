#ifndef MHO_IndexLabelInterface_HH__
#define MHO_IndexLabelInterface_HH__

#include "MHO_JSONHeaderWrapper.hh"

namespace hops 
{

    
    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

    //constructor is protected
    //this class is only intended to provide an interface that derived classes may inherit
    //this interface is to enforce a specific access pattern associated with modifying
    //meta data attached to a vector/axis like object that is in the form of a mho_json::array_t
    class MHO_IndexLabelInterface
    {

        protected:

            MHO_IndexLabelInterface():fIndexLabelObjectPtr(nullptr)
            {
                fDummy["index"] = -1; //dummy object for invalid returns, always has an invalid index
            };

            MHO_IndexLabelInterface(const MHO_IndexLabelInterface& copy)
            {
                fIndexLabelObjectPtr = copy.fIndexLabelObjectPtr;
            };

            void SetIndexLabelObject(mho_json* obj){fIndexLabelObjectPtr = obj;}

        public:

            virtual ~MHO_IndexLabelInterface(){};

            std::size_t GetIndexLabelSize() const {return fIndexLabelObjectPtr->size();}

            void ClearIndexLabels()
            {
                //(*fIndexLabelObjectPtr) = mho_json();
                fIndexLabelObjectPtr->clear();
            }

            template< typename XValueType >
            void InsertIndexLabelKeyValue(std::size_t index, const std::string& key, const XValueType& value)
            {
                if(fIndexLabelObjectPtr != nullptr)
                {
                    std::string ikey = index2key(index);
                    if( !(fIndexLabelObjectPtr->contains(ikey) ) )
                    {
                        //no such object, so insert one, make sure it gets an 'index' value
                        (*fIndexLabelObjectPtr).emplace(ikey, fDummy);
                        //(*fIndexLabelObjectPtr).emplace(ikey, fDummy);
                        // (*fIndexLabelObjectPtr)[ ikey ] = fDummy;
                        (*fIndexLabelObjectPtr)[ ikey ]["index"] = index;
                    }
                    //now update
                    mho_json obj;
                    obj[key] = value;
                    (*fIndexLabelObjectPtr)[ikey].update(obj);
                }
                else 
                {
                    msg_error("utilities", "cannot insert key:value pair, index label interface is missing object!" << eom);
                }
            }

            template< typename XValueType >
            bool RetrieveIndexLabelKeyValue(std::size_t index, const std::string& key, XValueType& value) const
            {
                if(fIndexLabelObjectPtr != nullptr)
                {
                    std::string ikey = index2key(index);
                    if( (*fIndexLabelObjectPtr)[ikey].contains(key) )
                    {
                        value = (*fIndexLabelObjectPtr)[ikey][key].get<XValueType>();
                        return true;
                    }
                }
                else 
                {
                    msg_error("utilities", "cannot retrieve key:value pair, index label interface is missing object!" << eom);
                }
                return false;
            }

            //get a reference to the dictionary object associated with this index
            void SetLabelObject(mho_json& obj, std::size_t index)
            {
                if(fIndexLabelObjectPtr != nullptr)
                {
                    if(obj.is_null()){return;}
                    if( !(fIndexLabelObjectPtr->is_object() ) ){ (*fIndexLabelObjectPtr) = mho_json(); } //something blew away our object, reset
                    std::string ikey = index2key(index);
                    if( !(fIndexLabelObjectPtr->contains(ikey) ) )
                    {
                        //no such object, so insert one, make sure it gets an 'index' value
                        (*fIndexLabelObjectPtr).emplace(ikey, fDummy);
                        (*fIndexLabelObjectPtr)[ ikey ]["index"] = index;
                    }
    
                    //make sure the object also contains the index value:
                    obj["index"] = index;
                    (*fIndexLabelObjectPtr)[ikey].update(obj);
                }
                else 
                {
                    msg_error("utilities", "cannot insert label object, index label interface is missing object!" << eom);
                }



                // if( !(fIndexLabelObjectPtr->is_object() ) )
                // {
                //     std::cout<<"this is garbage"<<std::endl;
                //     (*fIndexLabelObjectPtr) = mho_json();
                // }
                // 
                // 
                // if( !(fIndexLabelObjectPtr->contains(ikey) ) )
                // {
                //     //no such object, so insert one, make sure it gets an 'index' value
                //     (*fIndexLabelObjectPtr).emplace(ikey, fDummy);
                //     //(*fIndexLabelObjectPtr).emplace(ikey, fDummy);
                //     // (*fIndexLabelObjectPtr)[ ikey ] = fDummy;
                //     //(*fIndexLabelObjectPtr)[ ikey ]["index"] = index;
                // }
                // 
                // obj["index"] = index;
                // (*fIndexLabelObjectPtr)[ikey].update(obj);

                // //now update
                // mho_json obj;
                // obj[key] = value;
                // 
                // (*fIndexLabelObjectPtr)[ikey].update(obj);


                // 
                if( !(fIndexLabelObjectPtr->is_object() ) )
                {
                    std::cout<<"this is garbage"<<std::endl;
                    (*fIndexLabelObjectPtr) = mho_json();
                }
                // 
                std::string ikey = index2key(index);
                mho_json wrapper;
                obj["index"] = index;
                wrapper[ikey] = obj;
                std::cout<<wrapper.dump(2)<<std::endl;
                std::cout<<fIndexLabelObjectPtr->dump(2)<<std::endl;
                fIndexLabelObjectPtr->update(wrapper);



                // 
                // std::cout<<"obj dump = "<<obj.dump(2)<<std::endl;
                // if(fIndexLabelObjectPtr != nullptr)
                // {
                //     std::string ikey = index2key(index);
                //     if( !(fIndexLabelObjectPtr->contains(ikey) ) )
                //     {
                //         //no such object, so insert one, make sure it gets an 'index' value
                //         (*fIndexLabelObjectPtr)[ikey] = {};
                //         // (*fIndexLabelObjectPtr).emplace(ikey, fDummy);
                //         // (*fIndexLabelObjectPtr).emplace(ikey, fDummy);
                //         // (*fIndexLabelObjectPtr)[ ikey ] = fDummy;
                //         // (*fIndexLabelObjectPtr)[ ikey ]["index"] = index;
                //     }
                // 
                //     //make sure the object also contains the index value:
                //     // obj["index"] = index;
                //     // std::cout<<"index = "<<index<<" ikey = "<<ikey<<std::endl;
                //     std::cout<<"existing obj dump = "<<(*fIndexLabelObjectPtr)[ikey].dump(2)<<std::endl;
                //     (*fIndexLabelObjectPtr)[ikey].update(obj);
                //     std::cout<<"updated existing obj dump = "<<(*fIndexLabelObjectPtr).dump(2)<<std::endl;
                // }
                // else 
                // {
                //     msg_error("utilities", "cannot insert label object, index label interface is missing object!" << eom);
                // }
            }

            //get a reference to the dictionary object associated with this index
            mho_json& GetLabelObject(std::size_t index)
            {
                if(fIndexLabelObjectPtr != nullptr)
                {
                    std::string ikey = index2key(index);
                    return (*fIndexLabelObjectPtr)[ikey];
                }
                else 
                {
                    msg_error("utilities", "cannot retrieve label object, index label interface is missing object!" << eom);
                    return fDummy;
                }
            }

            mho_json GetLabelObject(std::size_t index) const
            {
                if(fIndexLabelObjectPtr != nullptr)
                {
                    std::string ikey = index2key(index);
                    return (*fIndexLabelObjectPtr)[ikey];
                }
                else 
                {
                    msg_error("utilities", "cannot retrieve label object, index label interface is missing object!" << eom);
                    return fDummy;
                }
            }


            //get a vector of indexes which contain a key with the same name
            std::vector< std::size_t > GetMatchingIndexes(std::string& key) const
            {
                std::vector<std::size_t> idx;
                if(fIndexLabelObjectPtr != nullptr)
                {
                    for(std::size_t i=0; i<fIndexLabelObjectPtr->size(); i++)
                    {
                        std::string ikey = index2key(i);
                        if( (*fIndexLabelObjectPtr)[ikey].contains(key) )
                        {
                            idx.push_back( i );
                        }
                    }
                }
                else 
                {
                    msg_error("utilities", "cannot determine matching indexes, index label interface is missing object!" << eom);
                }
                return idx;
            }

            //get a vector of indexes which contain a key with a value which matches the passed value
            template< typename XValueType >
            std::vector< std::size_t > GetMatchingIndexes(std::string& key, const XValueType& value) const
            {
                std::vector<std::size_t> idx;
                if(fIndexLabelObjectPtr != nullptr)
                {
                    for(std::size_t i=0; i<fIndexLabelObjectPtr->size(); i++)
                    {
                        std::string ikey = index2key(i);
                        if( (*fIndexLabelObjectPtr)[ikey].contains(key) )
                        {
                            XValueType v = (*fIndexLabelObjectPtr)[ikey][key].get<XValueType>();
                            if(v == value)
                            {
                                idx.push_back(i);
                            }
                        }
                    }
                }
                else 
                {
                    msg_error("utilities", "cannot determine matching indexes, index label interface is missing object!" << eom);
                }
                return idx;
            }


        private:

            static std::string index2key(const std::size_t& idx) {return std::to_string(idx);}

            mho_json* fIndexLabelObjectPtr; //array of mho_json objects holding key:value pairs
            mho_json fDummy;
    };

    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

} //end namespace


#endif /* end of include guard: MHO_IndexLabelInterface_HH__ */


