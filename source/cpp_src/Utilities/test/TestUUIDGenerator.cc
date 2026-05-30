#include <cctype>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"
#include "MHO_UUID.hh"
#include "MHO_UUIDGenerator.hh"

using namespace hops;

/* helpers */

static bool is_hex_lower(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

/*
 * Validate the structural format of a UUID string produced by
 * ConvertUUIDToString / GenerateUUIDAsString.
 *
 * NOTE We emit a bare 32-char lowercase-hex string with NO hyphens,
 * NOT the canonical 8-4-4-4-12 layout from RFC 4122!
 */
static bool validate_uuid_string(const std::string& s)
{
    REQUIRE(s.size() == 32);
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        REQUIRE(is_hex_lower(s[i]));
    }
    return true;
}

/* Convert a two-character lowercase-hex substring to uint8_t. */
static uint8_t hex_pair_to_byte(const std::string& s, std::size_t pos)
{
    uint8_t hi = static_cast<uint8_t>(s[pos]);
    uint8_t lo = static_cast<uint8_t>(s[pos + 1]);
    if (hi > '9') hi -= ('a' - 10);
    else          hi -= '0';
    if (lo > '9') lo -= ('a' - 10);
    else          lo -= '0';
    return (hi << 4) | lo;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // Test 1: canonical string format (32 lowercase-hex chars)
    {
        MHO_UUIDGenerator gen;
        std::string s = gen.GenerateUUIDAsString();
        REQUIRE(validate_uuid_string(s));
    }

    // Test 2: RFC 4122 version nibble == 4
    // byte 6 is encoded at string chars 12..13; high nibble (char 12) must be '4'.
    {
        MHO_UUIDGenerator gen;
        MHO_UUID u = gen.GenerateUUID();

        // byte-level check
        REQUIRE((u[6] & 0xF0) == 0x40);

        // string-level cross-check
        std::string s = gen.ConvertUUIDToString(u);
        REQUIRE(s[12] == '4');
    }

    // Test 3: RFC 4122 variant bits (10xx) on clock-seq-hi byte
    // byte 8 is at string chars 16..17; high nibble char 16 in {8,9,a,b}.
    {
        MHO_UUIDGenerator gen;
        MHO_UUID u = gen.GenerateUUID();

        REQUIRE((u[8] & 0xC0) == 0x80);

        std::string s = gen.ConvertUUIDToString(u);
        char c = s[16];
        REQUIRE(c == '8' || c == '9' || c == 'a' || c == 'b');
    }

    // Test 4: uniqueness / no collisions (10 000 UUIDs)
    {
        MHO_UUIDGenerator gen;
        std::set<std::string> seen;
        const int N = 10000;
        for (int i = 0; i < N; ++i)
        {
            seen.insert(gen.GenerateUUIDAsString());
        }
        REQUIRE(static_cast<int>(seen.size()) == N);
    }

    // Test 5: ConvertUUIDToString faithfully encodes UUID bytes
    {
        MHO_UUIDGenerator gen;
        MHO_UUID u = gen.GenerateUUID();
        std::string s = gen.ConvertUUIDToString(u);

        REQUIRE(s.size() == 32);
        for (std::size_t i = 0; i < 16; ++i)
        {
            uint8_t recovered = hex_pair_to_byte(s, i * 2);
            REQUIRE(recovered == u[i]);
        }
    }

    // Test 6: both GenerateUUIDAsString and ConvertUUIDToString
    //             produce strings that pass format validation
    {
        MHO_UUIDGenerator gen;
        std::string s1 = gen.GenerateUUIDAsString();
        MHO_UUID u = gen.GenerateUUID();
        std::string s2 = gen.ConvertUUIDToString(u);
        REQUIRE(validate_uuid_string(s1));
        REQUIRE(validate_uuid_string(s2));
    }

    // Test 7: two independent generators produce distinct UUIDs
    {
        MHO_UUIDGenerator gen1;
        MHO_UUIDGenerator gen2;
        MHO_UUID u1 = gen1.GenerateUUID();
        MHO_UUID u2 = gen2.GenerateUUID();
        REQUIRE(u1 != u2);
    }

    return 0;
}
