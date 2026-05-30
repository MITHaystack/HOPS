#include <complex>
#include <cstdio>
#include <iostream>
#include <string>

#include "MHO_BinaryFileInterface.hh"
#include "MHO_BinaryFileStreamer.hh"
#include "MHO_ClassIdentity.hh"
#include "MHO_FileKey.hh"
#include "MHO_Message.hh"
#include "MHO_ScalarContainer.hh"
#include "MHO_Taggable.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Helper: compute expected serialized size for a given type.
// POD types:   sizeof(MHO_ClassVersion) + sizeof(T) + taggable_size
// std::string: sizeof(MHO_ClassVersion) + sizeof(uint64_t) + len + tag
template< typename T > struct expected_serialized_size
{
        static uint64_t compute(T /*sentinel*/, uint64_t taggable_size)
        {
            return sizeof(MHO_ClassVersion) + sizeof(T) + taggable_size;
        }
};

template<> struct expected_serialized_size< std::string >
{
        static uint64_t compute(const std::string& sentinel, uint64_t taggable_size)
        {
            return sizeof(MHO_ClassVersion) + sizeof(uint64_t) + static_cast< uint64_t >(sentinel.size()) + taggable_size;
        }
};

// C1 - SetValue/GetValue + GetSize
template< typename T > bool test_c1(T sentinel)
{
    MHO_ScalarContainer< T > s;
    s.SetValue(sentinel);
    if(s.GetValue() != sentinel)
        return false;
    if(s.GetSize() != 1)
        return false;
    return true;
}

// C2 - GetData rank-0 accessor consistency
//    (rank-0 GetData() returns XValueType by value)
template< typename T > bool test_c2(T sentinel)
{
    MHO_ScalarContainer< T > s;
    s.SetValue(sentinel);
    T val = s.GetData();
    return (val == sentinel);
}

// C3 - Copy construction (non-const ref ctor) independence
template< typename T > bool test_c3(T sentinel)
{
    MHO_ScalarContainer< T > s;
    s.SetValue(sentinel);
    s.Insert("foo", 1.0);
    s.Insert("bar", std::string("baz"));
    s.Insert("count", 7);

    MHO_ScalarContainer< T > c(s);

    // Mutate original - copy must be independent
    s.SetValue(T());

    if(c.GetValue() != sentinel)
        return false;

    int countVal = 0;
    if(!c.Retrieve("count", countVal))
        return false;
    if(countVal != 7)
        return false;

    return true;
}

// C4 - Serialize/deserialize round-trip + serialized-size formula
template< typename T > bool test_c4(T sentinel, const char* filename)
{
    MHO_ScalarContainer< T > s;
    s.SetValue(sentinel);
    s.Insert("foo", 1.0);
    s.Insert("bar", std::string("baz"));
    s.Insert("count", 7);

    uint64_t reported_size = s.GetSerializedSize();

    MHO_BinaryFileInterface inter;
    if(inter.OpenToWrite(filename) != true)
        return false;
    inter.Write(s, "scalar");
    inter.Close();

    MHO_ScalarContainer< T > w;
    if(inter.OpenToRead(filename) != true)
        return false;
    MHO_FileKey key;
    inter.Read(w, key);
    inter.Close();

    if(w.GetValue() != sentinel)
        return false;

    // Verify tag round-trips
    double fooVal = 0.0;
    if(!w.Retrieve("foo", fooVal))
        return false;
    if(fooVal != 1.0)
        return false;

    std::string barVal;
    if(!w.Retrieve("bar", barVal))
        return false;
    if(barVal != "baz")
        return false;

    // CBOR encodes small positive ints as unsigned (number_unsigned),
    // but MHO_JSONWrapper::Retrieve does a strict JSON-type comparison
    // (signed int literal 0 -> number_integer).  Retrieve as unsigned
    // instead to match the CBOR-encoded type, then convert.
    unsigned int countU = 0;
    if(!w.Retrieve("count", countU))
        return false;
    if(static_cast< int >(countU) != 7)
        return false;

    // Verify serialized size against formula
    // Scope-qualified call to avoid virtual dispatch to derived class
    uint64_t taggable_size = s.MHO_Taggable::GetSerializedSize();
    uint64_t expected_size = expected_serialized_size< T >::compute(sentinel, taggable_size);

    if(reported_size != expected_size)
        return false;
    if(w.GetSerializedSize() != reported_size)
        return false;

    std::remove(filename);
    return true;
}

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    //  double
    {
        double sentinel = 1.23423423e30;
        REQUIRE(test_c1< double >(sentinel));
        REQUIRE(test_c2< double >(sentinel));
        REQUIRE(test_c3< double >(sentinel));
        REQUIRE(test_c4< double >(sentinel, "./TestMHO_ScalarContainer_d.bin"));
    }

    //  int
    {
        int sentinel = -424242;
        REQUIRE(test_c1< int >(sentinel));
        REQUIRE(test_c2< int >(sentinel));
        REQUIRE(test_c3< int >(sentinel));
        REQUIRE(test_c4< int >(sentinel, "./TestMHO_ScalarContainer_i.bin"));
    }

    //  complex<double>
    {
        std::complex< double > sentinel(3.0, -4.0);
        REQUIRE(test_c1< std::complex< double > >(sentinel));
        REQUIRE(test_c2< std::complex< double > >(sentinel));
        REQUIRE(test_c3< std::complex< double > >(sentinel));
        REQUIRE(test_c4< std::complex< double > >(sentinel, "./TestMHO_ScalarContainer_c.bin"));
    }

    //  std::string
    {
        std::string sentinel("scalar_payload_string");
        REQUIRE(test_c1< std::string >(sentinel));
        REQUIRE(test_c2< std::string >(sentinel));
        REQUIRE(test_c3< std::string >(sentinel));
        REQUIRE(test_c4< std::string >(sentinel, "./TestMHO_ScalarContainer_s.bin"));
    }

    //  bool
    {
        bool sentinel = true;
        REQUIRE(test_c1< bool >(sentinel));
        REQUIRE(test_c2< bool >(sentinel));
        REQUIRE(test_c3< bool >(sentinel));
        REQUIRE(test_c4< bool >(sentinel, "./TestMHO_ScalarContainer_b.bin"));
    }

    //
    // C5: Stable/idempotent type UUID (run once with double)
    //
    {
        MHO_ScalarContainer< double > s1;
        MHO_ScalarContainer< double > s2;

        MHO_UUID uuid1a = s1.GetTypeUUID();
        MHO_UUID uuid1b = s1.GetTypeUUID();
        MHO_UUID uuid2 = s2.GetTypeUUID();

        REQUIRE(uuid1a == uuid1b);
        REQUIRE(uuid1a == uuid2);
    }

    return 0;
}
