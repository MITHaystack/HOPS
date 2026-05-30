#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <cctype>
#include <ctime>

#include "MHO_LegacyRootCodeGenerator.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;


/* Helper: verify each character is in the base-36 upper alphabet      */
/* [0-9A-Z]                                                           */

static bool is_base36_upper(const std::string& s)
{
    for (std::size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')))
            return false;
    }
    return true;
}


/* Helper: verify each character is in the base-26 lower alphabet      */
/* [a-z]                                                              */

static bool is_base26_lower(const std::string& s)
{
    for (std::size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (!((c >= 'a' && c <= 'z')))
            return false;
    }
    return true;
}


int main(int /*argc*/, char** /*argv*/)
{
    MHO_LegacyRootCodeGenerator gen;


    /* Case 1  Single code: length == 6 and valid base-36 upper alpha  */
    /* Current wall-clock is well past HOPS_ROOT_BREAK (2018-02-26),   */
    /* so the live path is always root_id_later (base-36 upper).       */

    {
        std::string code = gen.GetCode();
        REQUIRE(code.size() == 6);
        REQUIRE(is_base36_upper(code));
    }


    /* Case 2  GetCodes(N) returns exactly N codes, all structurally   */
    /* valid (length 6, base-36 upper).                                */

    {
        std::size_t n = 10;
        std::vector<std::string> codes = gen.GetCodes(n);
        REQUIRE(codes.size() == n);
        for (std::size_t i = 0; i < codes.size(); ++i) {
            REQUIRE(codes[i].size() == 6);
            REQUIRE(is_base36_upper(codes[i]));
        }
    }


    /* Case 3  Sequential codes are unique (no duplicates).            */
    /* Post-break the 1-second delta guarantees distinct codes.        */

    {
        std::size_t n = 100;
        std::vector<std::string> codes = gen.GetCodes(n);
        std::set<std::string> unique_set(codes.begin(), codes.end());
        REQUIRE(unique_set.size() == n);
    }


    /* Case 4  Sequential codes are strictly increasing (lexicographic)*/
    /* Base-36 fixed-width 6-char encoding with most-significant       */
    /* digit first means lexicographic order matches numeric order.    */
    {
        std::size_t n = 50;
        std::vector<std::string> codes = gen.GetCodes(n);
        for (std::size_t i = 1; i < codes.size(); ++i) {
            REQUIRE(codes[i] > codes[i - 1]);
        }
    }


    /* Case 5  Boundary: N=0 returns empty vector; N=1 returns one     */
    /* valid code.                                                     */
    {
        std::vector<std::string> codes0 = gen.GetCodes(0);
        REQUIRE(codes0.size() == 0);

        std::vector<std::string> codes1 = gen.GetCodes(1);
        REQUIRE(codes1.size() == 1);
        REQUIRE(codes1[0].size() == 6);
        REQUIRE(is_base36_upper(codes1[0]));
    }


    /* Case 6  Pre-break code generation (base-26 lower [a-z]).         */
    /* HOPS_ROOT_BREAK == 1519659904 (2018-02-26 UTC); for times before */
    /* it the generator uses root_id(), a 6-char base-26 lower encoding */
    /* of the number of 4-second periods elapsed since 1979-01-01.      */
    {
        // root_id_delta selects the 4-second pre-break stride before the break,
        // and the 1-second stride at/after it.
        REQUIRE(gen.root_id_delta(HOPS_ROOT_BREAK - 1) == 4);
        REQUIRE(gen.root_id_delta(HOPS_ROOT_BREAK)     == 1);

        // Golden: elapsed == 0 (1979-01-01T00:00:00, tm_year 79, day-of-year 1)
        // encodes to the all-zero base-26 code "aaaaaa".
        REQUIRE_EQUAL(gen.root_id(79, 1, 0, 0, 0), "aaaaaa");
        // One 4-second period later increments the least-significant digit.
        REQUIRE_EQUAL(gen.root_id(79, 1, 0, 0, 4), "aaaaab");

        // A pre-break code is 6 chars of base-26 lower, and is NOT base-36 upper
        // (the two encodings are disjoint, so the era is unambiguous).
        std::string pre = gen.root_id(79, 1, 0, 0, 0);
        REQUIRE(pre.size() == 6);
        REQUIRE(is_base26_lower(pre));
        REQUIRE(!is_base36_upper(pre));

        // root_id_break must route a pre-break timestamp to the base-26 path,
        // yielding the same result as calling root_id() directly.
        std::string routed = gen.root_id_break(0 /* now < break */, 79, 1, 0, 0, 0);
        REQUIRE_EQUAL(routed, pre);
        REQUIRE(is_base26_lower(routed));
    }

    return 0;
}
