#!/usr/bin/env python3
"""
compare_alist_v6.py -- Compare two fringex alist files row-by-row (v6 format only).

Rows are matched on the TIME*TAG string (data column index 11,
e.g. "105-180005"). Numeric columns are compared with per-column
or global tolerances.

Usage:
    compare_alist_v6.py FILE4 FILE3 [--skip COL ...] [--atol F] [--rtol F] [-v]

Exit code: 0 = all comparisons passed, 1 = one or more failures.
Unmatched TIME*TAG values produce warnings but do not cause failure.
"""

import sys
import argparse

# -------------------------------------------------------------------------
# Column name -> data token index (0-based) for alist v6 format.
# Verified against fformat_v6 in MHO_AFileInfoExtractor.cc and
# actual fringex4/fringex output.
#
# v6 differs from v5 primarily by:
#   - splitting ELEVATION into ELref (28) and ELrem (29)
#   - splitting AZIMUTH into AZref (30) and AZrem (31)
#   - shifting U, V, ESDESP, EPCH, REF_FREQ by +2 columns
#   - adding SRCH-COH (41), LOSS-COH (42), RA (43), DECL (44),
#     RESIDUALDELAY (45) at the end
# -------------------------------------------------------------------------
COL_IDX = {
    "AMP":          19,
    "SNR":          20,
    "PHASE":        21,  # residual fringe phase (degrees)
    "PSNR":         22,  # phase SNR; always 0 in fringex3
    "SBDLY":        24,  # single-band delay (usec)
    "MBDLY":        25,  # multi-band delay (usec)
    "AMB":          26,  # delay ambiguity (usec)
    "DRATE":        27,  # delay rate (ps/s)
    "ELref":        28,  # reference station elevation (deg)
    "ELrem":        29,  # remote station elevation (deg)
    "AZref":        30,  # reference station azimuth (deg)
    "AZrem":        31,  # remote station azimuth (deg)
    "U":            32,  # megalambda
    "V":            33,  # megalambda
    "REF_FREQ":     36,  # MHz
    "TOTPHASE":     37,  # total phase (degrees)
    "TOTDRATE":     38,  # total delay rate (usec/sec)
    "TOTMBDELAY":   39,  # total multi-band delay (usec)
    "TSBD-MBD":     40,  # total sbd minus mbd (usec)
    "SRCH-COH":     41,  # search coherence time (sec)
    "LOSS-COH":     42,  # no-loss coherence time (sec)
    "RA":           43,  # right ascension (hr)
    "DECL":         44,  # declination (deg)
    "RESIDUALDELAY": 45, # residual delay (usec)
}

# Columns to compare
COMPARE_COLS = list(COL_IDX.keys())

# Per-column absolute tolerance.
# Columns not listed here fall back to the --atol argument.
COL_ATOL = {
    "PHASE":        5.0,    # degrees
    "TOTPHASE":     10.0,   # degrees
    "SNR":          5.0,
    "AMP":          0.1,
    "DRATE":        0.01,   # ps/s
    "TOTDRATE":     1e-5,   # usec/sec
    "TOTMBDELAY":   0.001,  # usec
    "TSBD-MBD":     0.001,  # usec
    "SBDLY":        0.001,  # usec
    "MBDLY":        0.001,  # usec
    "ELref":        1.0,    # deg
    "ELrem":        1.0,    # deg
    "AZref":        1.0,    # deg
    "AZrem":        1.0,    # deg
    "U":            0.1,    # megalambda
    "V":            0.1,    # megalambda
    "REF_FREQ":     0.001,  # MHz
    "RA":           1e-4,   # hr
    "DECL":         1e-4,   # deg
    "RESIDUALDELAY": 0.001, # usec
}

# Columns excluded from comparison by default.
# PSNR is always 0 in fringex3 output.
# AMB is file-level, not meaningful to compare per-segment.
# EPCH is always 0000 in fringex3.
# SRCH-COH and LOSS-COH are not computed by fringex4 (always 0).
DEFAULT_SKIP = {"PSNR", "AMB", "EPCH", "SRCH-COH", "LOSS-COH"}

# Index of the join key (TIME*TAG string, e.g. "105-180005")
JOIN_IDX = 11


def parse_rows(path):
    """Return dict mapping TIME*TAG string to split field list."""
    rows = {}
    with open(path) as fh:
        for line in fh:
            s = line.rstrip()
            if not s or s.startswith("*"):
                continue
            fields = s.split()
            if len(fields) < 20:
                continue
            key = fields[JOIN_IDX]
            rows[key] = fields
    return rows


def compare_files(file4, file3, extra_skip=None, global_atol=0.1,
                  global_rtol=0.0, verbose=False):
    rows4 = parse_rows(file4)
    rows3 = parse_rows(file3)
    skip = DEFAULT_SKIP | (extra_skip or set())

    only4 = sorted(set(rows4) - set(rows3))
    only3 = sorted(set(rows3) - set(rows4))
    common = sorted(set(rows4) & set(rows3))

    for k in only4:
        print(f"WARNING: TIME*TAG={k} present only in {file4}")
    for k in only3:
        print(f"WARNING: TIME*TAG={k} present only in {file3}")

    failures = []
    for key in common:
        f4, f3 = rows4[key], rows3[key]
        for col in COMPARE_COLS:
            if col in skip:
                continue
            idx = COL_IDX[col]
            try:
                v4 = float(f4[idx])
                v3 = float(f3[idx])
            except (IndexError, ValueError):
                continue
            atol = COL_ATOL.get(col) or global_atol
            tol = atol + global_rtol * max(abs(v4), abs(v3), 1e-30)
            diff = abs(v4 - v3)
            if diff > tol:
                failures.append((key, col, v4, v3, diff, tol))
                if verbose:
                    print(f"  FAIL  {key}  {col}: "
                          f"file4={v4}  file3={v3}  diff={diff:.4g}  tol={tol:.4g}")

    if failures:
        print(f"\nFAILED: {len(failures)} comparison(s) exceeded tolerance:")
        for key, col, v4, v3, diff, tol in failures:
            print(f"  {key:12s}  {col:14s}  "
                  f"file4={v4:13g}  file3={v3:13g}  "
                  f"diff={diff:.4g}  tol={tol:.4g}")
        return 1

    active = [c for c in COMPARE_COLS if c not in skip]
    print(f"PASSED: {len(common)} matched rows, "
          f"{len(common) * len(active)} comparisons within tolerance")
    return 0


def main():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("file4", help="fringex4 alist v6 output")
    p.add_argument("file3", help="fringex3 alist v6 output")
    p.add_argument("--skip", nargs="+", default=[], metavar="COL",
                   help="additional column names to exclude from comparison")
    p.add_argument("--atol", type=float, default=0.1,
                   help="fallback absolute tolerance for columns without a "
                        "specific tolerance (default: 0.1)")
    p.add_argument("--rtol", type=float, default=0.0,
                   help="relative tolerance added on top of atol (default: 0.0)")
    p.add_argument("--verbose", "-v", action="store_true",
                   help="print each failing comparison as it is encountered")
    args = p.parse_args()

    sys.exit(compare_files(args.file4, args.file3,
                           extra_skip=set(args.skip),
                           global_atol=args.atol,
                           global_rtol=args.rtol,
                           verbose=args.verbose))


if __name__ == "__main__":
    main()
