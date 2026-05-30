#include <iostream>
#include <string>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_StationIdentifier.hh"
#include "MHO_StationIdentity.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eError);

    // =========================================================
    // PART A- MHO_StationIdentity value-type behavior
    // =========================================================

    // Case 1: Normalization on construction / setters
    {
        MHO_StationIdentity s("westford", "Wfxx", "Eyz");
        REQUIRE_EQUAL(s.GetStationName(), "WESTFORD");
        REQUIRE_EQUAL(s.GetStationCode(), "Wf");
        REQUIRE_EQUAL(s.GetStationMk4Id(), "E");
    }

    // Case 2: Equality is by NAME only
    {
        MHO_StationIdentity a("WESTFORD", "Wf", "E");
        MHO_StationIdentity b("WESTFORD", "Zz", "Q");  // same name, diff code/id
        MHO_StationIdentity c("GGAO12M", "Wf", "E");   // diff name, same code/id
        REQUIRE(a == b);
        REQUIRE(!(a == c));
        REQUIRE(a != c);
    }

    // Case 3: matches() dispatch by identifier length
    {
        MHO_StationIdentity s("WESTFORD", "Wf", "E");
        REQUIRE(s.matches("E"));          // len 1> mk4id
        REQUIRE(s.matches("Wf"));         // len 2> code
        REQUIRE(s.matches("westford"));   // len>2> name (case-insensitive)
        REQUIRE(!s.matches("X"));         // wrong mk4id
        REQUIRE(!s.matches("Zz"));        // wrong code
        REQUIRE(!s.matches("GGAO12M"));   // wrong name
    }

    // Case 4: as_string / from_string round-trip
    // as_string emits "name,code,mk4id"; from_string must restore all three fields.
    {
        MHO_StationIdentity s("WESTFORD", "Wf", "E");
        std::string str = s.as_string();
        REQUIRE_EQUAL(str, "WESTFORD,Wf,E");

        MHO_StationIdentity t;
        bool ok = t.from_string(str);
        REQUIRE(ok);
        REQUIRE_EQUAL(t.GetStationName(), "WESTFORD");
        REQUIRE_EQUAL(t.GetStationCode(), "Wf");
        REQUIRE_EQUAL(t.GetStationMk4Id(), "E");
        // full round-trip should reproduce the original serialization
        REQUIRE_EQUAL(t.as_string(), str);
    }

    // Case 5: operator< orders by NAME only and is consistent with operator==
    {
        MHO_StationIdentity a("ALPHA", "Aa", "A");
        MHO_StationIdentity b("BRAVO", "Bb", "B");

        // "ALPHA" < "BRAVO" lexicographically; ordering is asymmetric
        REQUIRE(a < b);
        REQUIRE(!(b < a));

        // same name, different code/mk4id -> equivalent under < (neither precedes)
        MHO_StationIdentity c("ALPHA", "Zz", "Q");
        REQUIRE(!(a < c));
        REQUIRE(!(c < a));

        // consistency: two identities are equivalent under < exactly when == is true
        REQUIRE((!(a < c) && !(c < a)) == (a == c)); // same name  -> equivalent & equal
        REQUIRE((!(a < b) && !(b < a)) == (a == b)); // diff name  -> ordered & unequal

        // default-constructed (all-empty) identities compare equivalent (no fall-through)
        MHO_StationIdentity d1;
        MHO_StationIdentity d2;
        REQUIRE(!(d1 < d2));
        REQUIRE(!(d2 < d1));
        REQUIRE(d1 == d2);
    }

    // =========================================================
    // PART B: MHO_StationIdentifier singleton
    // =========================================================

    // Case 6: GetInstance returns a stable non-null pointer
    {
        MHO_StationIdentifier* p1 = MHO_StationIdentifier::GetInstance();
        MHO_StationIdentifier* p2 = MHO_StationIdentifier::GetInstance();
        REQUIRE(p1 != nullptr);
        REQUIRE(p1 == p2);
    }

    // Case 7: Insert + three-way lookup (happy path)
    {
        MHO_StationIdentifier* p = MHO_StationIdentifier::GetInstance();
        int rc1 = p->Insert("WESTFORD", "Wf", "E");
        REQUIRE(rc1 == 0);
        int rc2 = p->Insert("GGAO12M", "Gs", "G");
        REQUIRE(rc2 == 0);
        int rc3 = p->Insert("ONSALA60", "On", "X");
        REQUIRE(rc3 == 0);

        // Lookup by mk4id
        REQUIRE_EQUAL(p->CanonicalStationName("E"), "WESTFORD");
        REQUIRE_EQUAL(p->CanonicalStationName("G"), "GGAO12M");
        // Lookup by code
        REQUIRE_EQUAL(p->CanonicalStationName("Wf"), "WESTFORD");
        REQUIRE_EQUAL(p->CanonicalStationName("On"), "ONSALA60");
        // Lookup by name
        REQUIRE_EQUAL(p->CanonicalStationName("WESTFORD"), "WESTFORD");
        REQUIRE_EQUAL(p->CanonicalStationName("ONSALA60"), "ONSALA60");
        // Mk4ID from name
        REQUIRE_EQUAL(p->StationMk4IDFromName("WESTFORD"), "E");
        REQUIRE_EQUAL(p->StationMk4IDFromName("GGAO12M"), "G");
        // Code from name
        REQUIRE_EQUAL(p->StationCodeFromName("WESTFORD"), "Wf");
        REQUIRE_EQUAL(p->StationCodeFromName("ONSALA60"), "On");
    }

    // Case 8: Insert is idempotent for an already-present name
    {
        MHO_StationIdentifier* p = MHO_StationIdentifier::GetInstance();
        int rc = p->Insert("WESTFORD", "Wf", "E");
        REQUIRE(rc == 0);
        // Lookup from Case 7 should still work
        REQUIRE_EQUAL(p->CanonicalStationName("E"), "WESTFORD");
    }

    // Case 9: Insert of NEW name that COLLIDES on code/mk4id is rejected
    {
        MHO_StationIdentifier* p = MHO_StationIdentifier::GetInstance();
        int rc = p->Insert("NEWNAME", "Wf", "Q");  // code "Wf" belongs to WESTFORD
        REQUIRE(rc == -1);
        // NEWNAME must not be resolvable (echo on miss)
        REQUIRE_EQUAL(p->CanonicalStationName("NEWNAME"), "NEWNAME");
        // Original mapping unchanged
        REQUIRE_EQUAL(p->CanonicalStationName("Wf"), "WESTFORD");
    }

    // Case 10: Lookup miss echoes the input unchanged
    {
        MHO_StationIdentifier* p = MHO_StationIdentifier::GetInstance();
        REQUIRE_EQUAL(p->CanonicalStationName("ZZZNOPE"), "ZZZNOPE");
        REQUIRE_EQUAL(p->StationMk4IDFromName("ZZZNOPE"), "ZZZNOPE");
        REQUIRE_EQUAL(p->StationCodeFromName("ZZZNOPE"), "ZZZNOPE");
    }

    // Case 11: Insert via MHO_StationIdentity overload
    {
        MHO_StationIdentifier* p = MHO_StationIdentifier::GetInstance();
        MHO_StationIdentity s("HAYSTACK", "Ha", "H");
        int rc = p->Insert(s);
        REQUIRE(rc == 0);
        REQUIRE_EQUAL(p->CanonicalStationName("H"), "HAYSTACK");
    }

    return 0;
}
