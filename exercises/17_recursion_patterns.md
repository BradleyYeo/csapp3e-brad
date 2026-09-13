# Conceptual Foundations

## Recursion as Structural Induction
- A recursive function computes a result by decomposing an input into a strictly smaller subproblem of the identical structure.
- Base case: The minimal problem instance that can be solved directly without further decomposition.
- Inductive step: The reduction rule that expresses the solution to the current problem $P(n)$ in terms of the solution to a strictly smaller subproblem $P(n-1)$.
- Well-founded ordering: A guarantee that every sequence of recursive steps strictly decreases a monotonic metric (e.g. string length or pointer distance to `'\0'`) toward the base case, preventing infinite execution.

## Physical Stack Frame Mechanics
- Under the System V AMD64 ABI (Linux/macOS x86_64), each function invocation allocates an activation record (stack frame) on the thread call stack.
- Stack growth: The stack grows downward toward lower memory addresses. Calling a function pushes the return address and decrements the stack pointer (`%rsp`).
- Frame allocation: Local variables and callee-saved registers reside within the frame between the base pointer (`%rbp`) and stack pointer (`%rsp`).
- Parameter passing: The first six integer/pointer arguments are passed in hardware registers (`%rdi`, `%rsi`, `%rdx`, `%rcx`, `%r8`, `%r9`). Subsequent recursive calls overwrite or preserve these registers.
- Frame deallocation (Unwinding): When the base case returns, execution jumps back to the caller's return address, restoring registers and incrementing `%rsp` back up the stack.
- Non-tail recursion vs Tail call optimization: If computation occurs after the recursive call returns (e.g. `1 + rec_strlen(s + 1)`), the caller's frame cannot be discarded before the callee returns.

---

# Exercise 1: Linear Pointer Recursion

## Mental Model and Induction Hypothesis
- Goal: Compute the length of a null-terminated string without iteration loops.
- Monotonic variant: The distance in bytes between pointer cursor `s` and the terminal byte `'\0'`.
- Base case: When `*s == '\0'`, the remaining string has length `0`.
- Inductive hypothesis: Assume `rec_strlen(s + 1)` correctly returns the length of the string starting at `s + 1`.
- Inductive step: The length of the string at `s` is `1 + rec_strlen(s + 1)`.

## Call Stack Trace Diagram
- Trace for `rec_strlen("cat")`:
  ```
  Call 1: rec_strlen(0x1000 "cat")
    |-- s points to 'c' != '\0'
    |-- returns 1 + rec_strlen(0x1001 "at")
          |
          Call 2: rec_strlen(0x1001 "at")
            |-- s points to 'a' != '\0'
            |-- returns 1 + rec_strlen(0x1002 "t")
                  |
                  Call 3: rec_strlen(0x1002 "t")
                    |-- s points to 't' != '\0'
                    |-- returns 1 + rec_strlen(0x1003 "")
                          |
                          Call 4: rec_strlen(0x1003 "") [BASE CASE]
                            |-- *s == '\0' -> returns 0
                    |-- Call 3 computes: 1 + 0 = 1
            |-- Call 2 computes: 1 + 1 = 2
    |-- Call 1 computes: 1 + 2 = 3
  Final Return: 3
  ```

## Hoare Logic Invariants
- Precondition: `s` is a valid pointer to a null-terminated byte sequence in readable memory.
- Postcondition: `result == \sum_{i=0}^{\infty} [s[i] \neq '\0']`.
- Step Invariant: `*s != '\0' \implies rec_strlen(s) == 1 + rec_strlen(s + 1)`.

## Active Recall Self-Testing
- What happens if the check for `*s == '\0'` is omitted or placed after `rec_strlen(s + 1)`?
- Why is passing `++s` as an argument potentially dangerous compared to `s + 1`?
- How many total stack frames exist simultaneously in memory when calculating the length of an $N$-character string?

---

# Exercise 1b: Conditional Linear Pointer Recursion

## Mental Model and Conditional Accumulation
- Goal: Count how many times `target` appears in null-terminated string `s`.
- Base Case: When `*s == '\0'`, return `0`.
- Inductive Step:
  - If `*s == target`: This frame contributes `1`. Total count is `1 + rec_count_char(s + 1, target)`.
  - If `*s != target`: This frame contributes `0`. Total count is `rec_count_char(s + 1, target)`.
- Alternative functional form: `(*s == target ? 1 : 0) + rec_count_char(s + 1, target)`.

## Active Recall Self-Testing
- What is the invariant relating `rec_count_char(s, target)` to `rec_count_char(s + 1, target)`?
- Does the choice between ternary operator `(? :)` and an `if/else` change the stack frame behavior in machine code?

---

# Exercise 1c: Linear Pointer Search with Early Termination

## Mental Model and Short-Circuit Base Cases
- Goal: Determine if `target` exists anywhere in string `s` as a boolean (`true`/`false`).
- Key Difference from 1 and 1b: Does NOT accumulate across the unwinding phase. Instead, it terminates immediately upon finding the first match (short-circuit).
- Base Case 1 (Failure): When `*s == '\0'`, the entire string was traversed without a match. Return `false`.
- Base Case 2 (Success): When `*s == target`, the target is found right here. Return `true` immediately without recursing further.
- Inductive Step: If neither base case triggers, search the remainder: `return rec_contains(s + 1, target)`.
- Bridge to Lockstep Recursion: This introduces boolean pass-through without arithmetic (`+ 1`), directly preparing for Exercise 2 (`rec_starts_with`).

## Active Recall Self-Testing
- Why is no arithmetic (`+ 1`) needed on the return expression in `rec_contains`?
- What is the maximum number of stack frames allocated when searching for `'b'` in `"banana"` versus searching for `'z'`?

---

# Exercise 2: Dual-Pointer Lockstep Recursion

## Mental Model and Two-Pointer Invariant
- Goal: Determine if `text` begins with `prefix` by advancing two pointers in unison.
- Pointer state: `(prefix, text)`. Both advance by 1 byte per step as long as their dereferenced values match.
- Base Cases Precedence:
  - Base Case A (Success): `*prefix == '\0'`. All characters of `prefix` have been matched. Return `true`.
  - Base Case B (Failure): `*text == '\0'` while `*prefix != '\0'`. The text ended before the prefix could be matched. Return `false`.
  - Base Case C (Failure): `*prefix != *text`. Character mismatch at the current cursor. Return `false`.
- Inductive Step: When `*prefix == *text`, return `rec_starts_with(prefix + 1, text + 1)`.

## Call Stack Trace Diagram
- Trace for `rec_starts_with("ca", "cat")`:
  ```
  Call 1: rec_starts_with("ca", "cat")
    |-- *prefix = 'c', *text = 'c' (Match)
    |-- returns rec_starts_with("a", "at")
          |
          Call 2: rec_starts_with("a", "at")
            |-- *prefix = 'a', *text = 'a' (Match)
            |-- returns rec_starts_with("", "t")
                  |
                  Call 3: rec_starts_with("", "t") [BASE CASE A]
                    |-- *prefix == '\0' -> return true
            |-- Call 2 returns true
    |-- Call 1 returns true
  Final Return: true
  ```

## Active Recall Self-Testing
- Why must the check `*prefix == '\0'` be evaluated before checking `*text == '\0'`?
- What does `rec_starts_with("", "")` return, and which branch produces that result?
- If `text` is `"c"` and `prefix` is `"cat"`, which base case halts the recursion and at what call depth?

---

# Exercise 3: Branching Recursion with Backtracking

## Mental Model and Decision Trees
- Goal: Match one or more occurrences of `target` followed immediately by pattern `rest` (the regex pattern `target+rest`).
- Phase 1 (Mandatory First Match):
  - `target+` requires at least one match. If `*text != target` (including when `*text == '\0'`), the pattern cannot match starting at this position. Return `false`.
- Phase 2 (Branching Choice):
  - Having matched one instance of `target`, two valid continuation hypotheses exist:
  - Branch 1 (Greedy Repetition): Consume another `target` from `text + 1`. Call `rec_match_one_or_more(target, rest, text + 1)`.
  - Branch 2 (Fallback / Transition): Do not consume more `target`s; transition immediately to matching `rest` against `text + 1`. Call `rec_starts_with(rest, text + 1)`.
- Short-Circuit Disjunction: Return `Branch 1 || Branch 2`. If Branch 1 succeeds, execution completes. If Branch 1 evaluates to `false`, the runtime backtracks and attempts Branch 2.

## Backtracking Trace Walkthrough
- Trace for `rec_match_one_or_more('a', "at", "aaat")`:
  ```
  Call 1: rec_match_one_or_more('a', "at", "aaat")
    |-- *text == 'a' (Matches target)
    |-- Branch 1: Try rec_match_one_or_more('a', "at", "aat")
          |
          Call 2: rec_match_one_or_more('a', "at", "aat")
            |-- *text == 'a' (Matches target)
            |-- Branch 1: Try rec_match_one_or_more('a', "at", "at")
                  |
                  Call 3: rec_match_one_or_more('a', "at", "at")
                    |-- *text == 'a' (Matches target)
                    |-- Branch 1: Try rec_match_one_or_more('a', "at", "t")
                          |
                          Call 4: rec_match_one_or_more('a', "at", "t")
                            |-- *text == 't' != 'a' -> returns false
                          |
                    |-- Branch 1 failed! BACKTRACK to Call 3.
                    |-- Branch 2: Try rec_starts_with("at", text + 1) -> rec_starts_with("at", "t")
                    |     |-- 'a' != 't' -> returns false
                    |-- Call 3 returns false (both branches exhausted)
                  |
            |-- Branch 1 failed! BACKTRACK to Call 2.
            |-- Branch 2: Try rec_starts_with("at", text + 1) -> rec_starts_with("at", "at")
            |     |-- *prefix = 'a', *text = 'a' -> rec_starts_with("t", "t")
            |     |-- *prefix = 't', *text = 't' -> rec_starts_with("", "")
            |     |-- *prefix == '\0' -> returns true!
            |-- Call 2 Branch 2 succeeded! Call 2 returns true!
    |-- Call 1 Branch 1 succeeded! Call 1 returns true!
  Final Return: true
  ```

## Active Recall Self-Testing
- In the call `rec_match_one_or_more('a', "at", "aaat")`, why does Call 3 fail while Call 2 succeeds on its fallback?
- What would happen if Branch 2 was evaluated before Branch 1? Would `"aaat"` still match `'a'+ "at"`?
- What is the worst-case time complexity of this backtracking scheme when `text` consists of $N$ repetitions of `target` and `rest` is not present?

---

# Deliberate Practice Protocol

## Step-by-Step Implementation Workflow
- Open [17_recursion_patterns.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/17_recursion_patterns.c).
- Implement `rec_strlen` first. Remove the placeholder return and implement the base case and recursive step.
- Verify Exercise 1 by running:
  ```bash
  clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 17_recursion_patterns 17_recursion_patterns.c && ./17_recursion_patterns
  ```
- Proceed to implement `rec_starts_with`, run the test suite, and verify Exercise 2 passes.
- Implement `rec_match_one_or_more` using `rec_starts_with` as the fallback branch, run the test suite, and verify all tests pass.
