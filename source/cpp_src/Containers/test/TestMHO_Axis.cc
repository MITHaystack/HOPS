#include <algorithm>
#include <complex>
#include <cstdio>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "MHO_Axis.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_BinaryFileStreamer.hh"
#include "MHO_ClassIdentity.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // ----------------------------------------------------------------
    // Test (1): Construction and sizing
    // ----------------------------------------------------------------
    {
        MHO_AxisDouble a(16);
        REQUIRE(a.GetSize() == 16);
        REQUIRE(a.GetDimension(0) == 16);
        REQUIRE(a.GetRank() == 1);
    }

    // ----------------------------------------------------------------
    // Test (2): Default-constructed axis is empty
    // ----------------------------------------------------------------
    {
        MHO_AxisInt a;
        REQUIRE(a.GetSize() == 0);
    }

    // ----------------------------------------------------------------
    // Test (3): Indexing via operator() and at()
    // ----------------------------------------------------------------
    {
        std::size_t N = 16;
        MHO_AxisDouble a(N);
        for(std::size_t i = 0; i < N; i++)
            a(i) = 0.5 + 1.25 * static_cast< double >(i);

        REQUIRE(a(0) == 0.5);
        REQUIRE(a(N - 1) == (0.5 + 1.25 * static_cast< double >(N - 1)));
        REQUIRE(a.at(3) == (0.5 + 1.25 * 3.0));

        // out-of-range should throw
        REQUIRE_THROWS(a.at(N));
    }

    // ----------------------------------------------------------------
    // Test (4): SelectMatchingIndexes (single value)
    // ----------------------------------------------------------------
    {
        MHO_AxisInt a(8);
        a(0) = 1;
        a(1) = 2;
        a(2) = 3;
        a(3) = 2;
        a(4) = 5;
        a(5) = 2;
        a(6) = 7;
        a(7) = 8;

        std::vector< std::size_t > result = a.SelectMatchingIndexes(2);
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == 1);
        REQUIRE(result[1] == 3);
        REQUIRE(result[2] == 5);
    }

    // ----------------------------------------------------------------
    // Test (5): SelectMatchingIndexes (std::set)
    // ----------------------------------------------------------------
    {
        MHO_AxisInt a(8);
        a(0) = 1;
        a(1) = 2;
        a(2) = 3;
        a(3) = 2;
        a(4) = 5;
        a(5) = 2;
        a(6) = 7;
        a(7) = 8;

        std::set< int > search_set;
        search_set.insert(1);
        search_set.insert(7);
        std::vector< std::size_t > result = a.SelectMatchingIndexes(search_set);

        // sort result for order-independent comparison
        std::sort(result.begin(), result.end());
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 6);
    }

    // ----------------------------------------------------------------
    // Test (6): SelectFirstMatchingIndex
    // ----------------------------------------------------------------
    {
        MHO_AxisInt a(8);
        a(0) = 1;
        a(1) = 2;
        a(2) = 3;
        a(3) = 2;
        a(4) = 5;
        a(5) = 2;
        a(6) = 7;
        a(7) = 8;

        std::size_t idx = 0;
        bool ok = a.SelectFirstMatchingIndex(2, idx);
        REQUIRE(ok == true);
        REQUIRE(idx == 1);

        std::size_t idx2 = 0;
        bool ok2 = a.SelectFirstMatchingIndex(99, idx2);
        REQUIRE(ok2 == false);
    }

    // ----------------------------------------------------------------
    // Test (7): Copy construction and independence
    // ----------------------------------------------------------------
    {
        std::size_t N = 16;
        MHO_AxisDouble a(N);
        for(std::size_t i = 0; i < N; i++)
            a(i) = 0.5 + 1.25 * static_cast< double >(i);

        MHO_AxisDouble b(a);
        REQUIRE(b.GetSize() == 16);
        for(std::size_t i = 0; i < N; i++)
            REQUIRE(b(i) == a(i));

        // mutate b, verify a is unchanged
        b(0) = 9999.0;
        REQUIRE(a(0) == 0.5);
    }

    // ----------------------------------------------------------------
    // Test (8): Serialize / deserialize round-trip - double
    // ----------------------------------------------------------------
    {
        std::string filename = "./TestMHO_Axis_d.bin";
        std::size_t N = 16;
        MHO_AxisDouble a(N);
        for(std::size_t i = 0; i < N; i++)
            a(i) = 0.5 + 1.25 * static_cast< double >(i);

        MHO_BinaryFileInterface inter;
        REQUIRE(inter.OpenToWrite(filename) == true);
        std::string shortname = "axis_d";
        inter.Write(a, shortname);
        inter.Close();

        MHO_AxisDouble a2;
        REQUIRE(inter.OpenToRead(filename) == true);
        MHO_FileKey key;
        inter.Read(a2, key);
        inter.Close();

        REQUIRE(a2.GetSize() == 16);
        for(std::size_t i = 0; i < N; i++)
            REQUIRE(a2(i) == a(i));

        std::remove(filename.c_str());
    }

    // ----------------------------------------------------------------
    // Test (9): Serialize / deserialize round-trip - complex<double>
    // ----------------------------------------------------------------
    {
        std::string filename = "./TestMHO_Axis_c.bin";
        std::size_t N = 16;
        MHO_AxisComplexDouble a(N);
        for(std::size_t i = 0; i < N; i++)
            a(i) = std::complex< double >(static_cast< double >(i), -static_cast< double >(i));

        MHO_BinaryFileInterface inter;
        REQUIRE(inter.OpenToWrite(filename) == true);
        std::string shortname = "axis_c";
        inter.Write(a, shortname);
        inter.Close();

        MHO_AxisComplexDouble a2;
        REQUIRE(inter.OpenToRead(filename) == true);
        MHO_FileKey key;
        inter.Read(a2, key);
        inter.Close();

        REQUIRE(a2.GetSize() == 16);
        for(std::size_t i = 0; i < N; i++)
            REQUIRE(a2(i) == a(i));

        std::remove(filename.c_str());
    }

    // ----------------------------------------------------------------
    // Test (10): Serialize / deserialize round-trip - int
    // ----------------------------------------------------------------
    {
        std::string filename = "./TestMHO_Axis_i.bin";
        std::size_t N = 16;
        MHO_AxisInt a(N);
        for(std::size_t i = 0; i < N; i++)
            a(i) = 2 * static_cast< int >(i) - 3;

        MHO_BinaryFileInterface inter;
        REQUIRE(inter.OpenToWrite(filename) == true);
        std::string shortname = "axis_i";
        inter.Write(a, shortname);
        inter.Close();

        MHO_AxisInt a2;
        REQUIRE(inter.OpenToRead(filename) == true);
        MHO_FileKey key;
        inter.Read(a2, key);
        inter.Close();

        REQUIRE(a2.GetSize() == 16);
        for(std::size_t i = 0; i < N; i++)
            REQUIRE(a2(i) == a(i));

        std::remove(filename.c_str());
    }

    // ----------------------------------------------------------------
    // Test (11): Serialize / deserialize round-trip - std::string
    // ----------------------------------------------------------------
    {
        std::string filename = "./TestMHO_Axis_s.bin";
        MHO_AxisString a(4);
        a(0) = "XX";
        a(1) = "XY";
        a(2) = "YX";
        a(3) = "YY";

        MHO_BinaryFileInterface inter;
        REQUIRE(inter.OpenToWrite(filename) == true);
        std::string shortname = "axis_s";
        inter.Write(a, shortname);
        inter.Close();

        MHO_AxisString a2;
        REQUIRE(inter.OpenToRead(filename) == true);
        MHO_FileKey key;
        inter.Read(a2, key);
        inter.Close();

        REQUIRE(a2.GetSize() == 4);
        REQUIRE(a2(0) == "XX");
        REQUIRE(a2(1) == "XY");
        REQUIRE(a2(2) == "YX");
        REQUIRE(a2(3) == "YY");

        std::remove(filename.c_str());
    }

    // ----------------------------------------------------------------
    // Test (12): GetSerializedSize identity (write/read self-consistency)
    // ----------------------------------------------------------------
    {
        std::string filename = "./TestMHO_Axis_sz.bin";
        std::size_t N = 16;
        MHO_AxisDouble a(N);
        for(std::size_t i = 0; i < N; i++)
            a(i) = 0.5 + 1.25 * static_cast< double >(i);

        uint64_t orig_size = a.GetSerializedSize();

        MHO_BinaryFileInterface inter;
        REQUIRE(inter.OpenToWrite(filename) == true);
        std::string shortname = "axis_sz";
        inter.Write(a, shortname);
        inter.Close();

        MHO_AxisDouble a2;
        REQUIRE(inter.OpenToRead(filename) == true);
        MHO_FileKey key;
        inter.Read(a2, key);
        inter.Close();

        REQUIRE(a2.GetSerializedSize() == orig_size);

        std::remove(filename.c_str());
    }

    // ----------------------------------------------------------------
    // Test (13): Frozen class name (compiler-drift guard)
    // ----------------------------------------------------------------
    {
        std::string n = MHO_ClassIdentity::ClassName< MHO_AxisDouble >();
        REQUIRE(n == "MHO_Axis<double>");
    }

    return 0;
}
