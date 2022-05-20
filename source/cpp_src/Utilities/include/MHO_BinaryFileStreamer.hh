#ifndef MHO_BinaryFileStreamer_HH__
#define MHO_BinaryFileStreamer_HH__

/*
*File: MHO_BinaryFileStreamer.hh
*Class: MHO_BinaryFileStreamer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: variadic template parameter implemenation
* of a gen scatter hierarchy streamer for POD types
*/

#include <stdio.h>
#include <stdint.h>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <complex>

#include "MHO_FileStreamer.hh"

namespace hops
{

//forward declare our binary data streamer (for plain old data types ((POD))
class MHO_BinaryFileStreamer;

//template class for a single-type streamer, generic for most POD types
template<typename XValueType>
class MHO_BinaryFileStreamerSingleType
{
    public:

        MHO_BinaryFileStreamerSingleType(){};
        virtual ~MHO_BinaryFileStreamerSingleType(){};

        friend inline MHO_BinaryFileStreamer& operator>>(MHO_BinaryFileStreamerSingleType<XValueType>& s, XValueType& obj)
        {
            //TODO flag this for big-endian machines
            s.GetStream().read(reinterpret_cast<char*>(&obj), sizeof(XValueType));
            return s.Self();
        }

        friend inline MHO_BinaryFileStreamer& operator<<(MHO_BinaryFileStreamerSingleType<XValueType>& s, const XValueType& obj)
        {
            //TODO flag this for big-endian machines
            s.GetStream().write(reinterpret_cast<const char*>(&obj), sizeof(XValueType));
            s.AddBytesWritten(sizeof(XValueType));
            return s.Self();
        }

        virtual std::fstream& GetStream() = 0;

        virtual void ResetByteCount() = 0;
        virtual void AddBytesWritten(uint64_t) = 0;
        virtual uint64_t GetNBytesWritten() const = 0;

    protected:

        virtual MHO_BinaryFileStreamer& Self() = 0;
};


//specialization for string type (special among the basic POD types because it needs a size parameter)
template<> class MHO_BinaryFileStreamerSingleType<std::string>
{
    public:
        MHO_BinaryFileStreamerSingleType(){};
        virtual ~MHO_BinaryFileStreamerSingleType(){};

        //read in
        friend inline MHO_BinaryFileStreamer& operator>>(MHO_BinaryFileStreamerSingleType<std::string>& s, std::string& obj)
        {
            uint64_t size; //TODO FIXME 64bit int is probably overkill for storing string size...should we change this?
            s.GetStream().read(reinterpret_cast<char*>(&size), sizeof(uint64_t));
            obj.resize(size);
            s.GetStream().read(&obj[0], size);
            return s.Self();
        }

        //write out
        friend inline MHO_BinaryFileStreamer& operator<<(MHO_BinaryFileStreamerSingleType<std::string>& s, const std::string& obj)
        {
            uint64_t size = obj.size(); //TODO FIXME 64bit int is probably overkill for storing string size...should we change this?
            s.GetStream().write(reinterpret_cast<const char*>(&size), sizeof(uint64_t));
            s.GetStream().write(obj.c_str(), size);

            s.AddBytesWritten(sizeof(uint64_t));
            s.AddBytesWritten(size);
            return s.Self();
        }

        virtual std::fstream& GetStream() = 0;

        virtual void ResetByteCount() = 0;
        virtual void AddBytesWritten(uint64_t) = 0;
        virtual uint64_t GetNBytesWritten() const = 0;

    protected:

        virtual MHO_BinaryFileStreamer& Self() = 0;
};

//now declare a multi-type streamer with a variadic template parameter for the types to be streamed
template <typename... XValueTypeS>
class MHO_BinaryFileStreamerMultiType;

//declare the specialization for the base case of the recursion (in which the parameter XValueType is just a single type)
template <typename XValueType>
class MHO_BinaryFileStreamerMultiType< XValueType >: public MHO_BinaryFileStreamerSingleType< XValueType > {};

template<typename XValueType, typename... XValueTypeS >
class MHO_BinaryFileStreamerMultiType< XValueType, XValueTypeS...>:
public MHO_BinaryFileStreamerMultiType< XValueType >,
public MHO_BinaryFileStreamerMultiType< XValueTypeS... >{};

//construct the multi-type streamer for most basic POD types
typedef MHO_BinaryFileStreamerMultiType<
    bool,
    char,
    unsigned char,
    short,
    unsigned short,
    int,
    unsigned int,
    long,
    unsigned long,
    long long,
    unsigned long long,
    float,
    double,
    long double,
    std::complex<float>,
    std::complex<double>,
    std::complex<long double>,
    std::string >
MHO_BinaryFileStreamerBasicTypes;

//now declare the concrete class which does the work for file streams
class MHO_BinaryFileStreamer: public MHO_FileStreamer, public MHO_BinaryFileStreamerBasicTypes
{
    public:

        MHO_BinaryFileStreamer(){fNBytesWritten = 0;};
        virtual ~MHO_BinaryFileStreamer(){};

        virtual void OpenToRead() override;
        //for the time being we only allow writing in 'truncate' mode,
        //at some point we may want to allow for 'append' mode
        virtual void OpenToWrite() override;
        virtual void Close() override;

        virtual std::fstream& GetStream() override { return MHO_FileStreamer::GetStream();}
        virtual const std::fstream& GetStream() const override {return MHO_FileStreamer::GetStream();}

        virtual void ResetByteCount() override {fNBytesWritten = 0;}
        virtual void AddBytesWritten(uint64_t b) override {fNBytesWritten +=b;}
        virtual uint64_t GetNBytesWritten() const override {return fNBytesWritten;};

    protected:

        uint64_t fNBytesWritten;

        MHO_BinaryFileStreamer& Self() override {return *this;}

};


}//end of namespace

#endif /* end of include guard: MHO_BinaryFileStreamer */
