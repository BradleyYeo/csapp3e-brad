# Stage 5: Exercise 14 - Hoare Logic and Formal Refactoring

A guide explaining Jimmy Koppel's Three Levels of Software, Hoare triples $\{P\}\ C\ \{Q\}$, loop invariants, the Rule of Consequence, and specification refinement for C and Python developers.

# Conceptual Foundations and Mental Model

## The Three Levels of Software (Jimmy Koppel)
- Software exists simultaneously across three distinct levels of abstraction:
  - Level 3 (Design / Logic): Preconditions $\{P\}$, postconditions $\{Q\}$, and invariants $\{I\}$. Exists independently of programming language or CPU hardware.
  - Level 2 (Code / Implementation): Source syntax, control flow, functions, loops, and data structures.
  - Level 1 (Runtime State): CPU registers, stack frames, heap addresses, and variable values during a single run.
- The Core Insight: Novice developers focus on Level 1 (tweaking code until debug prints look right). Expert software engineers design and refactor at Level 3 (proving logical contracts).

## Hoare Triples: {P} C {Q}
- Formulated by Sir Tony Hoare in 1969 to mathematically reason about program correctness.
- Structure:
  $$\{P\}\ C\ \{Q\}$$
  - $P$ (Precondition): An assertion on program state that MUST be true immediately before command $C$ executes.
  - $C$ (Command): The program statements, function body, or block.
  - $Q$ (Postcondition): An assertion on program state that is GUARANTEED to be true immediately after command $C$ finishes (assuming $P$ held initially).
- Total Correctness vs Partial Correctness:
  - Partial Correctness: If $P$ holds and $C$ terminates, then $Q$ holds.
  - Total Correctness: If $P$ holds, $C$ is guaranteed to terminate and $Q$ holds.

## Loop Invariants: The Heart of Iteration
- A loop invariant $I$ is a logical property that remains true across every iteration of a loop.
- The 3 Invariant Proof Obligations:
  - Initialization: $I$ must be true before the loop starts.
  - Maintenance: If $I$ and loop condition $B$ are true before an iteration, $I$ remains true after executing the loop body.
  - Termination: When the loop terminates ($\neg B$), the invariant $I \land \neg B$ must logically imply the postcondition $Q$.

## Specification Refinement and The Rule of Consequence
- When refactoring code, how do you guarantee you didn't break existing callers?
- The Rule of Consequence:
  $$\frac{P' \implies P \quad \{P\}\ C\ \{Q\} \quad Q \implies Q'}{\{P'\}\ C\ \{Q'\}}$$
- Meaning:
  - You may WEAKEN the precondition ($P' \impliedby P$): Require LESS from callers.
  - You may STRONGLEN the postcondition ($Q \implies Q'$): Guarantee MORE to callers.
  - You can never require more or promise less without breaking callers.

---

# Line-by-Line Code Breakdown

## Decorators: @requires and @ensures

```python
def requires(predicate: Callable[..., bool], description: str) -> Callable[[F], F]:
    def decorator(func: F) -> F:
        @functools.wraps(func)
        def wrapper(*args: Any, **kwargs: Any) -> Any:
            if not predicate(*args, **kwargs):
                raise PreconditionViolationError(...)
            return func(*args, **kwargs)
        return wrapper
    return decorator
```

### Executable Specification Contracts
- Evaluates predicate immediately upon function call entry.
- If $P$ fails, execution halts immediately with `PreconditionViolationError`. The bug is blamed squarely on the caller.
- `@ensures` checks the return value against $Q$ before returning. If $Q$ fails, the bug is blamed squarely on the function implementation.

---

## Function: find_max_subsegment (Loop Invariant Proof)

```python
def find_max_subsegment(nums: List[int]) -> int:
    assert len(nums) > 0  # Precondition {P}

    max_so_far = nums[0]
    current_max = nums[0]

    for i in range(1, len(nums)):
        # Loop Invariant {I}:
        # current_max == max_ending_at(nums, i - 1)
        # max_so_far == max_subsegment(nums[0..i])
        current_max = max(nums[i], current_max + nums[i])
        max_so_far = max(max_so_far, current_max)

    # Postcondition {Q}: max_so_far is optimal over nums[0..len-1]
    return max_so_far
```

### Kadane's Algorithm Invariant
- Maintenance: At each step $i$, `current_max` either extends the previous optimal subarray ending at $i-1$ or starts fresh at $nums[i]$.
- Termination: Upon finishing the range, $i = \text{len}(nums)$, so `max_so_far` covers all subsegments across the entire array.

---

# Memory Layout Visualization

The Three Levels of Software hierarchy:

```
+-------------------------------------------------------------+
| Level 3: Design / Logic (Hidden Layer of Logic)            |
| - Preconditions {P}, Postconditions {Q}, Invariants {I}     |
| - Weaker Precondition, Stronger Postcondition               |
+-------------------------------------------------------------+
                              |
                              v Implemented by
+-------------------------------------------------------------+
| Level 2: Implementation Code                               |
| - Python / C AST, syntax, control flow, functions           |
| - What the program can do across all arbitrary inputs       |
+-------------------------------------------------------------+
                              |
                              v Executes as
+-------------------------------------------------------------+
| Level 1: Runtime Execution State                           |
| - Stack frames, memory addresses, register values           |
| - What happens during a single execution run                |
+-------------------------------------------------------------+
```

---

# Active Recall and Validation Drills

## Drill: Active Recall Questions
- Question 1: What is the Rule of Consequence, and why does safe refactoring allow weakening preconditions and strengthening postconditions?
- Question 2: What are the three formal obligations required to prove a loop invariant?
- Question 3: Why is testing code with random inputs (Level 1) insufficient to prove the absence of edge-case specification bugs (Level 3)?

## Drill: Hands-On Code Validation
- Open [14_hoare_logic_refactoring.py](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/14_hoare_logic_refactoring.py).
- Implement a binary search function with formal loop invariants:
  ```python
  @requires(lambda arr, target: is_sorted(arr), "Array must be sorted")
  @ensures(lambda result: result == -1 or arr[result] == target, "Valid index or not found")
  def binary_search(arr: List[int], target: int) -> int:
      ...
  ```
- Define the loop invariant: `target` is not in `arr[0..low-1]` and not in `arr[high+1..end]`.
- Execute verification:
  ```bash
  python3 14_hoare_logic_refactoring.py
  ```

---

# Next Step in Curriculum
Proceed to [06_mmap_search.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/06_mmap_search.md) (Stage 6) to master virtual file systems, memory mapping (`mmap`), page fault traps, and demand paging.
