#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"
#include "MHO_UniformGridPointsCalculator.hh"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <vector>

using namespace hops;

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    //Case 1: Regular grid, no gaps
    {
        std::vector< double > pts = {1000.0, 1032.0, 1064.0, 1096.0, 1128.0, 1160.0, 1192.0, 1224.0};
        MHO_UniformGridPointsCalculator calc;
        calc.SetPoints(pts);
        calc.Calculate();

        REQUIRE(!calc.GetSpacingErrorStatus());
        CHECK_CLOSE(calc.GetGridSpacing(), 32.0, 1e-9);
        CHECK_CLOSE(calc.GetGridStart(), 1000.0, 1e-9);

        std::size_t n = calc.GetNGridPoints();
        REQUIRE(n >= 8);
        // power of two check
        REQUIRE((n & (n - 1)) == 0);

        // Round-trip: each input point should map to a grid node
        auto imap = calc.GetGridIndexMap();
        double start = calc.GetGridStart();
        double spacing = calc.GetGridSpacing();
        for(std::size_t i = 0; i < pts.size(); i++)
        {
            auto it = imap.find(i);
            REQUIRE(it != imap.end());
            double grid_val = start + spacing * static_cast< double >(it->second);
            REQUIRE(std::fabs(pts[i] - grid_val) < 1e-6);
        }
    }

    //Case 2: Sparse grid (gaps present)
    {
        std::vector< double > pts = {1000.0, 1032.0, 1096.0, 1160.0, 1256.0};
        MHO_UniformGridPointsCalculator calc;
        calc.SetPoints(pts);
        calc.Calculate();

        REQUIRE(!calc.GetSpacingErrorStatus());
        CHECK_CLOSE(calc.GetGridSpacing(), 32.0, 1e-9);
        CHECK_CLOSE(calc.GetGridStart(), 1000.0, 1e-9);

        auto imap = calc.GetGridIndexMap();
        REQUIRE(imap[0] == 0u);
        REQUIRE(imap[1] == 1u);
        REQUIRE(imap[2] == 3u); // 1096 = 1000 + 32*3
        REQUIRE(imap[3] == 5u); // 1160 = 1000 + 32*5
        REQUIRE(imap[4] == 8u); // 1256 = 1000 + 32*8
    }

    //Case 3: Duplicate handling via GetUniquePoints()
    {
        std::vector< double > in_pts = {1000.0, 1000.0, 1032.0, 1032.0, 1064.0};
        std::vector< double > out_pts;
        std::map< std::size_t, std::size_t > index_map;
        MHO_UniformGridPointsCalculator calc;
        calc.GetUniquePoints(in_pts, 1e-6, out_pts, index_map);

        REQUIRE(out_pts.size() == 3);
        CHECK_CLOSE(out_pts[0], 1000.0, 1e-6);
        CHECK_CLOSE(out_pts[1], 1032.0, 1e-6);
        CHECK_CLOSE(out_pts[2], 1064.0, 1e-6);
        REQUIRE(index_map[0] == 0u);
        REQUIRE(index_map[1] == 0u);
        REQUIRE(index_map[2] == 1u);
        REQUIRE(index_map[3] == 1u);
        REQUIRE(index_map[4] == 2u);
    }

    //Case 4: Single-point input (degenerate)
    {
        std::vector< double > pts = {5000.0};
        MHO_UniformGridPointsCalculator calc;
        calc.SetPoints(pts);
        calc.Calculate();

        // Current impl: with 1 point, min_space stays MAX, div increments to 256,
        // spacing becomes very small, but no error is triggered.
        // Key: no crash, and index_map[0] is valid.
        auto imap = calc.GetGridIndexMap();
        REQUIRE(imap.find(0u) != imap.end());
        REQUIRE(calc.GetNGridPoints() > 0);
    }

    //Case 5: Two-point input
    {
        std::vector< double > pts = {3000.0, 3128.0};
        MHO_UniformGridPointsCalculator calc;
        calc.SetPoints(pts);
        calc.Calculate();

        double spacing = calc.GetGridSpacing();
        REQUIRE(spacing > 0);
        // 128 should be divisible by the spacing
        REQUIRE(std::fmod(128.0, spacing) < 1e-9);

        auto imap = calc.GetGridIndexMap();
        REQUIRE(imap.find(0u) != imap.end());
        REQUIRE(imap.find(1u) != imap.end());
    }

    //Case 6: Realistic 32-point spectrum (regression snapshot)
    {
        // Column 0 from TestUniformGridCalculator2 freq_pts (sorted as recommended)
        std::vector< double > pts = {3000.4,  3032.4,  3064.4,  3192.4,  3288.4,  3352.4,  3416.4,  3448.4,
                                     5240.4,  5272.4,  5304.4,  5432.4,  5528.4,  5592.4,  5656.4,  5688.4,
                                     6360.4,  6392.4,  6424.4,  6552.4,  6648.4,  6712.4,  6776.4,  6808.4,
                                     10200.4, 10232.4, 10264.4, 10392.4, 10488.4, 10552.4, 10616.4, 10648.4};
        MHO_UniformGridPointsCalculator calc;
        calc.SetPoints(pts);
        calc.Calculate();

        REQUIRE(!calc.GetSpacingErrorStatus());
        CHECK_CLOSE(calc.GetGridStart(), 3000.4, 1e-9);
        CHECK_CLOSE(calc.GetGridSpacing(), 32.0, 1e-9);
        REQUIRE(calc.GetNGridPoints() == 1024u);

        // Full index_map regression (from current implementation output)
        auto imap = calc.GetGridIndexMap();
        // The sorted order maps: 3000.4->0, 3032.4->1, ..., 10648.4->225
        std::vector< std::size_t > expected_map = {
            0,   1,   2,   6,   9,   11,  13,  14,  // 3000-3448 range
            70,  71,  72,  76,  79,  81,  83,  84,  // 5240-5688 range
            105, 106, 107, 111, 114, 116, 118, 119, // 6360-6808 range
            225, 226, 227, 231, 234, 236, 238, 239  // 10200-10648 range
        };
        for(std::size_t i = 0; i < pts.size(); i++)
        {
            auto it = imap.find(i);
            REQUIRE(it != imap.end());
            REQUIRE(it->second == expected_map[i]);
        }
    }

    //Case 7: Idempotency / re-use
    {
        std::vector< double > pts = {1000.0, 1032.0, 1064.0, 1096.0, 1128.0, 1160.0, 1192.0, 1224.0};

        MHO_UniformGridPointsCalculator calc1;
        calc1.SetPoints(pts);
        calc1.Calculate();
        double start1 = calc1.GetGridStart();
        double spacing1 = calc1.GetGridSpacing();
        std::size_t n1 = calc1.GetNGridPoints();
        auto map1 = calc1.GetGridIndexMap();

        // Re-use same calculator with same points
        calc1.SetPoints(pts);
        calc1.Calculate();
        double start2 = calc1.GetGridStart();
        double spacing2 = calc1.GetGridSpacing();
        std::size_t n2 = calc1.GetNGridPoints();
        auto map2 = calc1.GetGridIndexMap();

        REQUIRE(start1 == start2);
        REQUIRE(spacing1 == spacing2);
        REQUIRE(n1 == n2);
        REQUIRE(map1 == map2);
    }

    //Case 8: Custom epsilon affects uniqueness
    {
        std::vector< double > in_pts = {1000.0, 1000.5, 1001.0};
        std::vector< double > out_pts;
        std::map< std::size_t, std::size_t > index_map;
        MHO_UniformGridPointsCalculator calc;

        // eps=0.1 -> all 3 points are unique (1000.5-1000=0.5 > 0.1)
        calc.GetUniquePoints(in_pts, 0.1, out_pts, index_map);
        REQUIRE(out_pts.size() == 3);

        // eps=1.0 -> all collapse to 1 point (1001-1000=1.0 <= 1.0)
        out_pts.clear();
        index_map.clear();
        calc.GetUniquePoints(in_pts, 1.0, out_pts, index_map);
        REQUIRE(out_pts.size() == 1);
    }

    //Case 9: Test a sequence of primes.
    //Since there is no common factor we expect the calculator to choose a spacing of 1,
    //producing a (sparse) grid where prime p lands on index p - start.
    {
        std::vector< double > primes = {2.0,  3.0,  5.0,  7.0,  11.0, 13.0,  17.0,  19.0,  23.0,  29.0,
                                        31.0, 37.0, 41.0, 43.0, 47.0, 53.0,  59.0,  61.0,  67.0,  71.0,
                                        73.0, 79.0, 83.0, 89.0, 97.0, 101.0, 103.0, 107.0, 109.0, 113.0};
        MHO_UniformGridPointsCalculator calc;
        calc.SetPoints(primes);
        calc.Calculate();

        REQUIRE(!calc.GetSpacingErrorStatus());
        CHECK_CLOSE(calc.GetGridSpacing(), 1.0, 1e-9);
        CHECK_CLOSE(calc.GetGridStart(), 2.0, 1e-9);

        // grid must cover the largest prime (113 -> index 111) and be a power of two
        std::size_t n = calc.GetNGridPoints();
        REQUIRE(n > 111u);
        REQUIRE((n & (n - 1)) == 0);

        // each prime p maps to grid index p - 2
        auto imap = calc.GetGridIndexMap();
        for(std::size_t i = 0; i < primes.size(); i++)
        {
            auto it = imap.find(i);
            REQUIRE(it != imap.end());
            REQUIRE(it->second == static_cast< std::size_t >(primes[i] - 2.0));
        }
    }

    return 0;
}
