#ifndef MHO_BinaryFileStreamer_HH__
#define MHO_BinaryFileStreamer_HH__

#include <complex>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "MHO_FileStreamer.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Types.hh"

namespace hops
{

/*!
 *@file MHO_BinaryFileStreamer.hh
 *@class MHO_BinaryFileStreamer
 *@author J. Barrett - barrettj@mit.edu
 *@date Wed Apr 21 13:40:18 2021 -0400
 *@brief variadic template parameter implemenation
 * of a gen scatter hierarchy streamer for POD and JSON types to a file stream
 */

//forward declare our binary data streamer (for plain old data types ((POD))
/**
 * @brief Class MHO_BinaryFileStreamer
 */
class MHO_BinaryFileStreamer;

//template class for a single-type streamer, generic for most POD types
/**
 * @brief Class MHO_BinaryFileStreamerSingleType
 */
template< typename XValueType > class MHO_BinaryFileStreamerSingleType
{
    public:
        MHO_BinaryFileStreamerSingleType(){};
        virtual ~MHO_BinaryFileStreamerSingleType(){};

        friend inline MHO_BinaryFileStreamer& operator>>(MHO_BinaryFileStreamerSingleType< XValueType >& s, XValueType& obj)
        {
            //TODO flag this for big-endian machines
            s.GetStream().read(reinterpret_cast< char* >(&obj), sizeof(XValueType));
            return s.Self();
        }

        friend inline MHO_BinaryFileStreamer& operator<<(MHO_BinaryFileStreamerSingleType< XValueType >& s,
                                                         const XValueType& obj)
        {
            //TODO flag this for big-endian machines
            s.GetStream().write(reinterpret_cast< const char* >(&obj), sizeof(XValueType));
            s.AddBytesWritten(sizeof(XValueType));
            return s.Self();
        }

        /**
         * @brief Getter for stream
         * 
         * @return Reference to std::fstream object.
         * @note This is a virtual function.
         */
        virtual std::fstream& GetStream() = 0;

        /**
         * @brief Resets the byte count to zero.
         * @note This is a virtual function.
         */
        virtual void ResetByteCount() = 0;
        /**
         * @brief Increments the total bytes written by the provided value.
         * 
         * @param param1 Value to add to the total bytes written.
         * @note This is a virtual function.
         */
        virtual void AddBytesWritten(uint64_t) = 0;
        /**
         * @brief Getter for nbytes written
         * 
         * @return Number of bytes written as uint64_t.
         * @note This is a virtual function.
         */
        virtual uint64_t GetNBytesWritten() const = 0;

    protected:
        /**
         * @brief Returns a reference to the current instance of MHO_BinaryFileStreamer.
         * 
         * @return MHO_BinaryFileStreamer&: Reference to the current instance
         * @note This is a virtual function.
         */
        virtual MHO_BinaryFileStreamer& Self() = 0;
};

//specialization for string type (special among the basic POD types because it needs a size parameter)
/**
 * @brief Class MHO_BinaryFileStreamerSingleType<std::string>
 */
template<> class MHO_BinaryFileStreamerSingleType< std::string >
{
    public:
        MHO_BinaryFileStreamerSingleType(){};
        virtual ~MHO_BinaryFileStreamerSingleType(){};

        //read in
        friend inline MHO_BinaryFileStreamer& operator>>(MHO_BinaryFileStreamerSingleType< std::string >& s, std::string& obj)
        {
            uint64_t size;
            s.GetStream().read(reinterpret_cast< char* >(&size), sizeof(uint64_t));
            obj.resize(size);
            s.GetStream().read(&obj[0], size);
            return s.Self();
        }

        //write out
        friend inline MHO_BinaryFileStreamer& operator<<(MHO_BinaryFileStreamerSingleType< std::string >& s,
                                                         const std::string& obj)
        {
            uint64_t size = obj.size();
            s.GetStream().write(reinterpret_cast< const char* >(&size), sizeof(uint64_t));
            s.GetStream().write(obj.c_str(), size);

            s.AddBytesWritten(sizeof(uint64_t));
            s.AddBytesWritten(size);
            return s.Self();
        }

        /**
         * @brief Getter for stream
         * 
         * @return Reference to std::fstream object
         * @note This is a virtual function.
         */
        virtual std::fstream& GetStream() = 0;

        /**
         * @brief Resets the byte count to zero.
         * @note This is a virtual function.
         */
        virtual void ResetByteCount() = 0;
        /**
         * @brief Increments fNBytesWritten by param1.
         * 
         * @param param1 Number of bytes to add to fNBytesWritten.
         * @note This is a virtual function.
         */
        virtual void AddBytesWritten(uint64_t) = 0;
        /**
         * @brief Getter for nbytes written
         * 
         * @return The total number of bytes written as a uint64_t.
         * @note This is a virtual function.
         */
        virtual uint64_t GetNBytesWritten() const = 0;

    protected:
        /**
         * @brief Returns a reference to the current instance of MHO_BinaryFileStreamer.
         * 
         * @return MHO_BinaryFileStreamer&: Reference to the current instance
         * @note This is a virtual function.
         */
        virtual MHO_BinaryFileStreamer& Self() = 0;
};

//specialization for mho_json type (special, because it needs to be encoded and gets a size parameter)
/**
 * @brief Class MHO_BinaryFileStreamerSingleType<mho_json>
 */
template<> class MHO_BinaryFileStreamerSingleType< mho_json >
{
    public:
        MHO_BinaryFileStreamerSingleType(){};
        virtual ~MHO_BinaryFileStreamerSingleType(){};

        //read in
        friend inline MHO_BinaryFileStreamer& operator>>(MHO_BinaryFileStreamerSingleType< mho_json >& s, mho_json& obj)
        {
            uint64_t size;
            uint64_t encoding;
            s.GetStream().read(reinterpret_cast< char* >(&size), sizeof(uint64_t));
            s.GetStream().read(reinterpret_cast< char* >(&encoding), sizeof(uint64_t));
            std::vector< std::uint8_t > data;
            data.resize(size);
            s.GetStream().read(reinterpret_cast< char* >(&(data[0])), size);
            //TODO FIXME - Add the ability to support other JSON encodings besides CBOR
            //now decode from CBOR
            if(encoding == 0)
            {
                obj = mho_json::from_cbor(data);
            }
            return s.Self();
        }

        //write out
        friend inline MHO_BinaryFileStreamer& operator<<(MHO_BinaryFileStreamerSingleType< mho_json >& s, const mho_json& obj)
        {

            std::vector< std::uint8_t > data = mho_json::to_cbor(obj);
            uint64_t size = data.size();
            //must encode to CBOR
            //TODO FIXME - Add the ability to support other JSON encodings
            uint64_t encoding = 0; //CBOR is 0
            s.GetStream().write(reinterpret_cast< const char* >(&size), sizeof(uint64_t));
            s.GetStream().write(reinterpret_cast< const char* >(&encoding), sizeof(uint64_t));
            s.GetStream().write(reinterpret_cast< const char* >(&(data[0])), size);
            s.AddBytesWritten(sizeof(uint64_t));
            s.AddBytesWritten(sizeof(uint64_t));
            s.AddBytesWritten(size);
            return s.Self();
        }

        /**
         * @brief Getter for stream
         * 
         * @return Reference to std::fstream object.
         * @note This is a virtual function.
         */
        virtual std::fstream& GetStream() = 0;

        /**
         * @brief Resets byte count to zero.
         * @note This is a virtual function.
         */
        virtual void ResetByteCount() = 0;
        /**
         * @brief Increments fNBytesWritten by param1.
         * 
         * @param param1 Number of bytes to add to fNBytesWritten.
         * @note This is a virtual function.
         */
        virtual void AddBytesWritten(uint64_t) = 0;
        /**
         * @brief Getter for nbytes written
         * 
         * @return The total number of bytes written as a uint64_t.
         * @note This is a virtual function.
         */
        virtual uint64_t GetNBytesWritten() const = 0;

    protected:
        /**
         * @brief Returns a reference to the current instance of MHO_BinaryFileStreamer.
         * 
         * @return MHO_BinaryFileStreamer&: Reference to the current instance
         * @note This is a virtual function.
         */
        virtual MHO_BinaryFileStreamer& Self() = 0;
};

//now declare a multi-type streamer with a variadic template parameter for the types to be streamed
/**
 * @brief Class MHO_BinaryFileStreamerMultiType
 */
template< typename... XValueTypeS > class MHO_BinaryFileStreamerMultiType;

//declare the specialization for the base case of the recursion (in which the parameter XValueType is just a single type)
/**
 * @brief Class MHO_BinaryFileStreamerMultiType<XValueType>
 * 
 * @tparam XValueTypeS Template parameter XValueTypeS
 */
template< typename XValueType >
class MHO_BinaryFileStreamerMultiType< XValueType >: public MHO_BinaryFileStreamerSingleType< XValueType >
{};

/**
 * @brief Class MHO_BinaryFileStreamerMultiType<XValueType, XValueTypeS...>
 */
template< typename XValueType, typename... XValueTypeS >
class MHO_BinaryFileStreamerMultiType< XValueType, XValueTypeS... >: public MHO_BinaryFileStreamerMultiType< XValueType >,
                                                                     public MHO_BinaryFileStreamerMultiType< XValueTypeS... >
{};

//construct the multi-type streamer for most basic POD types and some vectors
typedef MHO_BinaryFileStreamerMultiType< bool, char, unsigned char, short, unsigned short, int, unsigned int, long,
                                         unsigned long, long long, unsigned long long, float, double, long double,
                                         std::complex< float >, std::complex< double >, std::complex< long double >,
                                         std::string, mho_json >
    MHO_BinaryFileStreamerBasicTypes;

//now declare the concrete class which does the work for file streams
/**
 * @brief Class MHO_BinaryFileStreamer
 */
class MHO_BinaryFileStreamer: public MHO_FileStreamer, public MHO_BinaryFileStreamerBasicTypes
{
    public:
        MHO_BinaryFileStreamer() { fNBytesWritten = 0; };

        virtual ~MHO_BinaryFileStreamer(){};

        /**
         * @brief Opens a file for binary reading and sets appropriate state.
         * @note This is a virtual function.
         */
        virtual void OpenToRead() override;
        /**
         * @brief Opens a file for binary writing and sets appropriate state.
         * @note This is a virtual function.
         */
        virtual void OpenToWrite() override;
        /**
         * @brief Opens a binary file for appending and sets appropriate file state.
         * @note This is a virtual function.
         */
        virtual void OpenToAppend() override;
        /**
         * @brief Closes the open file if it exists and sets the file state to closed.
         * @note This is a virtual function.
         */
        virtual void Close() override;

        /**
         * @brief Getter for stream
         * 
         * @return Reference to std::fstream& representing the current file stream.
         * @note This is a virtual function.
         */
        virtual std::fstream& GetStream() override { return MHO_FileStreamer::GetStream(); }

        /**
         * @brief Getter for stream
         * 
         * @return Reference to std::fstream object.
         * @note This is a virtual function.
         */
        virtual const std::fstream& GetStream() const override { return MHO_FileStreamer::GetStream(); }

        /**
         * @brief Resets byte count to zero.
         * @note This is a virtual function.
         */
        virtual void ResetByteCount() override { fNBytesWritten = 0; }

        /**
         * @brief Increments the total bytes written by the given amount.
         * 
         * @param b The number of bytes to add to the total.
         * @note This is a virtual function.
         */
        virtual void AddBytesWritten(uint64_t b) override { fNBytesWritten += b; }

        /**
         * @brief Getter for nbytes written
         * 
         * @return The total number of bytes written as a uint64_t.
         * @note This is a virtual function.
         */
        virtual uint64_t GetNBytesWritten() const override { return fNBytesWritten; };

    protected:
        uint64_t fNBytesWritten;

        /**
         * @brief Returns a reference to the current instance of MHO_BinaryFileStreamer.
         * 
         * @return MHO_BinaryFileStreamer&: Reference to the current instance
         */
        MHO_BinaryFileStreamer& Self() override { return *this; }
};

} // namespace hops

#endif /*! end of include guard: MHO_BinaryFileStreamer */
