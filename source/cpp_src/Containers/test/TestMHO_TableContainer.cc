#include <complex>
#include <cstdio>
#include <iostream>
#include <string>
#include <type_traits>

#include "MHO_Axis.hh"
#include "MHO_AxisPack.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_BinaryFileStreamer.hh"
#include "MHO_ClassIdentity.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_FileKey.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Fixed axis-pack type used for T1-T6
typedef MHO_AxisPack< MHO_Axis< double >, MHO_Axis< double >, MHO_Axis< std::string > > test_axis_pack;
const std::size_t DIM0 = 3;
const std::size_t DIM1 = 4;
const std::size_t DIM2 = 2;

//  Type-dispatch helpers (C++11 SFINAE)

// MakeZero
template< typename T > typename std::enable_if< !std::is_same< T, std::complex< double > >::value, T >::type make_zero()
{
    return T(0);
}

template< typename T > typename std::enable_if< std::is_same< T, std::complex< double > >::value, T >::type make_zero()
{
    return std::complex< double >(0.0, 0.0);
}

// MakeValue(fill) - returns T(fill) or std::complex<double>(fill, -fill)
template< typename T >
typename std::enable_if< !std::is_same< T, std::complex< double > >::value, T >::type make_value(double fill)
{
    return static_cast< T >(fill);
}

template< typename T >
typename std::enable_if< std::is_same< T, std::complex< double > >::value, T >::type make_value(double fill)
{
    return std::complex< double >(fill, -fill);
}

// Marker values
template< typename T > typename std::enable_if< !std::is_same< T, std::complex< double > >::value, T >::type make_marker1()
{
    return T(7777);
}

template< typename T > typename std::enable_if< std::is_same< T, std::complex< double > >::value, T >::type make_marker1()
{
    return std::complex< double >(7777.0, -7777.0);
}

template< typename T > typename std::enable_if< !std::is_same< T, std::complex< double > >::value, T >::type make_marker2()
{
    return T(8888);
}

template< typename T > typename std::enable_if< std::is_same< T, std::complex< double > >::value, T >::type make_marker2()
{
    return std::complex< double >(8888.0, -8888.0);
}

template< typename T > typename std::enable_if< !std::is_same< T, std::complex< double > >::value, T >::type make_mutated()
{
    return T(99999);
}

template< typename T > typename std::enable_if< std::is_same< T, std::complex< double > >::value, T >::type make_mutated()
{
    return std::complex< double >(99999.0, -99999.0);
}

//  Per-type test runner (returns number of failures)

template< typename T > int run_table_case()
{
    int failures = 0;

#define RQ(expr)                                                                                                               \
    do                                                                                                                         \
    {                                                                                                                          \
        if(!(expr))                                                                                                            \
        {                                                                                                                      \
            std::cerr << "[FAIL] " << #expr << " (T=" << typeid(T).name() << " @ line " << __LINE__ << " in " << __FILE__      \
                      << ")" << std::endl;                                                                                     \
            ++failures;                                                                                                        \
        }                                                                                                                      \
    }                                                                                                                          \
    while(0)

    //
    // T1. Construction, sizing, rank
    //
    {
        std::size_t dim[] = {DIM0, DIM1, DIM2};
        MHO_TableContainer< T, test_axis_pack > table(dim);

        RQ(table.GetDimension(0) == DIM0);
        RQ(table.GetDimension(1) == DIM1);
        RQ(table.GetDimension(2) == DIM2);
        RQ(table.GetRank() == 3);
        RQ(table.GetSize() == DIM0 * DIM1 * DIM2);
    }

    //
    // T2. Indexing consistency (expanded)
    //
    {
        std::size_t dim[] = {DIM0, DIM1, DIM2};
        MHO_TableContainer< T, test_axis_pack > table(dim);

        for(std::size_t i = 0; i < DIM0; i++)
        {
            for(std::size_t j = 0; j < DIM1; j++)
            {
                for(std::size_t k = 0; k < DIM2; k++)
                {
                    double fill = static_cast< double >(1000 * i + 100 * j + 10 * k);
                    table(i, j, k) = make_value< T >(fill);
                }
            }
        }

        for(std::size_t i = 0; i < DIM0; i++)
        {
            for(std::size_t j = 0; j < DIM1; j++)
            {
                for(std::size_t k = 0; k < DIM2; k++)
                {
                    std::size_t flat = ((i * DIM1) + j) * DIM2 + k;

                    T& via_paren = table(i, j, k);
                    T& via_bracket = table[flat];
                    T& via_at = table.at(i, j, k);
                    T& via_data = *(table.GetData() + flat);

                    RQ(via_paren == via_bracket);
                    RQ(via_paren == via_at);
                    RQ(via_paren == via_data);
                }
            }
        }
    }

    //
    // T3. at() bounds checking - out of range should throw
    //
    {
        std::size_t dim[] = {DIM0, DIM1, DIM2};
        MHO_TableContainer< T, test_axis_pack > table(dim);

        bool threw = false;
        try
        {
            table.at(3, 0, 0);
        }
        catch(const std::out_of_range&)
        {
            threw = true;
        }
        catch(...)
        {}
        RQ(threw == true);
    }

    //
    // T4. SubView / SliceView are aliasing views
    //
    {
        std::size_t dim[] = {DIM0, DIM1, DIM2};
        MHO_TableContainer< T, test_axis_pack > table(dim);

        T zero_val = make_zero< T >();
        for(std::size_t i = 0; i < DIM0; i++)
        {
            for(std::size_t j = 0; j < DIM1; j++)
            {
                for(std::size_t k = 0; k < DIM2; k++)
                {
                    table(i, j, k) = zero_val;
                }
            }
        }

        auto sub = table.SubView(1);
        T marker1 = make_marker1< T >();
        sub(0, 0) = marker1;
        RQ(table(1, 0, 0) == marker1);

        auto sl = table.SliceView(":", ":", ":");
        T marker2 = make_marker2< T >();
        sl(2, 0, 1) = marker2;
        RQ(table(2, 0, 1) == marker2);
    }

    //
    // T5. Resize resizes data AND axes
    //
    {
        std::size_t dim[] = {DIM0, DIM1, DIM2};
        MHO_TableContainer< T, test_axis_pack > table(dim);

        std::size_t new_dim[] = {5, 4, 2};
        table.Resize(new_dim);

        RQ(table.GetSize() == 5 * 4 * 2);
        RQ(std::get< 0 >(table).GetSize() == 5);
        RQ(std::get< 1 >(table).GetSize() == 4);
        RQ(std::get< 2 >(table).GetSize() == 2);
    }

    //
    // T6. Clone deep copy + serialize round-trip
    //
    {
        std::size_t dim[] = {DIM0, DIM1, DIM2};
        MHO_TableContainer< T, test_axis_pack > table(dim);

        for(std::size_t i = 0; i < DIM0; i++)
        {
            for(std::size_t j = 0; j < DIM1; j++)
            {
                for(std::size_t k = 0; k < DIM2; k++)
                {
                    double fill = static_cast< double >(1000 * i + 100 * j + 10 * k);
                    table(i, j, k) = make_value< T >(fill);
                }
            }
        }

        for(std::size_t i = 0; i < DIM0; i++)
            std::get< 0 >(table)(i) = 0.1 * static_cast< double >(i);
        for(std::size_t j = 0; j < DIM1; j++)
            std::get< 1 >(table)(j) = 10.0 + static_cast< double >(j);
        std::get< 2 >(table)(0) = "r";
        std::get< 2 >(table)(1) = "g";

        auto* axis0 = &std::get< 0 >(table);
        axis0->Insert(std::string("units"), std::string("Hz"));

        table.Insert(std::string("baseline"), std::string("GsWf"));
        double nap_value = 4.0;
        table.Insert(std::string("nap"), nap_value);

        // --- Clone ---
        auto* clone = table.Clone();
        RQ(clone->GetSize() == DIM0 * DIM1 * DIM2);

        T mutated_val = make_mutated< T >();
        table(0, 0, 0) = mutated_val;

        // Clone should be unchanged (value at (0,0,0) = make_value(0))
        RQ((*clone)(0, 0, 0) == make_value< T >(0.0));

        for(std::size_t i = 0; i < DIM0; i++)
            RQ(std::get< 0 >(*clone)(i) == 0.1 * static_cast< double >(i));
        for(std::size_t j = 0; j < DIM1; j++)
            RQ(std::get< 1 >(*clone)(j) == 10.0 + static_cast< double >(j));

        delete clone;

        // Restore original (0,0,0) before serialization (was mutated for clone test)
        table(0, 0, 0) = make_value< T >(0.0);

        // --- Serialize / Deserialize ---
        std::string filename = "./TestMHO_TableContainer.bin";
        std::string index_filename = "./TestMHO_TableContainer.index";

        MHO_BinaryFileInterface inter;
        RQ(inter.OpenToWrite(filename, index_filename) == true);
        {
            std::string shortname = "table";
            inter.Write(table, shortname);
        }
        inter.Close();

        MHO_TableContainer< T, test_axis_pack > written;
        RQ(inter.OpenToRead(filename) == true);
        {
            MHO_FileKey key;
            inter.Read(written, key);
        }
        inter.Close();

        RQ(written.GetSize() == DIM0 * DIM1 * DIM2);

        for(std::size_t i = 0; i < DIM0; i++)
        {
            for(std::size_t j = 0; j < DIM1; j++)
            {
                for(std::size_t k = 0; k < DIM2; k++)
                {
                    double fill = static_cast< double >(1000 * i + 100 * j + 10 * k);
                    RQ(written(i, j, k) == make_value< T >(fill));
                }
            }
        }

        // Axis coordinates
        for(std::size_t i = 0; i < DIM0; i++)
            RQ(std::get< 0 >(written)(i) == 0.1 * static_cast< double >(i));
        for(std::size_t j = 0; j < DIM1; j++)
            RQ(std::get< 1 >(written)(j) == 10.0 + static_cast< double >(j));
        RQ(std::get< 2 >(written)(0) == "r");
        RQ(std::get< 2 >(written)(1) == "g");

        // Container tags
        {
            std::string baseline_val;
            RQ(written.Retrieve(std::string("baseline"), baseline_val) == true);
            RQ(baseline_val == "GsWf");
        }
        {
            double nap_rt = 0.0;
            RQ(written.Retrieve(std::string("nap"), nap_rt) == true);
            RQ(nap_rt == 4.0);
        }
        // Axis tag
        {
            std::string units_val;
            auto* w_axis0 = &std::get< 0 >(written);
            RQ(w_axis0->Retrieve(std::string("units"), units_val) == true);
            RQ(units_val == "Hz");
        }

        std::remove(filename.c_str());
        std::remove(index_filename.c_str());
    }

#undef RQ
    return failures;
}

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // T1-T6 for each element type
    {
        int f = run_table_case< double >();
        if(f > 0)
        {
            std::cerr << "[FAIL] run_table_case<double>() returned " << f << " failures" << std::endl;
            return 1;
        }
    }
    {
        int f = run_table_case< std::complex< double > >();
        if(f > 0)
        {
            std::cerr << "[FAIL] run_table_case<std::complex<double>>() returned " << f << " failures" << std::endl;
            return 1;
        }
    }
    {
        int f = run_table_case< int >();
        if(f > 0)
        {
            std::cerr << "[FAIL] run_table_case<int>() returned " << f << " failures" << std::endl;
            return 1;
        }
    }

    //
    // T7. Frozen type-id strings (regression guard)
    //
    {
        MHO_ContainerDictionary cdict;

        // visibility_type
        {
            visibility_type v;
            std::string name = MHO_ClassIdentity::ClassName(v);
            REQUIRE(name == "MHO_TableContainer<std::complex<double>,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<"
                            "double>,MHO_Axis<double>>>");
            REQUIRE(v.GetTypeUUID() == cdict.GetUUIDFor< visibility_type >());
        }

        // weight_type
        {
            weight_type w;
            std::string name = MHO_ClassIdentity::ClassName(w);
            REQUIRE(
                name ==
                "MHO_TableContainer<double,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>,MHO_Axis<double>>>");
            REQUIRE(w.GetTypeUUID() == cdict.GetUUIDFor< weight_type >());
        }

        // mbd_dr_type
        {
            mbd_dr_type m;
            std::string name = MHO_ClassIdentity::ClassName(m);
            REQUIRE(name == "MHO_TableContainer<std::complex<double>,MHO_AxisPack<MHO_Axis<double>,MHO_Axis<double>>>");
            REQUIRE(m.GetTypeUUID() == cdict.GetUUIDFor< mbd_dr_type >());
        }

        // station_coord_type
        {
            station_coord_type s;
            std::string name = MHO_ClassIdentity::ClassName(s);
            REQUIRE(name == "MHO_TableContainer<double,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<int>>>");
            REQUIRE(s.GetTypeUUID() == cdict.GetUUIDFor< station_coord_type >());
        }

        // mbd_dr_axis_pack
        {
            mbd_dr_axis_pack a;
            std::string name = MHO_ClassIdentity::ClassName(a);
            REQUIRE(name == "MHO_AxisPack<MHO_Axis<double>,MHO_Axis<double>>");
        }

        // baseline_axis_pack
        {
            baseline_axis_pack a;
            std::string name = MHO_ClassIdentity::ClassName(a);
            REQUIRE(name == "MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>,MHO_Axis<double>>");
        }

        // uch_baseline_axis_pack
        {
            uch_baseline_axis_pack a;
            std::string name = MHO_ClassIdentity::ClassName(a);
            REQUIRE(name == "MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>>");
        }

        // station_coord_axis_pack
        {
            station_coord_axis_pack a;
            std::string name = MHO_ClassIdentity::ClassName(a);
            REQUIRE(name == "MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<int>>");
        }
    }

    return 0;
}
