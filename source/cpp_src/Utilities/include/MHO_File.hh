#ifndef MHO_File_HH__
#define MHO_File_HH__

/*
*File: MHO_File.hh
*Class: MHO_File
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <stdio.h>
#include <stdint.h>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "MHO_UUID.hh"
#include "MHO_UUIDGenerator.hh"
#include "MHO_ObjectFileKey.hh"
#include "MHO_BinaryFileStreamer.hh"

namespace hops
{


class MHO_File
{
    public:

        MHO_File();
        virtual ~MHO_File();

        void SetFilename(std::string filename);
        void GetFilename() const {return fFilename;}

        void template< XWriteType > Write(const WriteType& obj);


    private:


        std::string fFilename;
        MHO_BinaryFileStreamer fStreamer;

        MHO_UUIDGenerator fUUIDGenerator;

};



void template< XWriteType >
MHO_File::Write(const WriteType& obj)
{

    MHO_FileKey key;

    key.fUUID = Generator.GenerateUUID();




}


}


#endif /* end of include guard: MHO_File */
