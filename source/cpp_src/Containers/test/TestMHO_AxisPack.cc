#include "MHO_Axis.hh"
#include "MHO_AxisPack.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_BinaryFileStreamer.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerTypeTags.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace hops;

static const std::string testFile = "./TestMHO_AxisPack.bin";

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    using baseline_pack = baseline_axis_pack;
    using mbd_dr_pack = mbd_dr_axis_pack;
    using station_pack = station_coord_axis_pack;

    //  Test 1: NAXES exposes the correct count
    {
        REQUIRE(baseline_pack::NAXES::value == 4);
        REQUIRE(MHO_AxisPack< MHO_Axis< int > >::NAXES::value == 1);
        REQUIRE(mbd_dr_pack::NAXES::value == 2);
        REQUIRE(station_pack::NAXES::value == 3);
    }

    //  Test 2: Construction with dim array sets every axis size
    {
        std::size_t dims[4] = {4, 8, 16, 32};
        baseline_pack p(dims);
        REQUIRE(std::get< 0 >(p).GetSize() == 4);
        REQUIRE(std::get< 1 >(p).GetSize() == 8);
        REQUIRE(std::get< 2 >(p).GetSize() == 16);
        REQUIRE(std::get< 3 >(p).GetSize() == 32);
    }

    //  Test 3: Tuple access and assignment
    {
        std::size_t dims[4] = {4, 8, 16, 32};
        baseline_pack p(dims);
        std::get< 0 >(p).at(0) = std::string("XX");
        std::get< 0 >(p).at(1) = std::string("XY");
        std::get< 0 >(p).at(2) = std::string("YX");
        std::get< 0 >(p).at(3) = std::string("YY");
        REQUIRE(std::get< 0 >(p).at(0) == "XX");
        REQUIRE(std::get< 0 >(p).at(3) == "YY");

        for(std::size_t i = 0; i < 32; ++i)
            std::get< 3 >(p).at(i) = static_cast< double >(i) * 0.5;
        REQUIRE(std::fabs(std::get< 3 >(p).at(10) - 5.0) < 1e-12);
    }

    //  Test 4: Assignment operator (operator=) deep-copies all axes
    {
        std::size_t dims[4] = {4, 8, 16, 32};
        baseline_pack p(dims);
        std::get< 0 >(p).at(0) = std::string("XX");
        for(std::size_t i = 0; i < 32; ++i)
            std::get< 3 >(p).at(i) = static_cast< double >(i) * 0.5;

        baseline_pack q;
        q = p;
        REQUIRE(std::get< 0 >(q).GetSize() == std::get< 0 >(p).GetSize());
        REQUIRE(std::get< 3 >(q).GetSize() == std::get< 3 >(p).GetSize());
        REQUIRE(std::get< 0 >(q).at(0) == "XX");
        REQUIRE(std::fabs(std::get< 3 >(q).at(10) - 5.0) < 1e-12);

        // Mutating q should not affect p
        std::get< 0 >(q).at(0) = std::string("ZZ");
        REQUIRE(std::get< 0 >(p).at(0) == "XX");
    }

    //  Test 5: Serialize / deserialize round-trip
    {
        std::size_t dims[4] = {4, 8, 16, 32};
        baseline_pack p(dims);
        std::get< 0 >(p).at(0) = std::string("XX");
        std::get< 0 >(p).at(1) = std::string("XY");
        std::get< 0 >(p).at(2) = std::string("YX");
        std::get< 0 >(p).at(3) = std::string("YY");
        for(std::size_t i = 0; i < 8; ++i)
            std::get< 1 >(p).at(i) = static_cast< double >(i) * 1.1;
        for(std::size_t i = 0; i < 16; ++i)
            std::get< 2 >(p).at(i) = static_cast< double >(i) * 2.2;
        for(std::size_t i = 0; i < 32; ++i)
            std::get< 3 >(p).at(i) = static_cast< double >(i) * 0.5;

        MHO_BinaryFileInterface inter;
        bool status = inter.OpenToWrite(testFile);
        REQUIRE(status == true);
        if(status)
        {
            inter.Write(p, "axis-pack");
            inter.Close();
        }

        baseline_pack q;
        status = inter.OpenToRead(testFile);
        REQUIRE(status == true);
        if(status)
        {
            MHO_FileKey key;
            inter.Read(q, key);
            inter.Close();

            // Verify sizes
            REQUIRE(std::get< 0 >(q).GetSize() == 4);
            REQUIRE(std::get< 1 >(q).GetSize() == 8);
            REQUIRE(std::get< 2 >(q).GetSize() == 16);
            REQUIRE(std::get< 3 >(q).GetSize() == 32);

            // Verify contents
            REQUIRE(std::get< 0 >(q).at(0) == "XX");
            REQUIRE(std::get< 0 >(q).at(3) == "YY");
            REQUIRE(std::fabs(std::get< 1 >(q).at(5) - 5.5) < 1e-12);
            REQUIRE(std::fabs(std::get< 2 >(q).at(10) - 22.0) < 1e-12);
            REQUIRE(std::fabs(std::get< 3 >(q).at(10) - 5.0) < 1e-12);
        }
    }

    //  Test 6: GetSerializedSize matches
    {
        std::size_t dims[4] = {4, 8, 16, 32};
        baseline_pack p(dims);
        for(std::size_t i = 0; i < 4; ++i)
            std::get< 0 >(p).at(i) = std::string("XX");

        uint64_t sizeP = p.GetSerializedSize();

        // Write and read back
        MHO_BinaryFileInterface inter;
        inter.OpenToWrite(testFile);
        inter.Write(p, "axis-pack");
        inter.Close();

        baseline_pack q;
        inter.OpenToRead(testFile);
        MHO_FileKey key;
        inter.Read(q, key);
        inter.Close();

        uint64_t sizeQ = q.GetSerializedSize();
        REQUIRE(sizeP == sizeQ);
        REQUIRE(sizeP > 0);
    }

    //  Test 7: Frozen class-name pinning
    {
        REQUIRE(MHO_ClassIdentity::ClassName< baseline_pack >() ==
                "MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>,MHO_Axis<double>>");
        REQUIRE(MHO_ClassIdentity::ClassName< mbd_dr_pack >() == "MHO_AxisPack<MHO_Axis<double>,MHO_Axis<double>>");
        REQUIRE(MHO_ClassIdentity::ClassName< station_pack >() ==
                "MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<int>>");
    }

    //  Test 8: Resize via construction with new dims
    {
        baseline_pack p;
        // empty pack
        REQUIRE(std::get< 0 >(p).GetSize() == 0);

        std::size_t newDims[4] = {2, 4, 6, 8};
        baseline_pack p2(newDims);
        REQUIRE(std::get< 0 >(p2).GetSize() == 2);
        REQUIRE(std::get< 1 >(p2).GetSize() == 4);
        REQUIRE(std::get< 2 >(p2).GetSize() == 6);
        REQUIRE(std::get< 3 >(p2).GetSize() == 8);
        // p should remain empty
        REQUIRE(std::get< 0 >(p).GetSize() == 0);
    }

    // Clean up temp file
    std::remove(testFile.c_str());
    return 0;
}
