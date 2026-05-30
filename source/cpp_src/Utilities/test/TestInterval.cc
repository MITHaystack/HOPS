#include <cstddef>
#include <iostream>
#include <utility>

#include "MHO_Interval.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // Test 1: Default construction
    {
        MHO_Interval<int> a;
        REQUIRE(a.GetLowerBound() == 0);
        REQUIRE(a.GetUpperBound() == 0);
        REQUIRE(a.GetLength() == 0);
    }

    // Test 2: Two-arg construction, ordered
    {
        MHO_Interval<int> a(3, 8);
        REQUIRE(a.GetLowerBound() == 3);
        REQUIRE(a.GetUpperBound() == 8);
        REQUIRE(a.GetLength() == 5);
        std::pair<int,int> p = a.GetInterval();
        REQUIRE(p.first == 3);
        REQUIRE(p.second == 8);
    }

    // Test 3: Construction with reversed bounds is normalized (swap)
    {
        MHO_Interval<int> a(8, 3);
        REQUIRE(a.GetLowerBound() == 3);
        REQUIRE(a.GetUpperBound() == 8);
        REQUIRE(a.GetLength() == 5);
    }

    // Test 4: Equal bounds> zero-length valid interval
    {
        MHO_Interval<int> a(5, 5);
        REQUIRE(a.GetLowerBound() == 5);
        REQUIRE(a.GetUpperBound() == 5);
        REQUIRE(a.GetLength() == 0);
    }

    // Test 5: SetBounds (both overloads) and SetLowerBound/SetUpperBound
    {
        // SetBounds(int, int)
        MHO_Interval<int> a;
        a.SetBounds(10, 20);
        REQUIRE(a.GetLowerBound() == 10);
        REQUIRE(a.GetUpperBound() == 20);

        // SetLowerBound
        a.SetLowerBound(15);
        REQUIRE(a.GetLowerBound() == 15);
        REQUIRE(a.GetUpperBound() == 20);

        // SetUpperBound
        a.SetUpperBound(40);
        REQUIRE(a.GetLowerBound() == 15);
        REQUIRE(a.GetUpperBound() == 40);

        // SetLowerBound with value > upper triggers swap via SetIntervalImpl
        // [15,40]> SetLowerBound(50)> swap> [40,50]
        a.SetLowerBound(50);
        REQUIRE(a.GetLowerBound() == 40);
        REQUIRE(a.GetUpperBound() == 50);

        // SetBounds(pair) on a fresh object
        MHO_Interval<int> b;
        b.SetBounds(std::make_pair(2, 9));
        REQUIRE(b.GetLowerBound() == 2);
        REQUIRE(b.GetUpperBound() == 9);
    }

    // Test 6: Copy constructor and assignment
    {
        MHO_Interval<int> a(3, 8);
        MHO_Interval<int> b(a);
        REQUIRE(b.GetLowerBound() == 3);
        REQUIRE(b.GetUpperBound() == 8);
        REQUIRE(b.GetLength() == 5);

        MHO_Interval<int> c;
        c = a;
        REQUIRE(c.GetLowerBound() == 3);
        REQUIRE(c.GetUpperBound() == 8);
        REQUIRE(c.GetLength() == 5);

        // Self-assignment safety
        a = a;
        REQUIRE(a.GetLowerBound() == 3);
        REQUIRE(a.GetUpperBound() == 8);
        REQUIRE(a.GetLength() == 5);
    }

    // Test 7: Intersects(point) CLOSED interval
    {
        MHO_Interval<int> a(3, 8);
        REQUIRE(a.Intersects(3) == true);     // lower endpoint included
        REQUIRE(a.Intersects(8) == true);     // upper endpoint included
        REQUIRE(a.Intersects(5) == true);     // inside
        REQUIRE(a.Intersects(2) == false);    // below lower
        REQUIRE(a.Intersects(9) == false);    // above upper
    }

    // Test 8: Intersects(interval) overlapping
    // FindIntersection treats touching endpoints as intersection (returns 1).
    // So [3,8] Intersects [8,20] should return true (touching at 8).
    {
        MHO_Interval<int> a(3, 8);
        MHO_Interval<int> b(5, 12);
        MHO_Interval<int> c(8, 20);
        MHO_Interval<int> d(20, 30);

        REQUIRE(a.Intersects(b) == true);   // overlap [5,8]
        // a and c touch at endpoint 8 FindIntersection returns 1 for touching
        // endpoints, so Intersects returns true
        REQUIRE(a.Intersects(c) == true);   // touch at 8 (FindIntersection returns 1)
        REQUIRE(a.Intersects(d) == false);  // disjoint
        REQUIRE(b.Intersects(a) == true);   // symmetry

        // Symmetry check
        REQUIRE(a.Intersects(b) == b.Intersects(a));
    }

    // Test 9: Intersection(interval) overlap region
    {
        MHO_Interval<int> a(3, 8);
        MHO_Interval<int> b(5, 12);
        MHO_Interval<int> d(20, 30);

        MHO_Interval<int> r = a.Intersection(b);
        REQUIRE(r.GetLowerBound() == 5);
        REQUIRE(r.GetUpperBound() == 8);
        REQUIRE(r.GetLength() == 3);

        MHO_Interval<int> r2 = a.Intersection(d);
        REQUIRE(r2.GetLowerBound() == 0);
        REQUIRE(r2.GetUpperBound() == 0);
        REQUIRE(r2.GetLength() == 0);
    }


    // Test 10: Union(interval) overlapping vs disjoint
    {
        MHO_Interval<int> a(3, 8);
        MHO_Interval<int> b(5, 12);
        MHO_Interval<int> d(20, 30);

        MHO_Interval<int> u = a.Union(b);
        REQUIRE(u.GetLowerBound() == 3);
        REQUIRE(u.GetUpperBound() == 12);
        REQUIRE(u.GetLength() == 9);

        MHO_Interval<int> u2 = a.Union(d);
        REQUIRE(u2.GetLowerBound() == 0);
        REQUIRE(u2.GetUpperBound() == 0);
        REQUIRE(u2.GetLength() == 0);
    }

    // Test 11: Default std::size_t instantiation smoke + large values
    {
        MHO_Interval<> a(0u, 1000000u);
        REQUIRE(a.GetLength() == 1000000u);
        REQUIRE(a.Intersects(500000u) == true);
        REQUIRE(a.Intersects(2000000u) == false);
    }

    // Test 12: Negative bounds (signed instantiation)
    {
        MHO_Interval<int> a(-10,-3);
        REQUIRE(a.GetLowerBound() == -10);
        REQUIRE(a.GetUpperBound() == -3);
        REQUIRE(a.GetLength() == 7);
        REQUIRE(a.Intersects(-5) == true);
        REQUIRE(a.Intersects(0) == false);
    }

    return 0;
}
