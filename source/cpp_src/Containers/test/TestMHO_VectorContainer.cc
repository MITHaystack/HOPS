#include <complex>
#include <cstdio>
#include <iostream>
#include <string>

#include "MHO_BinaryFileInterface.hh"
#include "MHO_BinaryFileStreamer.hh"
#include "MHO_ClassIdentity.hh"
#include "MHO_FileKey.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"
#include "MHO_VectorContainer.hh"

using namespace hops;

// Fill rule: produce element value for index i

static inline double make_value(double, size_t i)
{
    return 0.5 * static_cast< double >(i) - 3.25;
}

static inline int make_value(int, size_t i)
{
    return static_cast< int >(i) - 7;
}

static inline std::complex< double > make_value(std::complex< double >, size_t i)
{
    return std::complex< double >(0.5 * static_cast< double >(i), -0.25 * static_cast< double >(i) + 1.0);
}

static inline std::string make_value(std::string, size_t i)
{
    return "elem_" + std::to_string(i);
}

// Helper: compute expected serialized size for POD element types

template< typename T > struct expected_serialized_size
{
        static uint64_t compute(const MHO_VectorContainer< T >& v, uint64_t taggable_size)
        {
            (void)v;
            return sizeof(MHO_ClassVersion) + taggable_size + sizeof(uint64_t) + v.GetSize() * sizeof(T);
        }
};

// std::string specialization: per-string [sizeof(uint64_t) + bytes]
template<> struct expected_serialized_size< std::string >
{
        static uint64_t compute(const MHO_VectorContainer< std::string >& v, uint64_t taggable_size)
        {
            uint64_t total = sizeof(MHO_ClassVersion) + taggable_size + sizeof(uint64_t);
            std::size_t dsize = v.GetSize();
            auto data_ptr = v.GetData();
            for(std::size_t i = 0; i < dsize; i++)
            {
                total += sizeof(uint64_t);
                total += data_ptr[i].size();
            }
            return total;
        }
};

// Helper: attach the standard tag fixture to a container

template< typename T > static void attach_tags(MHO_VectorContainer< T >& v)
{
    v.Insert("gain", 1.5);
    v.Insert("station", std::string("Gs"));
    v.Insert("nchan", 32);
}

// Helper: verify tags on a container (pre-serialization, int type)

template< typename T > static bool verify_tags_inmem(const MHO_VectorContainer< T >& v)
{
    double gainVal = 0.0;
    if(!v.Retrieve("gain", gainVal))
        return false;
    if(gainVal != 1.5)
        return false;

    std::string stationVal;
    if(!v.Retrieve("station", stationVal))
        return false;
    if(stationVal != "Gs")
        return false;

    int nchanVal = 0;
    if(!v.Retrieve("nchan", nchanVal))
        return false;
    if(nchanVal != 32)
        return false;

    return true;
}

// Helper: verify tags after serialization round-trip
// (CBOR may encode small positive ints as unsigned)

template< typename T > static bool verify_tags_roundtrip(const MHO_VectorContainer< T >& v)
{
    double gainVal = 0.0;
    if(!v.Retrieve("gain", gainVal))
        return false;
    if(gainVal != 1.5)
        return false;

    std::string stationVal;
    if(!v.Retrieve("station", stationVal))
        return false;
    if(stationVal != "Gs")
        return false;

    // CBOR encodes small positive ints as unsigned (number_unsigned),
    // but MHO_JSONWrapper::Retrieve does strict type comparison.
    unsigned int nchanU = 0;
    if(!v.Retrieve("nchan", nchanU))
        return false;
    if(static_cast< int >(nchanU) != 32)
        return false;

    return true;
}

// Frozen class-name strings for UUID verification

static const std::string frozen_name_double = "MHO_VectorContainer<double>";
static const std::string frozen_name_int = "MHO_VectorContainer<int>";
static const std::string frozen_name_complex = "MHO_VectorContainer<std::complex<double>>";

// C1 - Construction & sizing

template< typename T > bool test_c1()
{
    MHO_VectorContainer< T > v(100);
    if(v.GetSize() != 100)
        return false;
    if(v.GetDimension(0) != 100)
        return false;
    if(v.GetRank() != 1)
        return false;
    return true;
}

// C2 - Indexing read/write round-trip in memory

template< typename T > bool test_c2()
{
    MHO_VectorContainer< T > v(100);
    // Fill via operator()
    for(std::size_t i = 0; i < v.GetSize(); i++)
        v(i) = make_value(T(), i);

    // Read back via operator() and operator[]
    for(std::size_t i = 0; i < v.GetSize(); i++)
    {
        T expected = make_value(T(), i);
        if(v(i) != expected)
            return false;
        if(v[i] != expected)
            return false;
    }
    return true;
}

// C3 - Resize semantics

template< typename T > bool test_c3()
{
    MHO_VectorContainer< T > v(100);
    std::size_t new_dim[] = {10};
    v.Resize(new_dim);
    if(v.GetSize() != 10)
        return false;

    new_dim[0] = 250;
    v.Resize(new_dim);
    if(v.GetSize() != 250)
        return false;

    return true;
}

// C4 - Copy / Clone deep-copy independence

template< typename T > bool test_c4()
{
    MHO_VectorContainer< T > v(100);
    for(std::size_t i = 0; i < v.GetSize(); i++)
        v(i) = make_value(T(), i);
    attach_tags(v);

    MHO_VectorContainer< T >* c = v.Clone();

    // Save original value at index 0 before mutation
    T orig_val = make_value(T(), 0);

    // Mutate original - clone must be independent
    v(0) = make_value(T(), 50);

    if(c->GetSize() != v.GetSize())
        return false;
    for(std::size_t i = 0; i < c->GetSize(); i++)
    {
        T expected = make_value(T(), i);
        if((*c)(i) != expected)
            return false;
    }
    // Clone index 0 must still be the original fill value
    if((*c)(0) != orig_val)
        return false;

    // Tags must be copied
    if(!verify_tags_inmem(*c))
        return false;

    delete c;
    return true;
}

// C5 - Serialize / deserialize round-trip + class-name + serialized
//       size formula

template< typename T > bool test_c5(const char* filename, const std::string* frozen_name)
{
    std::size_t N = 100;
    MHO_VectorContainer< T > v(N);
    for(std::size_t i = 0; i < v.GetSize(); i++)
        v(i) = make_value(T(), i);
    attach_tags(v);

    // --- Frozen class name check ---
    std::string name = MHO_ClassIdentity::ClassName(v);
    if(frozen_name != NULL)
    {
        if(name != *frozen_name)
            return false;
    }
    else
    {
        // For types without a frozen name, just ensure non-empty
        if(name.empty())
            return false;
    }

    // --- Serialized size check ---
    uint64_t reported_size = v.GetSerializedSize();
    // Scope-qualified call to avoid virtual dispatch to derived class
    uint64_t taggable_size = v.MHO_Taggable::GetSerializedSize();
    uint64_t expected_size = expected_serialized_size< T >::compute(v, taggable_size);
    if(reported_size != expected_size)
        return false;

    // --- Write ---
    MHO_BinaryFileInterface inter;
    if(inter.OpenToWrite(filename) != true)
        return false;
    inter.Write(v, "vec");
    inter.Close();

    // --- Read into fresh container ---
    MHO_VectorContainer< T > w;
    if(inter.OpenToRead(filename) != true)
        return false;
    MHO_FileKey key;
    inter.Read(w, key);
    inter.Close();

    // --- Size match ---
    if(w.GetSize() != v.GetSize())
        return false;

    // --- Element-by-element comparison ---
    for(std::size_t i = 0; i < v.GetSize(); i++)
    {
        if(w(i) != v(i))
            return false;
    }

    // --- Tag round-trip ---
    if(!verify_tags_roundtrip(w))
        return false;

    // --- Read-back container has same serialized size ---
    if(w.GetSerializedSize() != reported_size)
        return false;

    return true;
}

// C6 - Zero-size boundary (double only)

static bool test_c6(const char* filename)
{
    std::size_t zero_dim[] = {0};
    MHO_VectorContainer< double > z(zero_dim);
    if(z.GetSize() != 0)
        return false;

    MHO_BinaryFileInterface inter;
    if(inter.OpenToWrite(filename) != true)
        return false;
    inter.Write(z, "zero");
    inter.Close();

    MHO_VectorContainer< double > w;
    if(inter.OpenToRead(filename) != true)
        return false;
    MHO_FileKey key;
    inter.Read(w, key);
    inter.Close();

    if(w.GetSize() != 0)
        return false;
    return true;
}

// C7 - UUID stability / idempotency (double only)

static bool test_c7()
{
    MHO_VectorContainer< double > a;
    MHO_VectorContainer< double > b;

    MHO_UUID uuid1a = a.GetTypeUUID();
    MHO_UUID uuid1b = a.GetTypeUUID();
    MHO_UUID uuid2 = b.GetTypeUUID();

    if(uuid1a != uuid1b)
        return false;
    if(uuid1a != uuid2)
        return false;
    return true;
}

// Run all typed cases for element type T (returns 0 on success)

template< typename T > static int run_vector_case(const char* filename, const std::string* frozen_name)
{
    if(!test_c1< T >())
        return 1;
    if(!test_c2< T >())
        return 1;
    if(!test_c3< T >())
        return 1;
    if(!test_c4< T >())
        return 1;
    if(!test_c5< T >(filename, frozen_name))
        return 1;
    return 0;
}

// Main driver

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    //  double
    {
        REQUIRE(run_vector_case< double >("./TestMHO_VectorContainer_d.bin", &frozen_name_double) == 0);
    }

    //  int
    {
        REQUIRE(run_vector_case< int >("./TestMHO_VectorContainer_i.bin", &frozen_name_int) == 0);
    }

    //  complex<double>
    {
        REQUIRE(run_vector_case< std::complex< double > >("./TestMHO_VectorContainer_c.bin", &frozen_name_complex) == 0);
    }

    //  std::string (no frozen name pinned)
    {
        REQUIRE(run_vector_case< std::string >("./TestMHO_VectorContainer_s.bin", NULL) == 0);
    }

    //
    // C6: Zero-size boundary (double only)
    //
    {
        REQUIRE(test_c6("./TestMHO_VectorContainer_zero.bin"));
    }

    //
    // C7: UUID stability (double only)
    //
    {
        REQUIRE(test_c7());
    }

    //
    // Cleanup scratch files
    //
    std::remove("./TestMHO_VectorContainer_d.bin");
    std::remove("./TestMHO_VectorContainer_i.bin");
    std::remove("./TestMHO_VectorContainer_c.bin");
    std::remove("./TestMHO_VectorContainer_s.bin");
    std::remove("./TestMHO_VectorContainer_zero.bin");
    return 0;
}
