#ifndef MHO_SerializableObjectFactory_HH__
#define MHO_SerializableObjectFactory_HH__

/*
*@file: MHO_SerializableObjectFactory.hh
*@class: MHO_SerializableObjectFactory
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include "MHO_Serializable.hh"


namespace hops
{

//base class does nothing
class MHO_SerializableObjectFactory
{
    public:
        MHO_SerializableObjectFactory(){};
        virtual ~MHO_SerializableObjectFactory(){};

        virtual MHO_Serializable* Build(){return nullptr;}
        virtual MHO_Serializable* BuildFromFileInterface(MHO_BinaryFileInterface& /*inter*/, const MHO_FileKey& /*key*/){return nullptr;}

    private:
};


template< typename XClassType > //XClassType must inherit from MHO_Serializable
class MHO_SerializableObjectFactorySpecific: public MHO_SerializableObjectFactory
{
    public:
        MHO_SerializableObjectFactorySpecific(){};
        virtual ~MHO_SerializableObjectFactorySpecific(){};

        virtual MHO_Serializable* Build()
        {
            XClassType* obj = new XClassType();
            return obj;
        }

        virtual MHO_Serializable* BuildFromFileInterface(MHO_BinaryFileInterface& inter, const MHO_FileKey& key)
        {
            XClassType* obj = new XClassType();
            MHO_FileKey read_key;
            bool ok = inter.Read(*obj, read_key);
            if(ok){return obj;}
            else{ delete obj; return nullptr;}
        }

    private:
};

}//end namespace

#endif /* end of include guard: MHO_SerializableObjectFactory */