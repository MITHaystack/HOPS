// Unit tests for MHO_ChannelUtilities::MapChannelQuantities.
//
// Behavior (per the implementation): the channel-name string is split on commas
// if any comma is present, otherwise into single characters; the names are then
// zipped with the value vector into a map. On a length mismatch an error is
// logged and the result is truncated to the shorter of the two (zip_into_map
// stops at the shorter sequence)

#include <map>
#include <string>
#include <vector>

#include "MHO_ChannelUtilities.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal); // suppress the mismatch error logs

    // Case 1: comma-separated names, matched lengths -> full map
    {
        std::vector< double > vals = {1.0, 2.0, 3.0};
        std::map< std::string, double > m = MapChannelQuantities("a,bb,ccc", vals);
        REQUIRE(m.size() == 3);
        CHECK_CLOSE(m.at("a"), 1.0, 1e-15);
        CHECK_CLOSE(m.at("bb"), 2.0, 1e-15);
        CHECK_CLOSE(m.at("ccc"), 3.0, 1e-15);
    }

    // Case 2: no comma -> split into single characters
    {
        std::vector< double > vals = {10.0, 20.0, 30.0};
        std::map< std::string, double > m = MapChannelQuantities("abc", vals);
        REQUIRE(m.size() == 3);
        CHECK_CLOSE(m.at("a"), 10.0, 1e-15);
        CHECK_CLOSE(m.at("b"), 20.0, 1e-15);
        CHECK_CLOSE(m.at("c"), 30.0, 1e-15);
    }

    // Case 3: single channel
    {
        std::vector< double > vals = {5.5};
        std::map< std::string, double > m = MapChannelQuantities("a", vals);
        REQUIRE(m.size() == 1);
        CHECK_CLOSE(m.at("a"), 5.5, 1e-15);
    }

    // Case 4: more names than values -> truncated to the values count
    {
        std::vector< double > vals = {1.0, 2.0};
        std::map< std::string, double > m = MapChannelQuantities("a,b,c", vals);
        REQUIRE(m.size() == 2);
        CHECK_CLOSE(m.at("a"), 1.0, 1e-15);
        CHECK_CLOSE(m.at("b"), 2.0, 1e-15);
        REQUIRE(m.count("c") == 0); // dropped by truncation
    }

    // Case 5: more values than names -> truncated to the names count
    {
        std::vector< double > vals = {1.0, 2.0, 3.0};
        std::map< std::string, double > m = MapChannelQuantities("a,b", vals);
        REQUIRE(m.size() == 2);
        CHECK_CLOSE(m.at("a"), 1.0, 1e-15);
        CHECK_CLOSE(m.at("b"), 2.0, 1e-15);
    }

    // Case 6: empty values vector -> empty map (zip stops immediately)
    {
        std::vector< double > vals;
        std::map< std::string, double > m = MapChannelQuantities("abc", vals);
        REQUIRE(m.size() == 0);
    }

    return 0;
}
