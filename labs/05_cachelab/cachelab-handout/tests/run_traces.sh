#!/usr/bin/env bash
#
# run_traces.sh - Stage 5 end-to-end tests for ./csim.
#
# Expected counts come from the x86-64 reference simulator (csim-ref), so
# these tests run natively on any host without needing the Linux binary.
#
set -u
cd "$(dirname "$0")/.."

pass=0
fail=0

expect_summary() {
  local args="$1" expected="$2"
  local actual
  actual=$(./csim $args 2>&1 | tail -n 1)
  if [[ "$actual" == "$expected" ]]; then
    echo "  PASS ./csim $args"
    pass=$((pass + 1))
  else
    echo "  FAIL ./csim $args"
    echo "       expected: $expected"
    echo "       actual:   $actual"
    fail=$((fail + 1))
  fi
}

expect_verbose() {
  local args="$1" expected="$2"
  local actual
  # Strip trailing whitespace so "miss " and "miss" compare equal.
  actual=$(./csim $args 2>&1 | sed 's/[[:space:]]*$//')
  if [[ "$actual" == "$expected" ]]; then
    echo "  PASS ./csim $args (verbose)"
    pass=$((pass + 1))
  else
    echo "  FAIL ./csim $args (verbose)"
    diff <(echo "$expected") <(echo "$actual") | sed 's/^/       /'
    fail=$((fail + 1))
  fi
}

echo "Stage 5: end-to-end traces"

# Reference-trace cases graded by test-csim (27 points).
expect_summary "-s 1 -E 1 -b 1 -t traces/yi2.trace"   "hits:9 misses:8 evictions:6"
expect_summary "-s 4 -E 2 -b 4 -t traces/yi.trace"    "hits:4 misses:5 evictions:2"
expect_summary "-s 2 -E 1 -b 4 -t traces/dave.trace"  "hits:2 misses:3 evictions:1"
expect_summary "-s 2 -E 1 -b 3 -t traces/trans.trace" "hits:167 misses:71 evictions:67"
expect_summary "-s 2 -E 2 -b 3 -t traces/trans.trace" "hits:201 misses:37 evictions:29"
expect_summary "-s 2 -E 4 -b 3 -t traces/trans.trace" "hits:212 misses:26 evictions:10"
expect_summary "-s 5 -E 1 -b 5 -t traces/trans.trace" "hits:231 misses:7 evictions:0"
expect_summary "-s 5 -E 1 -b 5 -t traces/long.trace"  "hits:265189 misses:21775 evictions:21743"

# Instruction loads ('I' lines) must be ignored. If counted, the I 20 line
# would evict block 0x10 and the second L 10 would miss.
expect_summary "-s 0 -E 1 -b 0 -t tests/traces/ignore_instr.trace" "hits:1 misses:1 evictions:0"

# Modify = load + store: one miss then one hit on a cold cache.
expect_summary "-s 0 -E 1 -b 0 -t tests/traces/modify.trace" "hits:1 misses:1 evictions:0"

# Verbose output format (-v), matching the lab writeup example.
expect_verbose "-v -s 4 -E 1 -b 4 -t traces/yi.trace" "L 10,1 miss
M 20,1 miss hit
L 22,1 hit
S 18,1 hit
L 110,1 miss eviction
L 210,1 miss eviction
M 12,1 miss eviction hit
hits:4 misses:5 evictions:3"

echo
echo "$pass passed, $fail failed"
[[ $fail -eq 0 ]]
