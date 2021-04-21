#include "MHO_BinaryFileStreamer.hh"
#include "MHO_Message.hh"

namespace hops
{

void
MHO_BinaryFileStreamer::OpenToRead()
{
    //open file for binary rwading
    fFile.open(fFilename.c_str(), std::fstream::in | std::ios::binary);
    if( !fFile.is_open() || !fFile.good() )
    {
        msg_error("file", "Failed to open for reading, file: " << fFilename << eom);
        fFileState = FileState::closed;
    }
    else
    {
        fFileState = FileState::readable;
    }
}

void
MHO_BinaryFileStreamer::OpenToWrite()
{
    //open file for binary writing
    fFile.open(fFilename.c_str(), std::fstream::out | std::ios::binary);

    if( !fFile.is_open() || !fFile.good() )
    {
        msg_error("file", "Failed to open for writing, file: " << fFilename << eom);
        fFileState = FileState::closed;
    }
    else
    {
        fFileState = FileState::writeable;
    }
}

void
MHO_BinaryFileStreamer::Close()
{
    if(fFile.is_open())
    {
        fFile.close();
        fFileState = FileState::closed;
    }
}

}//end of namespace
