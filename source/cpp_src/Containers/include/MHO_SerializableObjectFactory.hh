#ifndef MHO_SerializableObjectFactory_HH__
#define MHO_SerializableObjectFactory_HH__

#include "MHO_Message.hh"
#include "MHO_Serializable.hh"

namespace hops
{

/*!
 *@file  MHO_SerializableObjectFactory.hh
 *@class  MHO_SerializableObjectFactory
 *@author  J. Barrett - barrettj@mit.edu
 *@date Sun Feb 13 19:54:27 2022 -0500
 *@brief
 */

//base class does nothing
class MHO_SerializableObjectFactory
{
    public:
        MHO_SerializableObjectFactory(){};
        virtual ~MHO_SerializableObjectFactory(){};

        virtual MHO_Serializable* Build() { return nullptr; }

        virtual MHO_Serializable* BuildFromFileInterface(MHO_BinaryFileInterface& /*!inter*/) { return nullptr; }

        virtual bool WriteToFileInterface(MHO_BinaryFileInterface& /*!inter*/, const MHO_Serializable* /*!object*/,
                                          const std::string& shortname = "")
        {
            return false;
        };

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

        virtual MHO_Serializable* BuildFromFileInterface(MHO_BinaryFileInterface& inter)
        {
            XClassType* obj = nullptr;
            MHO_FileKey read_key;
            if(inter.IsOpenForRead())
            {
                obj = new XClassType();
                bool ok = inter.Read(*obj, read_key);
                if(ok)
                {
                    return obj;
                }
                else
                {
                    msg_debug("file", "failed to build object from file." << eom);
                    delete obj;
                    obj = nullptr;
                }
            }
            else
            {
                msg_debug("file", "failed to build object from file, interface not open." << eom);
            }
            return obj;
        }

        virtual bool WriteToFileInterface(MHO_BinaryFileInterface& inter, const MHO_Serializable* object,
                                          const std::string& shortname = "")
        {

            const XClassType* obj = dynamic_cast< const XClassType* >(object);
            bool ok = false;
            if(obj != nullptr)
            {
                if(inter.IsOpenForWrite())
                {
                    ok = inter.Write(*obj, shortname);
                }
            }
            if(!ok)
            {
                msg_debug("file", "failed to write object to file." << eom);
            }
            return ok;
        }

    private:
};

} // namespace hops

#endif /*! end of include guard: MHO_SerializableObjectFactory */
