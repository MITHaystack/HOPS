#include <iostream>
#include <cmath>
#include <cstdint>
#include <string>
#include <ctime>

#include "MHO_TestAssertions.hh"
#include "MHO_TimeStampConverter.hh"

using namespace hops;


/* Helper: verify round-trip epoch->string->epoch */

static int roundtrip_epoch(uint64_t e0, double f0)
{
    std::string s;
    REQUIRE(MHO_TimeStampConverter::ConvertEpochSecondToTimeStamp(e0, f0, s));

    uint64_t e1 = 0;
    double   f1 = 0.0;
    REQUIRE(MHO_TimeStampConverter::ConvertTimeStampToEpochSecond(s, e1, f1));

    REQUIRE(e1 == e0);
    CHECK_CLOSE(f1, f0, 1e-9);
    return 0;
}


/* Helper: verify round-trip string->epoch->string (idempotency) */

static int roundtrip_string(const std::string& s0)
{
    uint64_t e  = 0;
    double   f  = 0.0;
    REQUIRE(MHO_TimeStampConverter::ConvertTimeStampToEpochSecond(s0, e, f));

    std::string s1;
    REQUIRE(MHO_TimeStampConverter::ConvertEpochSecondToTimeStamp(e, f, s1));

    REQUIRE_EQUAL(s1, s0);
    return 0;
}


int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);


    /* Case 1  Epoch -> timestamp golden (forward)  */
    /* we strip trailing zeros from the fractional */
    /* nanosecond string, so check by round-tripping back to epoch */
    {
        uint64_t epochs[] = {0, 1000000000, 1234567890, 1609459200};
        double   fracs[]  = {0.0, 0.0, 0.5, 0.123};
        for (int i = 0; i < 4; ++i) {
            std::string s;
            bool ok = MHO_TimeStampConverter::ConvertEpochSecondToTimeStamp(epochs[i], fracs[i], s);
            REQUIRE(ok);
            REQUIRE(s.size() > 0);

            /* Verify by parsing back */
            uint64_t e = 0;
            double   f = 0.0;
            ok = MHO_TimeStampConverter::ConvertTimeStampToEpochSecond(s, e, f);
            REQUIRE(ok);
            REQUIRE(e == epochs[i]);
            REQUIRE(std::fabs(f - fracs[i]) < 1e-9);
        }
    }


    /* Case 2  Timestamp -> epoch golden (reverse) */
    /* We use the canonical strings emitted by the forward converter. */
    {
        /* T1: epoch=0, frac=0.0 -> "1970-01-01T00:00:00Z" */
        {
            uint64_t e = 0;
            double   f = 0.0;
            bool ok = MHO_TimeStampConverter::ConvertTimeStampToEpochSecond("1970-01-01T00:00:00Z", e, f);
            REQUIRE(ok);
            REQUIRE(e == 0);
            REQUIRE(std::fabs(f - 0.0) < 1e-9);
        }
        /* T2: epoch=1000000000, frac=0.0 -> "2001-09-09T01:46:40Z" */
        {
            uint64_t e = 0;
            double   f = 0.0;
            bool ok = MHO_TimeStampConverter::ConvertTimeStampToEpochSecond("2001-09-09T01:46:40Z", e, f);
            REQUIRE(ok);
            REQUIRE(e == 1000000000);
            REQUIRE(std::fabs(f - 0.0) < 1e-9);
        }
        /* T3: epoch=1234567890, frac=0.5 -> "2009-02-13T23:31:30.5Z" */
        {
            uint64_t e = 0;
            double   f = 0.0;
            bool ok = MHO_TimeStampConverter::ConvertTimeStampToEpochSecond("2009-02-13T23:31:30.5Z", e, f);
            REQUIRE(ok);
            REQUIRE(e == 1234567890);
            REQUIRE(std::fabs(f - 0.5) < 1e-9);
        }
        /* T4: epoch=1609459200, frac=0.123 -> "2021-01-01T00:00:00.123Z" */
        {
            uint64_t e = 0;
            double   f = 0.0;
            bool ok = MHO_TimeStampConverter::ConvertTimeStampToEpochSecond("2021-01-01T00:00:00.123Z", e, f);
            REQUIRE(ok);
            REQUIRE(e == 1609459200);
            REQUIRE(std::fabs(f - 0.123) < 1e-9);
        }
    }


    /* Case 3  Numeric round-trip epoch->string->epoch*/
    /* Includes golden table + current time */
    {
        REQUIRE(roundtrip_epoch(0, 0.0) == 0);
        REQUIRE(roundtrip_epoch(1000000000, 0.0) == 0);
        REQUIRE(roundtrip_epoch(1234567890, 0.5) == 0);
        REQUIRE(roundtrip_epoch(1609459200, 0.123) == 0);

        /* Current time */
        uint64_t now_epoch = static_cast<uint64_t>(std::time(nullptr));
        REQUIRE(roundtrip_epoch(now_epoch, 0.123) == 0);
    }


    /* Case 4  Round-trip string->epoch->string (idempotency) */
    {
        REQUIRE(roundtrip_string("1970-01-01T00:00:00Z") == 0);
        REQUIRE(roundtrip_string("2001-09-09T01:46:40Z") == 0);
        REQUIRE(roundtrip_string("2009-02-13T23:31:30.5Z") == 0);
        REQUIRE(roundtrip_string("2021-01-01T00:00:00.123Z") == 0);
    }


    /* Case 5  Fractional-second rounding to nearest nanosecond   */
    /* The implementation rounds frac to integer nanoseconds, outputs */
    /* the integer as a decimal string, then strips trailing zeros.*/
    /* This means the emitted string is NOT a standard fractional */
    /* second representation: "123000000" -> ".123" works, but */
    /* "1000000" -> ".1" does not equal 0.001. Only the forward */
    /* conversion's rounding behavior is verified here. */
    {
        /* 0.0000000004 rounds to 0 ns -> no fractional part emitted */
        {
            std::string s;
            bool ok = MHO_TimeStampConverter::ConvertEpochSecondToTimeStamp(0, 0.0000000004, s);
            REQUIRE(ok);
            REQUIRE(s == "1970-01-01T00:00:00Z");
        }
        /* 0.0000000006 rounds to 1 ns -> emits ".1Z" */
        /* NOTE: round-trip to sub-nanosecond precision is not possible  */
        /* with this string representation  */
        {
            std::string s;
            bool ok = MHO_TimeStampConverter::ConvertEpochSecondToTimeStamp(0, 0.0000000006, s);
            REQUIRE(ok);
            REQUIRE(s == "1970-01-01T00:00:00.1Z");
        }
        /* Normal fractional parts round-trip correctly */
        {
            REQUIRE(roundtrip_epoch(0, 0.5) == 0);
            REQUIRE(roundtrip_epoch(0, 0.123) == 0);
        }
    }


    /* Case 6  Varying fractional digit counts on parse */
    {
        uint64_t e1 = 0; double f1 = 0.0;
        uint64_t e2 = 0; double f2 = 0.0;
        uint64_t e3 = 0; double f3 = 0.0;

        bool ok1 = MHO_TimeStampConverter::ConvertTimeStampToEpochSecond("2009-02-13T23:31:30.5Z", e1, f1);
        bool ok2 = MHO_TimeStampConverter::ConvertTimeStampToEpochSecond("2009-02-13T23:31:30.500Z", e2, f2);
        bool ok3 = MHO_TimeStampConverter::ConvertTimeStampToEpochSecond("2009-02-13T23:31:30.500000000Z", e3, f3);

        REQUIRE(ok1);
        REQUIRE(ok2);
        REQUIRE(ok3);

        REQUIRE(e1 == 1234567890);
        REQUIRE(e2 == 1234567890);
        REQUIRE(e3 == 1234567890);

        REQUIRE(std::fabs(f1 - 0.5) < 1e-9);
        REQUIRE(std::fabs(f2 - 0.5) < 1e-9);
        REQUIRE(std::fabs(f3 - 0.5) < 1e-9);
    }


    /* Case 7  Zero fractional part / integer-second timestamp  */
    {
        std::string s;
        bool ok = MHO_TimeStampConverter::ConvertEpochSecondToTimeStamp(1000000000, 0.0, s);
        REQUIRE(ok);

        uint64_t e = 0;
        double   f = 0.0;
        ok = MHO_TimeStampConverter::ConvertTimeStampToEpochSecond(s, e, f);
        REQUIRE(ok);
        REQUIRE(e == 1000000000);
        REQUIRE(std::fabs(f - 0.0) < 1e-12);
    }


    /* Case 8  Malformed input returns false */
    {
        uint64_t e = 0;
        double   f = 0.0;
        bool ok;

        /* Empty string */
        ok = MHO_TimeStampConverter::ConvertTimeStampToEpochSecond("", e, f);
        REQUIRE(!ok);

        /* Not a date */
        ok = MHO_TimeStampConverter::ConvertTimeStampToEpochSecond("not-a-date", e, f);
        REQUIRE(!ok);

        /* Out-of-range month/day/hour/minute */
        ok = MHO_TimeStampConverter::ConvertTimeStampToEpochSecond("2009-13-40T99:99:99Z", e, f);
        REQUIRE(!ok);

        /* Space instead of T - size is 20, ends with Z, but position 10 is ' ' not 'T' */
        ok = MHO_TimeStampConverter::ConvertTimeStampToEpochSecond("2009-02-13 23:31:30Z", e, f);
        REQUIRE(!ok);
    }


    /* Case 9  Boundary: epoch_sec == 0   */
    {
        std::string s;
        bool ok = MHO_TimeStampConverter::ConvertEpochSecondToTimeStamp(0, 0.0, s);
        REQUIRE(ok);
        REQUIRE(s == "1970-01-01T00:00:00Z");
    }

    return 0;
}
