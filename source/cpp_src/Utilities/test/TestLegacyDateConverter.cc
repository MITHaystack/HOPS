#include <iostream>
#include <cmath>
#include <string>

#include "MHO_LegacyDateConverter.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static bool date_eq(const legacy_hops_date& a,
                    const legacy_hops_date& b, double sec_tol)
{
    return a.year  == b.year  &&
           a.day   == b.day   &&
           a.hour  == b.hour  &&
           a.minute == b.minute &&
           std::fabs(a.second - b.second) < sec_tol;
}

static legacy_hops_date mk(short y, short doy, short h, short m, double s)
{
    legacy_hops_date d;
    d.year   = y;
    d.day    = doy;
    d.hour   = h;
    d.minute = m;
    d.second = s;
    return d;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // Golden data
    legacy_hops_date G[4] = {
        mk(2021, 305, 17,  0, 30.0),   // G1
        mk(2000,   1,  0,  0,  0.0),   // G2  (Jan 1 epoch)
        mk(2020, 366, 23, 59, 59.5),   // G3  (leap-year last day)
        mk(2019,  60, 12, 30, 15.0),   // G4  (non-leap day 60 = Mar 1)
    };
    const char* ISO[4] = {
        "2021-11-01T17:00:30.000000000Z",
        "2000-01-01T00:00:00.000000000Z",
        "2020-12-31T23:59:59.500000000Z",
        "2019-03-01T12:30:15.000000000Z",
    };
    const char* VEX[4] = {
        "2021y305d17h00m30s",
        "2000y001d00h00m00s",
        "2020y366d23h59m59.5s",
        "2019y060d12h30m15s",
    };

    // 1. ConvertToISO8601Format (forward, golden)
    for (int i = 0; i < 4; ++i) {
        std::string iso = MHO_LegacyDateConverter::ConvertToISO8601Format(G[i]);
        REQUIRE_EQUAL(iso, ISO[i]);
    }

    // 2. ConvertToVexFormat (forward, golden)
    for (int i = 0; i < 4; ++i) {
        std::string vex = MHO_LegacyDateConverter::ConvertToVexFormat(G[i]);
        REQUIRE_EQUAL(vex, VEX[i]);
    }

    // 3. ConvertFromISO8601Format (reverse, golden)--
    for (int i = 0; i < 4; ++i) {
        legacy_hops_date d = MHO_LegacyDateConverter::ConvertFromISO8601Format(ISO[i]);
        REQUIRE(date_eq(d, G[i], 1e-6));
    }

    // 4. ConvertFromVexFormat (reverse, golden)
    // VEX preserves fractional seconds, so round-trip is exact.
    for (int i = 0; i < 4; ++i) {
        legacy_hops_date d = MHO_LegacyDateConverter::ConvertFromVexFormat(VEX[i]);
        REQUIRE(date_eq(d, G[i], 1e-6));
    }

    // 5. Round-trip: legacy> ISO8601> legacy
    for (int i = 0; i < 4; ++i) {
        std::string iso = MHO_LegacyDateConverter::ConvertToISO8601Format(G[i]);
        legacy_hops_date d1 = MHO_LegacyDateConverter::ConvertFromISO8601Format(iso);
        REQUIRE(date_eq(G[i], d1, 1e-6));
    }

    // 6. Round-trip: legacy> VEX> legacy (exact VEX preserves fractions)
    for (int i = 0; i < 4; ++i) {
        std::string vex = MHO_LegacyDateConverter::ConvertToVexFormat(G[i]);
        legacy_hops_date d1 = MHO_LegacyDateConverter::ConvertFromVexFormat(vex);
        REQUIRE(date_eq(G[i], d1, 1e-6));
    }

    // 7. Empty-string inputs return the HOPS epoch--
    legacy_hops_date di = MHO_LegacyDateConverter::ConvertFromISO8601Format("");
    legacy_hops_date dv = MHO_LegacyDateConverter::ConvertFromVexFormat("");
    legacy_hops_date de = MHO_LegacyDateConverter::HopsEpoch();
    REQUIRE(date_eq(di, de, 1e-6));
    REQUIRE(date_eq(dv, de, 1e-6));

    // 8. HopsEpoch is deterministic / idempotent
    legacy_hops_date e1 = MHO_LegacyDateConverter::HopsEpoch();
    legacy_hops_date e2 = MHO_LegacyDateConverter::HopsEpoch();
    REQUIRE(date_eq(e1, e2, 1e-9));

    // 9. Now() smoke test (clock-dependent sanity only)
    legacy_hops_date n = MHO_LegacyDateConverter::Now();
    REQUIRE(n.year >= 2024);
    REQUIRE(n.day  >= 1  && n.day  <= 366);
    REQUIRE(n.hour >= 0  && n.hour <= 23);
    REQUIRE(n.minute >= 0 && n.minute <= 59);
    REQUIRE(n.second >= 0.0 && n.second < 60.5);

    return 0;
}
