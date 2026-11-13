#!/usr/bin/env python3
"""
Exercise 14: Hoare Logic and Refactoring via the Hidden Layer of Logic

Teaches:
- Hoare Triples {P} C {Q}: Preconditions, Commands, and Postconditions.
- Loop Invariants (Initialization, Maintenance, Termination/Correctness).
- Rule of Consequence and Specification Refinement (Weaker Pre, Stronger Post).
- Sequential Composition Rule ({P} C1 {R} and {R} C2 {Q}) for Extract Method refactoring.
"""

from __future__ import annotations
from dataclasses import dataclass
from typing import Callable, Any, TypeVar, List, Tuple
import functools

# =====================================================================
# Lightweight Executable Contract Framework
# =====================================================================

class ContractViolationError(AssertionError):
    """Base class for specification contract violations."""
    pass

class PreconditionViolationError(ContractViolationError):
    """Raised when {P} is violated at function entry."""
    pass

class PostconditionViolationError(ContractViolationError):
    """Raised when {Q} is violated at function return."""
    pass

class LoopInvariantViolationError(ContractViolationError):
    """Raised when loop invariant {I} fails during execution."""
    pass


F = TypeVar("F", bound=Callable[..., Any])

def requires(predicate: Callable[..., bool], description: str) -> Callable[[F], F]:
    """
    Decorator enforcing Hoare precondition {P}.
    Evaluated immediately upon function entry.
    """
    def decorator(func: F) -> F:
        @functools.wraps(func)
        def wrapper(*args: Any, **kwargs: Any) -> Any:
            if not predicate(*args, **kwargs):
                raise PreconditionViolationError(
                    f"Precondition failed in {func.__name__}(): {description}\n"
                    f"Args: args={args}, kwargs={kwargs}"
                )
            return func(*args, **kwargs)
        return wrapper  # type: ignore
    return decorator


def ensures(predicate: Callable[[Any], bool], description: str) -> Callable[[F], F]:
    """
    Decorator enforcing Hoare postcondition {Q}.
    Evaluated with the return value upon function exit.
    """
    def decorator(func: F) -> F:
        @functools.wraps(func)
        def wrapper(*args: Any, **kwargs: Any) -> Any:
            result = func(*args, **kwargs)
            if not predicate(result):
                raise PostconditionViolationError(
                    f"Postcondition failed in {func.__name__}(): {description}\n"
                    f"Returned: {result}"
                )
            return result
        return wrapper  # type: ignore
    return decorator


# =====================================================================
# Domain Model: Disjoint Interval Consolidation
# =====================================================================

@dataclass(frozen=True)
class Interval:
    start: int
    end: int

    def __post_init__(self) -> None:
        if self.start >= self.end:
            raise ValueError(f"Invalid interval [{self.start}, {self.end}): start must be < end")

    @property
    def duration(self) -> int:
        return self.end - self.start

    def __repr__(self) -> str:
        return f"[{self.start}, {self.end})"


# Logical Predicates (Level 3: The Hidden Layer of Logic)

def is_valid_interval(iv: Any) -> bool:
    """Predicate: iv is an Interval with start < end."""
    return isinstance(iv, Interval) and iv.start < iv.end

def are_intervals_touching_or_overlapping(a: Interval, b: Interval) -> bool:
    """Predicate: a and b overlap or touch, assuming a.start <= b.start."""
    return b.start <= a.end

def is_sorted_intervals(intervals: List[Interval]) -> bool:
    """Predicate: intervals are ordered monotonically by start time."""
    return all(
        intervals[i].start <= intervals[i + 1].start
        for i in range(len(intervals) - 1)
    )

def is_pairwise_disjoint(intervals: List[Interval]) -> bool:
    """Predicate: no two intervals touch or overlap (sorted strictly)."""
    return all(
        intervals[i].end < intervals[i + 1].start
        for i in range(len(intervals) - 1)
    )


# =====================================================================
# Part 1: The Legacy Implementation (Tangled, Implicit Logic)
# =====================================================================

def legacy_merge_intervals(raw_pairs: List[Tuple[int, int]]) -> List[Tuple[int, int]]:
    """
    Legacy code with implicit assumptions and coupled logic:
    - Assumes inputs are sorted (often crashes or corrupts if not).
    - Mutates state in-place with ad-hoc index tracking.
    - No clear pre/postconditions.
    """
    if not raw_pairs:
        return []

    # Hidden assumption: raw_pairs is sorted!
    merged = [list(raw_pairs[0])]
    for i in range(1, len(raw_pairs)):
        current = raw_pairs[i]
        last = merged[-1]
        if current[0] <= last[1]:
            last[1] = max(last[1], current[1])
        else:
            merged.append(list(current))

    return [tuple(x) for x in merged]


# =====================================================================
# Part 2: Hoare Triples & Deliberate Practice Drills
# =====================================================================

# ---------------------------------------------------------------------
# Drill 1: Two-Interval Consolidation (Primitive Hoare Triple)
#
# Hoare Triple:
# {P}: a.start <= b.start and are_intervals_touching_or_overlapping(a, b)
#  C : merge_two_touching(a, b)
# {Q}: res.start == a.start and res.end == max(a.end, b.end)
# ---------------------------------------------------------------------

@requires(
    lambda a, b: is_valid_interval(a) and is_valid_interval(b) and a.start <= b.start and are_intervals_touching_or_overlapping(a, b),
    "Intervals must be valid, ordered (a.start <= b.start), and touching or overlapping"
)
@ensures(
    lambda res: is_valid_interval(res),
    "Result must be a valid consolidated interval"
)
def merge_two_touching(a: Interval, b: Interval) -> Interval:
    """Merges two touching or overlapping intervals into one."""
    new_start = a.start
    new_end = max(a.end, b.end)
    return Interval(new_start, new_end)


# ---------------------------------------------------------------------
# Drill 2: Loop Invariant Verification in Sorted Collapse
#
# The while/for loop rule:
# {I and B} C {I}
# -----------------------------
# {I} while B do C {I and not B}
#
# Loop Invariant I:
# 1. collapsed is pairwise disjoint and sorted.
# 2. Every interval in input[:cursor] is covered by collapsed.
# 3. If collapsed is non-empty, collapsed[-1] may only overlap with upcoming
#    intervals in input[cursor:], never with earlier ones.
# ---------------------------------------------------------------------

@requires(
    lambda intervals: all(is_valid_interval(x) for x in intervals) and is_sorted_intervals(intervals),
    "Precondition P_collapse: All elements are valid intervals sorted by start time"
)
@ensures(
    lambda result: is_pairwise_disjoint(result),
    "Postcondition Q_collapse: Output intervals are strictly pairwise disjoint"
)
def collapse_sorted_intervals(intervals: List[Interval]) -> List[Interval]:
    """Collapses a pre-sorted list of intervals into minimal disjoint intervals."""
    if not intervals:
        return []

    collapsed: List[Interval] = [intervals[0]]

    # Invariant Verification Hook
    def verify_loop_invariant(curr_idx: int) -> None:
        # Invariant Part 1: collapsed list remains pairwise disjoint
        if not is_pairwise_disjoint(collapsed):
            raise LoopInvariantViolationError(f"Collapsed output is not pairwise disjoint at step {curr_idx}: {collapsed}")
        # Invariant Part 2: start points of collapsed must be monotonically increasing
        if not is_sorted_intervals(collapsed):
            raise LoopInvariantViolationError(f"Collapsed output is not sorted at step {curr_idx}: {collapsed}")

    # Initialization: I holds for first element
    verify_loop_invariant(0)

    # Maintenance: iterate across remaining intervals
    for idx in range(1, len(intervals)):
        candidate = intervals[idx]
        last = collapsed[-1]

        if are_intervals_touching_or_overlapping(last, candidate):
            # Maintenance Step: consolidate last and candidate
            merged = merge_two_touching(last, candidate)
            collapsed[-1] = merged
        else:
            # Maintenance Step: disjoint gap found, append new interval
            collapsed.append(candidate)

        # Invariant holds after each iteration
        verify_loop_invariant(idx)

    # Termination: loop terminates when all intervals processed.
    # At loop exit: (I and not B) implies Q_collapse (pairwise disjoint).
    return collapsed


# ---------------------------------------------------------------------
# Drill 3: Refactoring via Sequential Composition
#
# Hoare Composition Rule:
# {P} C1 {R}  and  {R} C2 {Q}
# ---------------------------
#       {P} C1; C2 {Q}
#
# Stage 1 (C1): Sorting
#   {P_any}: List of arbitrary valid intervals (unsorted)
#   C1     : sort_intervals_by_start(intervals)
#   {R}    : List of valid intervals sorted by start time
#
# Stage 2 (C2): Consolidation
#   {R}    : List of valid intervals sorted by start time
#   C2     : collapse_sorted_intervals(sorted_intervals)
#   {Q}    : Pairwise disjoint intervals covering original domain
# ---------------------------------------------------------------------

@requires(
    lambda intervals: all(is_valid_interval(x) for x in intervals),
    "Precondition P_any: Elements must be valid intervals (order does not matter)"
)
@ensures(
    lambda sorted_res: is_sorted_intervals(sorted_res),
    "Intermediate Assertion R: Intervals are strictly sorted by start"
)
def sort_intervals_by_start(intervals: List[Interval]) -> List[Interval]:
    """Pure Stage 1: Sorts intervals monotonically by start coordinate."""
    return sorted(intervals, key=lambda iv: (iv.start, iv.end))


# ---------------------------------------------------------------------
# Drill 4: Refactoring via Specification Refinement (Rule of Consequence)
#
# Rule of Consequence:
# P_refactored => P_legacy  and  Q_legacy => Q_refactored
# --------------------------------------------------------
#              {P_legacy} C {Q_legacy}  =>  {P_refactored} C' {Q_refactored}
#
# By composing C1 (sorting) and C2 (collapsing):
# - We WEAKEN the precondition: Caller no longer needs to supply pre-sorted data!
#   P_refactored (any valid intervals) is strictly WEAKER than P_sorted (must be sorted).
#   P_sorted => P_refactored.
# - We STRENGTHEN the postcondition:
#   Q_refactored guarantees immutability, pure return, and verified disjointness.
# ---------------------------------------------------------------------

@requires(
    lambda intervals: all(is_valid_interval(x) for x in intervals),
    "Precondition: Any collection of valid Interval objects"
)
@ensures(
    lambda result: is_pairwise_disjoint(result),
    "Postcondition: Consolidated intervals are strictly pairwise disjoint"
)
def consolidate_intervals(intervals: List[Interval]) -> List[Interval]:
    """
    Refactored Engine composed using Hoare Logic:
    1. C1: Sort intervals {P_any} -> {R}
    2. C2: Collapse sorted {R} -> {Q_disjoint}
    """
    sorted_intervals = sort_intervals_by_start(intervals)  # Stage 1: establishes {R}
    return collapse_sorted_intervals(sorted_intervals)      # Stage 2: establishes {Q}


# =====================================================================
# Unit Test and Contract Verification Harness
# =====================================================================

def run_tests() -> None:
    print("Running Exercise 14: Hoare Logic & Refactoring Verification Suite...")

    # Test 1: Primitive Hoare Triple (merge_two_touching)
    i1 = Interval(1, 5)
    i2 = Interval(3, 8)
    merged = merge_two_touching(i1, i2)
    assert merged == Interval(1, 8), f"Expected [1, 8), got {merged}"
    print("  [PASS] Drill 1: Primitive Hoare triple verified.")

    # Test 2: Precondition Enforcement on merge_two_touching
    disjoint_a = Interval(1, 3)
    disjoint_b = Interval(5, 7)
    try:
        merge_two_touching(disjoint_a, disjoint_b)
        assert False, "Should have raised PreconditionViolationError on disjoint intervals!"
    except PreconditionViolationError:
        print("  [PASS] Drill 1: Precondition violation correctly trapped.")

    # Test 3: Loop Invariant Verification in collapse_sorted_intervals
    sorted_input = [Interval(1, 3), Interval(2, 6), Interval(8, 10), Interval(15, 18)]
    collapsed = collapse_sorted_intervals(sorted_input)
    assert collapsed == [Interval(1, 6), Interval(8, 10), Interval(15, 18)]
    print("  [PASS] Drill 2: Loop invariant verified across all iterations.")

    # Test 4: Sequential Composition and Refinement (consolidate_intervals)
    # Notice: Input is deliberately UNSORTED and contains overlapping/touching items.
    unsorted_input = [
        Interval(15, 18),
        Interval(1, 4),
        Interval(2, 6),
        Interval(8, 10),
        Interval(10, 12),  # Touching [8, 10)
    ]
    result = consolidate_intervals(unsorted_input)
    expected = [Interval(1, 6), Interval(8, 12), Interval(15, 18)]
    assert result == expected, f"Expected {expected}, got {result}"
    print("  [PASS] Drill 3 & 4: Sequential composition and refined specification verified.")

    # Test 5: Edge Cases (empty list, singletons)
    assert consolidate_intervals([]) == []
    assert consolidate_intervals([Interval(5, 10)]) == [Interval(5, 10)]
    print("  [PASS] Edge cases (empty, singleton) verified.")

    print("\n[PASS] All Hoare Logic & Refactoring drills passed successfully.")


if __name__ == "__main__":
    run_tests()
