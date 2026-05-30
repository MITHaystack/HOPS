#include <cstddef>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "MHO_MD5HashGenerator.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static std::string md5_of_string(const std::string& s)
{
    MHO_MD5HashGenerator gen;
    std::string streamable = s;
    gen << streamable;
    gen.Finalize();
    return gen.GetDigest();
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // Test 1: RFC 1321 string test vectors (table-driven)
    {
        struct Vector { std::string input; std::string expected; };

        Vector vectors[] = {
            { "", "d41d8cd98f00b204e9800998ecf8427e" },
            { "a", "0cc175b9c0f1b6a831c399e269772661" },
            { "abc", "900150983cd24fb0d6963f7d28e17f72" },
            { "message digest", "f96b697d7cb7938d525a2f31aaf161d0" },
            { "abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b" },
            { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "d174ab98d277d9f5a5611c2c9f419d9f" },
            { "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "57edf4a22be3c955ac49da2e2107b67a" }
        };

        for(unsigned int i = 0; i < 7; i++)
        {
            std::string result = md5_of_string(vectors[i].input);
            REQUIRE(result == vectors[i].expected);
        }
    }

    // Test 2: Digest format (32 lowercase hex chars)
    {
        std::string digest = md5_of_string("abc");
        REQUIRE(digest.size() == 32);
        for(unsigned int i = 0; i < digest.size(); i++)
        {
            char c = digest[i];
            bool valid = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
            REQUIRE(valid);
        }
    }

    // Test 3: Initialize() resets state (reuse without fresh construction)
    {
        MHO_MD5HashGenerator g;

        std::string s1 = "abc";
        g << s1;
        g.Finalize();
        std::string digest1 = g.GetDigest();
        REQUIRE(digest1 == "900150983cd24fb0d6963f7d28e17f72");

        g.Initialize();
        std::string s2 = "message digest";
        g << s2;
        g.Finalize();
        std::string digest2 = g.GetDigest();
        REQUIRE(digest2 == "f96b697d7cb7938d525a2f31aaf161d0");
    }

    // Test 4: GetDigestAsUUID round-trip vs GetDigest
    {
        MHO_MD5HashGenerator g;
        std::string s = "abc";
        g << s;
        g.Finalize();

        std::string digest = g.GetDigest();
        REQUIRE(digest == "900150983cd24fb0d6963f7d28e17f72");

        MHO_UUID uuid = g.GetDigestAsUUID();

        // Rebuild the 32-char hex string from the 16 UUID bytes
        std::stringstream ss;
        for(unsigned int i = 0; i < MHO_UUID_LENGTH; i++)
        {
            uint32_t tmp = uuid[i];
            std::stringstream hss;
            hss << std::setw(2) << std::setfill('0') << std::hex << (int)(tmp);
            ss << hss.str();
        }
        std::string rebuilt = ss.str();
        REQUIRE(rebuilt == digest);
    }

    // Test 5: POD streaming determinism (self-referential)
    {
        MHO_MD5HashGenerator genA;
        MHO_MD5HashGenerator genB;

        int data1 = 32349482;
        int data2 = 238;

        genA << data1;
        genA << data2;
        genA.Finalize();

        genB << data1;
        genB << data2;
        genB.Finalize();

        REQUIRE(genA.GetDigest() == genB.GetDigest());

        // Also verify the digest is 32 lowercase hex chars
        std::string digest = genA.GetDigest();
        REQUIRE(digest.size() == 32);
        for(unsigned int i = 0; i < digest.size(); i++)
        {
            char c = digest[i];
            bool valid = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
            REQUIRE(valid);
        }
    }

    // Test 6: Streaming equivalence (string via operator<< matches RFC vector)
    {
        MHO_MD5HashGenerator g;
        std::string s = "abc";
        g << s;
        g.Finalize();
        REQUIRE(g.GetDigest() == "900150983cd24fb0d6963f7d28e17f72");
    }

    return 0;
}
