#ifndef MHO_FileStreamer_HH__
#define MHO_FileStreamer_HH__

#include "MHO_Message.hh"
#include <fstream>
#include <string>

namespace hops
{

/*!
 *@file MHO_FileStreamer.hh
 *@class MHO_FileStreamer
 *@author J. Barrett - barrettj@mit.edu
 *@date Wed Apr 21 13:40:18 2021 -0400
 *@brief Streams objects to/from a file, uses a 2MB buffer
 */

/**
 * @brief Class MHO_FileStreamer
 */
class MHO_FileStreamer
{
    public:
        MHO_FileStreamer()
        {
            fFileState = FileState::undefined;
            fObjectState = ObjectState::unset;
            //fBufferSize = 64*1024; ///64KB chunk
            fBufferSize = 2 * 1024 * 1024; ///2MB chunk
            fBuffer = new char[fBufferSize];
        };

        virtual ~MHO_FileStreamer() { delete[] fBuffer; };

        /**
         * @brief Setter for filename
         *
         * @param filename New filename to set
         */
        void SetFilename(const std::string filename)
        {
            fFilename = filename;
            fFileState = FileState::undefined;
        }

        /**
         * @brief Getter for filename
         *
         * @return Current filename as a string
         */
        std::string GetFilename() { return fFilename; };

        /**
         * @brief Function OpenToRead - let derived class specify the exact handling of file
         * @note This is a virtual function.
         */
        virtual void OpenToRead() = 0;

        /**
         * @brief Function OpenToAppend
         * @note This is a virtual function.
         */
        virtual void OpenToAppend() = 0;

        /**
         * @brief Function OpenToWrite
         * @note This is a virtual function.
         */
        virtual void OpenToWrite() = 0;

        /**
         * @brief Checks if file is closed.
         * @note This is a virtual function.
         */
        virtual void Close() = 0;

        /**
         * @brief Checks if file is open for writing.
         *
         * @return True if file is open for write, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool IsOpenForWrite()
        {
            if(fFileState == FileState::writeable)
            {
                return true;
            }
            return false;
        }

        /**
         * @brief Checks if a file is open for reading.
         *
         * @return True if file is readable, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool IsOpenForRead()
        {
            if(fFileState == FileState::readable)
            {
                return true;
            }
            return false;
        }

        /**
         * @brief Checks if the file is closed.
         *
         * @return True if the file is closed, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool IsClosed()
        {
            if(fFileState == FileState::closed)
            {
                return true;
            }
            return false;
        }

        /**
         * @brief Setter for object state -
         * if an unrecognized object is encountered in streaming, flag this object
         * by changing the 'object' state to 'unknown'
         * @note This is a virtual function.
         */
        virtual void SetObjectUnknown() { fObjectState = ObjectState::unknown; }

        /**
         * @brief Resets the object state to unset.
         * @note This is a virtual function.
         */
        virtual void ResetObjectState() { fObjectState = ObjectState::unset; };

        /**
         * @brief Checks if object state is unknown.
         *
         * @return True if object state is unknown, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool IsObjectUnknown() { return (fObjectState == ObjectState::unknown); };

        /**
         * @brief Seeks ahead in file by specified number of bytes and updates object state.
         *
         * @param n_bytes Number of bytes to skip ahead.
         * @note This is a virtual function.
         */
        virtual void SkipAhead(size_t n_bytes)
        {
            msg_debug("file", "Seeking ahead by " << n_bytes << " bytes." << eom);
            if(fFileState == FileState::readable)
            {
                //fFile.ignore(n_bytes);
                fObjectState = ObjectState::unset;
                fFile.seekg(n_bytes, std::ios_base::cur);
            }
        }

        /**
         * @brief Getter for stream
         *
         * @return Reference to the std::fstream object
         * @note This is a virtual function.
         */
        virtual std::fstream& GetStream() { return fFile; }

        /**
         * @brief Getter for stream
         *
         * @return Reference to std::fstream object
         * @note This is a virtual function.
         */
        virtual const std::fstream& GetStream() const { return fFile; }

    protected:
        enum FileState
        {
            writeable,
            readable,
            closed,
            undefined
        };

        enum ObjectState
        {
            unset,
            unknown
        };

        std::string fFilename;
        std::fstream fFile;
        FileState fFileState;
        ObjectState fObjectState;

        std::streamsize fBufferSize;
        char* fBuffer;
};

/**
 * @brief Class MHO_ObjectStreamState
 */
template< typename XStreamType > struct MHO_ObjectStreamState
{
        //default behavior on an unknown XStreamType is to doing nothing
        /**
         * @brief Setter for unknown
         *
         * @param !s Parameter description
         * @note This is a static function.
         */
        static void SetUnknown(XStreamType& /*!s*/){};
        /**
         * @brief Resets the state of the XStreamType object to unset.
         *
         * @param !s Reference to the XStreamType object whose state is reset.
         * @note This is a static function.
         */
        static void Reset(XStreamType& /*!s*/){};
};

//NOTE: the use of the keyword 'inline' is necessary for the template specializations
//to satsify the C++ ODR, otherwise you will get a multiple-def error on linking

/**
 * @brief Function MHO_ObjectStreamState<MHO_FileStreamer>::SetUnknown
 *
 * @param s (MHO_FileStreamer&)
 * @return Return value (void MHO_ObjectStreamState< MHO_FileStreamer)
 */
template<> inline void MHO_ObjectStreamState< MHO_FileStreamer >::SetUnknown(MHO_FileStreamer& s)
{
    s.SetObjectUnknown();
}

/**
 * @brief Function MHO_ObjectStreamState<MHO_FileStreamer>::Reset
 *
 * @param s (MHO_FileStreamer&)
 * @return Return value (void MHO_ObjectStreamState< MHO_FileStreamer)
 */
template<> inline void MHO_ObjectStreamState< MHO_FileStreamer >::Reset(MHO_FileStreamer& s)
{
    s.ResetObjectState();
}

} // namespace hops

#endif /*! end of include guard: MHO_FileStreamer */
