#ifndef MHO_FileStreamer_HH__
#define MHO_FileStreamer_HH__

/*!
*@file MHO_FileStreamer.hh
*@class MHO_FileStreamer
*@author J. Barrett - barrettj@mit.edu 
*
*@date
*@brief
*/

#include "MHO_Message.hh"
#include <fstream>
#include <string>

namespace hops
{

class MHO_FileStreamer
{
    public:

        MHO_FileStreamer()
        {
            fFileState = FileState::undefined;
            fObjectState = ObjectState::unset;
            //fBufferSize = 64*1024; ///64KB chunk
            fBufferSize = 2*1024*1024; ///2MB chunk
            fBuffer = new char[fBufferSize];
        };

        virtual ~MHO_FileStreamer()
        {
            delete[] fBuffer;
        };

        void SetFilename(const std::string filename)
        {
            fFilename = filename;
            fFileState = FileState::undefined;
        }

        std::string GetFilename(){return fFilename;};

        //let derived class specify the exact handling of file
        virtual void OpenToRead() = 0;
        virtual void OpenToAppend() = 0;
        virtual void OpenToWrite() = 0;
        virtual void Close() = 0;

        virtual bool IsOpenForWrite()
        {
            if(fFileState == FileState::writeable){return true;}
            return false;
        }

        virtual bool IsOpenForRead()
        {
            if(fFileState == FileState::readable){return true;}
            return false;
        }

        virtual bool IsClosed()
        {
            if(fFileState == FileState::closed){return true;}
            return false;
        }

        //if an unrecognized object is encountered in streaming, flag this object
        //by changing the 'object' state
        virtual void SetObjectUnknown(){fObjectState = ObjectState::unknown;}
        virtual void ResetObjectState(){fObjectState = ObjectState::unset;};
        virtual bool IsObjectUnknown(){ return (fObjectState == ObjectState::unknown);};

        virtual void SkipAhead(size_t n_bytes)
        {
            msg_debug("file", "Seeking ahead by " << n_bytes << " bytes." <<eom);
            if(fFileState == FileState::readable)
            {
                //fFile.ignore(n_bytes);
                fObjectState = ObjectState::unset;
                fFile.seekg(n_bytes, std::ios_base::cur);
            }
        }

        virtual std::fstream& GetStream() { return fFile;}
        virtual const std::fstream& GetStream() const {return fFile;}

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



template< typename XStreamType >
struct MHO_ObjectStreamState
{
    //default behavior on an unknown XStreamType is to doing nothing
    static void SetUnknown( XStreamType& /*!s*/){};
    static void Reset( XStreamType& /*!s*/){};
};

//NOTE: the use of the keyword 'inline' is necessary for the template specializations
//to satsify the C++ ODR, otherwise you will get a multiple-def error on linking

template<> inline void
MHO_ObjectStreamState<MHO_FileStreamer>::SetUnknown(MHO_FileStreamer& s)
{
    s.SetObjectUnknown();
}

template<> inline void
MHO_ObjectStreamState<MHO_FileStreamer>::Reset(MHO_FileStreamer& s)
{
    s.ResetObjectState();
}


}//end of hops namespace

#endif /*! end of include guard: MHO_FileStreamer */
