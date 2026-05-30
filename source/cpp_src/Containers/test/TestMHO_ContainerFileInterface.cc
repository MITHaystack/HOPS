#include <complex>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include "MHO_ClassIdentity.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerFileInterface.hh"
#include "MHO_ContainerJSONConverter.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"
#include "MHO_UUID.hh"

using namespace hops;

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    const std::string datafile = "./TestMHO_ContainerFileInterface.bin";
    const std::string indexfile = "./TestMHO_ContainerFileInterface.index";

    // ---- Build a store holding two distinct container types ----
    // (store takes ownership of the heap objects)
    visibility_type* vis = new visibility_type();
    vis->Resize(1, 2, 2, 2);
    std::size_t vis_n = vis->GetSize();
    for(std::size_t i = 0; i < vis_n; i++)
        (*vis)[i] = std::complex< double >(static_cast< double >(i), -static_cast< double >(i));

    weight_type* wt = new weight_type();
    wt->Resize(1, 2, 2, 2);
    std::size_t wt_n = wt->GetSize();
    for(std::size_t i = 0; i < wt_n; i++)
        (*wt)[i] = 0.5 * static_cast< double >(i);

    MHO_UUID vis_uuid = vis->GetObjectUUID();
    MHO_UUID wt_uuid = wt->GetObjectUUID();

    MHO_ContainerStore store;
    REQUIRE(store.AddObject(vis));
    REQUIRE(store.AddObject(wt));
    REQUIRE(store.SetShortName(vis_uuid, "vis"));
    REQUIRE(store.SetShortName(wt_uuid, "wt"));

    // C1: SetIndexFileName + write/read round-trip (index-driven)
    {
        MHO_ContainerFileInterface writer;
        writer.SetFilename(datafile);
        writer.SetIndexFileName(indexfile); // <-- under test: writes a companion index file
        writer.WriteStoreToFile(store);

        // Read back using the index file (single-pass key extraction path)
        MHO_ContainerStore store_idx;
        MHO_ContainerFileInterface reader;
        reader.SetFilename(datafile);
        reader.SetIndexFileName(indexfile); // <-- under test: index-driven read
        reader.PopulateStoreFromFile(store_idx, true);

        REQUIRE(store_idx.GetNObjects() == 2);
        REQUIRE(store_idx.GetNObjects< visibility_type >() == 1);
        REQUIRE(store_idx.GetNObjects< weight_type >() == 1);

        // both objects retain their short names
        std::vector< std::string > names;
        store_idx.GetAllShortNames(names);
        REQUIRE(names.size() == 2);
    }

    // C2: read WITHOUT an index file (2-pass path) gives same result
    //     (contrasts the SetIndexFileName branch against its absence)
    {
        MHO_ContainerStore store_noidx;
        MHO_ContainerFileInterface reader;
        reader.SetFilename(datafile);
        // intentionally no SetIndexFileName() -> 2-pass key extraction
        reader.PopulateStoreFromFile(store_noidx, true);

        REQUIRE(store_noidx.GetNObjects() == 2);
        REQUIRE(store_noidx.GetNObjects< visibility_type >() == 1);
        REQUIRE(store_noidx.GetNObjects< weight_type >() == 1);
    }

    // C3: ConvertObjectInStoreToJSON converts only the requested object
    {
        MHO_ContainerFileInterface iface;

        mho_json j;
        iface.ConvertObjectInStoreToJSON(store, vis_uuid, j, eJSONBasic);

        std::string vis_key = vis_uuid.as_string();
        REQUIRE(j.contains(vis_key));
        REQUIRE(j.at(vis_key).contains("class_name"));
        REQUIRE(j.at(vis_key).at("class_name") == MHO_ClassIdentity::ClassName< visibility_type >());
        REQUIRE(j.at(vis_key).contains("rank"));
        REQUIRE(j.at(vis_key).at("rank") == 4);

        // only the requested object is present -- the other object is not converted
        REQUIRE(j.contains(wt_uuid.as_string()) == false);

        // a UUID that is not in the store leaves the json empty
        mho_json j_bogus;
        MHO_UUID bogus; // zeroed
        iface.ConvertObjectInStoreToJSON(store, bogus, j_bogus, eJSONBasic);
        REQUIRE(j_bogus.is_null() || j_bogus.empty());
    }

    // C4: ConvertObjectInStoreToJSONAndRaw exposes JSON + raw byte access

    {
        MHO_ContainerFileInterface iface;

        mho_json j;
        std::size_t rank = 0;
        const char* raw_data = nullptr;
        std::size_t raw_data_bytes = 0;
        std::string raw_descriptor = "";

        iface.ConvertObjectInStoreToJSONAndRaw(store, wt_uuid, j, rank, raw_data, raw_data_bytes, raw_descriptor, eJSONAll);

        std::string wt_key = wt_uuid.as_string();
        REQUIRE(j.contains(wt_key));

        // weight_type is a rank-4 table of double
        REQUIRE(rank == 4);
        REQUIRE(raw_data != nullptr);
        REQUIRE(raw_data_bytes == wt_n * sizeof(double));
        REQUIRE(raw_descriptor.empty() == false);

        // the raw pointer aliases the live container's storage
        REQUIRE(reinterpret_cast< const double* >(raw_data) == wt->GetData());

        // and the bytes themselves match the container contents
        const double* raw_as_double = reinterpret_cast< const double* >(raw_data);
        for(std::size_t i = 0; i < wt_n; i++)
        {
            CHECK_CLOSE(raw_as_double[i], (*wt)[i], 1e-15);
        }
    }

    std::remove(datafile.c_str());
    std::remove(indexfile.c_str());
    return 0;
}
