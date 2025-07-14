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
/**
 * @brief Class MHO_SerializableObjectFactory
 */
class MHO_SerializableObjectFactory
{
    public:
        MHO_SerializableObjectFactory(){};
        virtual ~MHO_SerializableObjectFactory(){};

        /**
         * @brief Virtual function to build and return an instance of a MHO_Serializable object
         * 
         * @return Pointer to MHO_Serializable object.
         * @note This is a virtual function.
         */
        virtual MHO_Serializable* Build() { return nullptr; }

        /**
         * @brief Builds an MHO_Serializable object from a given MHO_BinaryFileInterface.
         * 
         * @param !inter Reference to an MHO_BinaryFileInterface object
         * @return Pointer to an MHO_Serializable object, or nullptr if failed
         * @note This is a virtual function.
         */
        virtual MHO_Serializable* BuildFromFileInterface(MHO_BinaryFileInterface& /*!inter*/) { return nullptr; }

        /**
         * @brief Writes an object to a binary file interface using a short name.
         * 
         * @param !inter Reference to MHO_BinaryFileInterface for writing operations
         * @param !object Pointer to const MHO_Serializable object to be written
         * @param shortname Short name associated with the object (default is empty string)
         * @return Boolean indicating success of write operation
         * @note This is a virtual function.
         */
        virtual bool WriteToFileInterface(MHO_BinaryFileInterface& /*!inter*/, const MHO_Serializable* /*!object*/,
                                          const std::string& shortname = "")
        {
            return false;
        };

    private:
};

/**
 * @brief Class MHO_SerializableObjectFactorySpecific
 */
template< typename XClassType > //XClassType must inherit from MHO_Serializable
class MHO_SerializableObjectFactorySpecific: public MHO_SerializableObjectFactory
{
    public:
        MHO_SerializableObjectFactorySpecific(){};
        virtual ~MHO_SerializableObjectFactorySpecific(){};

        /**
         * @brief Virtual function to build and return an instance of an MHO_Serializable which points to the underlying XClassType object
         * 
         * @return Pointer to MHO_Serializable object.
         * @note This is a virtual function.
         */
        virtual MHO_Serializable* Build()
        {
            XClassType* obj = new XClassType();
            return obj;
        }

        /**
         * @brief Builds an MHO_Serializable object from a file interface, which points to the underlying XClassType object
         * 
         * @param inter Reference to an MHO_BinaryFileInterface object.
         * @return Pointer to an MHO_Serializable object or nullptr if failed.
         * @note This is a virtual function.
         */
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

        /**
         * @brief Writes an object to a binary file interface using its short name, with full knowledge of the underlying XClassType
         * 
         * @param inter Reference to MHO_BinaryFileInterface for writing operations
         * @param object Pointer to const MHO_Serializable object to be written
         * @param shortname Short name associated with the object
         * @return Boolean indicating success of write operation
         * @note This is a virtual function.
         */
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
