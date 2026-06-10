"""unit test: vpal.utility angle helpers (no data files required).

Regression test for the minimum_angular_difference() branch-cut bug: the
old implementation only corrected differences > +span/2, so e.g.
(-170) - (+170) = -340 was returned as -340 instead of +20. Downstream,
ffres2pcp's sigma cut took abs() of that and spuriously discarded phase
estimates on channels straddling the +/-180 branch cut.

"""

import math
import os
import sys
import importlib.util

#HOPS imports
import hops_test as ht
import vpal.fourphase_lib
import vpal.utility as util

# UTILITY_PATH = os.path.join("@CMAKE_CURRENT_SOURCE_DIR@", "..", "vpal_module", "vpal", "utility.py")
#
#
# def load_utility():
#     spec = importlib.util.spec_from_file_location("vpal_utility", UTILITY_PATH)
#     mod = importlib.util.module_from_spec(spec)
#     spec.loader.exec_module(mod)
#     return mod


def approx(a, b, tol=1e-9):
    return abs(a - b) <= tol


def main():
    #util = load_utility()
    failures = []

    def check(label, got, expected, tol=1e-9):
        if not approx(got, expected, tol):
            failures.append("%s: got %r expected %r" % (label, got, expected))

    # ---- limit_periodic_quantity_to_range ----
    lim = util.limit_periodic_quantity_to_range
    check("lim(190,-180,180)", lim(190.0, -180.0, 180.0), -170.0)
    check("lim(-190,-180,180)", lim(-190.0, -180.0, 180.0), 170.0)
    check("lim(-180,-180,180)", lim(-180.0, -180.0, 180.0), -180.0)
    check("lim(180,-180,180)", lim(180.0, -180.0, 180.0), -180.0)  # [low, high)
    check("lim(370,0,360)", lim(370.0, 0.0, 360.0), 10.0)
    check("lim(-10,0,360)", lim(-10.0, 0.0, 360.0), 350.0)
    # ranges not symmetric about zero (broken before the (v - low) % span fix)
    check("lim(10,10,370)", lim(10.0, 10.0, 370.0), 10.0)
    check("lim(5,10,370)", lim(5.0, 10.0, 370.0), 365.0)
    # swapped bounds are reordered
    check("lim(190,180,-180)", lim(190.0, 180.0, -180.0), -170.0)

    # ---- minimum_angular_difference ----
    mad = util.minimum_angular_difference
    check("mad(10,350) deg", mad(10.0, 350.0), 20.0)
    check("mad(350,10) deg", mad(350.0, 10.0), -20.0)
    # the regression case: wrap in the negative direction
    check("mad(-170,170)", mad(-170.0, 170.0), 20.0)
    check("mad(170,-170)", mad(170.0, -170.0), -20.0)
    check("mad(0,0)", mad(0.0, 0.0), 0.0)
    check("mad(45,30)", mad(45.0, 30.0), 15.0)
    check("mad(30,45)", mad(30.0, 45.0), -15.0)
    # antipodal point folds to -span/2 by the [-span/2, span/2) convention
    check("mad(180,0)", mad(180.0, 0.0), -180.0)
    # radians range as used by pcc_delay_fitting
    check("mad pi-wrap", mad(-3.0, 3.0, -math.pi, math.pi), 2.0 * math.pi - 6.0)
    # magnitude never exceeds span/2
    for a in range(-360, 361, 17):
        for b in range(-360, 361, 23):
            d = mad(float(a), float(b))
            if abs(d) > 180.0 + 1e-9:
                failures.append("mad(%d,%d) magnitude %r exceeds 180" % (a, b, d))

    if failures:
        for f in failures:
            print("FAIL:", f)
        return 1
    print("test_utility_angles: all checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
