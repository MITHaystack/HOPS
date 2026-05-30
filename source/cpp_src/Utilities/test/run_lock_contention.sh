#!/usr/bin/env bash
#
# Orchestrates multiple LockContentionWorker processes to stress the
# MHO_LockFileHandler write-lock mechanism, then verifies that the assigned
# fringe sequence numbers are unique and contiguous (1..TOTAL). Because each
# holder writes its fringe file before releasing the lock, mutual exclusion +
# correct increment imply exactly one of each number 1..TOTAL.
#
# usage: run_lock_contention.sh <path-to-LockContentionWorker>

set -u

WORKER="${1:?usage: run_lock_contention.sh <path-to-LockContentionWorker>}"

if [ ! -x "$WORKER" ]; then
    echo "FAIL: worker executable not found or not executable: $WORKER"
    exit 1
fi

N=8                    # number of concurrent worker processes
K=5                    # lock acquisitions per worker
TOTAL=$(( N * K ))     # total acquisitions across all workers
PER_WORKER_TIMEOUT=60  # seconds; guards against a hang

run_scenario()
{
    local mode="$1" # legacy | nolegacy
    local tmp lockdir logs rc idx p
    tmp="$(mktemp -d)"
    lockdir="$tmp/lockdir"
    logs="$tmp/logs"
    mkdir -p "$lockdir" "$logs"

    # launch all workers as close to simultaneously as possible
    local pids=()
    for i in $(seq 1 "$N"); do
        ( timeout "$PER_WORKER_TIMEOUT" "$WORKER" "$lockdir" "$K" "$mode" \
              > "$logs/worker_$i.log" 2> "$logs/worker_$i.err" ) &
        pids+=("$!")
    done

    rc=0
    idx=0
    for p in "${pids[@]}"; do
        idx=$(( idx + 1 ))
        if ! wait "$p"; then
            echo "FAIL[$mode]: worker $idx exited non-zero; stderr: $(cat "$logs/worker_$idx.err" 2>/dev/null)"
            rc=1
        fi
    done

    # gather all claimed sequence numbers across every worker
    local seqs count distinct expected actual leftover nfringe
    seqs="$(cat "$logs"/worker_*.log 2>/dev/null | awk '/^SEQ /{print $2}')"
    count="$(printf '%s\n' "$seqs" | grep -c '[0-9]')"
    distinct="$(printf '%s\n' "$seqs" | grep '[0-9]' | sort -n | uniq | wc -l)"

    if [ "$count" -ne "$TOTAL" ]; then
        echo "FAIL[$mode]: expected $TOTAL sequence numbers, got $count"
        rc=1
    fi
    if [ "$distinct" -ne "$TOTAL" ]; then
        echo "FAIL[$mode]: sequence numbers are not unique ($distinct distinct of $count)"
        rc=1
    fi

    # contiguity: the sorted unique sequence numbers must be exactly 1..TOTAL
    expected="$(seq 1 "$TOTAL")"
    actual="$(printf '%s\n' "$seqs" | grep '[0-9]' | sort -n | uniq)"
    if [ "$expected" != "$actual" ]; then
        echo "FAIL[$mode]: sequence numbers are not contiguous 1..$TOTAL"
        rc=1
    fi

    # the directory must be left clean (no orphaned lock files)
    leftover="$(find "$lockdir" -maxdepth 1 -name '*.lock' | wc -l)"
    if [ "$leftover" -ne 0 ]; then
        echo "FAIL[$mode]: $leftover orphaned .lock file(s) remain"
        rc=1
    fi

    # exactly TOTAL fringe files must have been created
    if [ "$mode" = "legacy" ]; then
        nfringe="$(find "$lockdir" -maxdepth 1 -name 'GE.X.*.ABCDEF' | wc -l)"
    else
        nfringe="$(find "$lockdir" -maxdepth 1 -name '*.frng' | wc -l)"
    fi
    if [ "$nfringe" -ne "$TOTAL" ]; then
        echo "FAIL[$mode]: expected $TOTAL fringe files, found $nfringe"
        rc=1
    fi

    if [ "$rc" -ne 0 ]; then
        # surface the colliding sequence numbers and the processes that claimed
        # them (with timestamps), then preserve the working tree for inspection
        local dups s
        dups="$(printf '%s\n' "$seqs" | grep '[0-9]' | sort -n | uniq -d)"
        if [ -n "$dups" ]; then
            echo "  [$mode] duplicate sequence number(s): $(echo $dups | tr '\n' ' ')"
            for s in $dups; do
                echo "  [$mode] claimants of seq $s (pid / acquire-time):"
                grep -h "^SEQ $s " "$logs"/worker_*.log | sed 's/^/        /'
            done
        fi
        echo "  [$mode] artifacts preserved for inspection in: $tmp"
    else
        rm -rf "$tmp"
        echo "PASS[$mode]: $N workers x $K iters -> $TOTAL unique contiguous sequence numbers"
    fi
    return "$rc"
}

overall=0
run_scenario legacy   || overall=1
run_scenario nolegacy || overall=1

if [ "$overall" -ne 0 ]; then
    echo "lock contention test FAILED"
    exit 1
fi

echo "lock contention test PASSED"
exit 0
