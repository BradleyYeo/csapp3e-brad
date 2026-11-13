# C Practice Workspace — How to Use These Exercises

This folder is not a course to read top-to-bottom. It is a **training gym** with five
stations, each built for a different job. The single biggest optimiser for learning C
here is *how you work each file*, not how many files you finish — so the protocol
comes first.

---

## 1. The five stations

| Directory | What it is | Job it does for you |
|---|---|---|
| `diagnostic/` | Self-quizzing files (`1.c`–`4.c`) with questions embedded in comments, answers already written out | **Measure** — find what you don't actually know |
| `Exercises/` | K&R *The C Programming Language* Ch. 1–8, plus [`Exercises.md`](Exercises/Exercises.md) — a Phase A–D remediation curriculum targeting your known weak spots | **Repair** — fix the specific misconceptions the diagnostics exposed |
| `c-exercises/` | 20 numbered drills (`.c` + `.md` pairs) with a Makefile and tests; graded curriculum in [`02_README.md`](c-exercises/02_README.md), reading layers in [`01_FUNDAMENTALS.md`](c-exercises/01_FUNDAMENTALS.md) | **Drill** — deliberate practice on pointers, memory, sanitizers, POSIX |
| `c3/` | CSAPP Chapter 3 (machine-level representation) exercises; [`run.sh`](c3/run.sh) compiles a C file to x86-64 assembly | **Translate** — tie C source to what the compiler actually emits |
| `projects/` | `ls_clone` (+ guide with 5 exercises) and `malloc_clone` (5 stages: implicit coalescer → explicit freelist → tcmalloc) | **Integrate** — everything at once, nothing handed to you |

---

## 2. The core loop (do this for every file)

Work in WSL (the tooling here assumes a POSIX environment):

```bash
sudo apt update && sudo apt install -y clang make build-essential valgrind
```

For each exercise, run this cycle — **never skip step 1 or 4**:

1. **Predict before running.** Read the code or the task. Write down (in the file's
   comment block, like you already did in `diagnostic/1.c`) what you expect: output,
   `sizeof` values, whether it compiles, whether it crashes and where.
2. **Implement / fix.** Only now touch the code. Compile:
   `make -C c-exercises <name> && ./c-exercises/<name>`
3. **Run under instrumentation.** Anything involving memory gets run *both* ways:
   `make -C c-exercises test-sanitizers` and `valgrind --leak-check=full ./<binary>`.
   A program that "works" without ASan but fails with it is a failed exercise.
4. **Explain the diff.** Where prediction ≠ reality, don't move on until you can
   write one paragraph explaining the mechanism (stride, decay, segment, lifetime).
   This step is where the learning actually happens.
5. **Log it.** One line per surprise in a `PROGRESS.md` error journal:
   *"thought X, actual Y, because Z"*. This journal becomes your spaced-repetition source.

---

## 3. Session order — the sequence that optimises retention

### Week 0: calibrate
Do all four `diagnostic/*.c` files cold: cover the answers, write your response, grade
yourself. Mark every miss. These misses define your personal curriculum — the
[remediation plan](Exercises/Exercises.md) in `Exercises/Exercises.md` was already
built from an earlier calibration; re-validate it against today's misses.

### Phase A–D: work the remediation curriculum, not the K&R chapters linearly
`Exercises/Exercises.md` sequences the *specific K&R files you own* by concept
(pointer decay → string immutability → bounded ops → stack lifetimes → double
indirection → POSIX I/O → the ls project). Follow it in that order. Each stage ends
with concrete Key Tasks — those are your acceptance criteria.

Run the matching `c-exercises/` drills **in parallel** as the drill layer:

| Remediation phase | Pair these drills |
|---|---|
| A — pointers & memory layout | `01`, `05`, `07`, `08` |
| B — allocation & lifetime | `03`, `09`, `10`, `11`, `13` |
| C — POSIX I/O & processes | `06`, `12`, `15`, `16` |
| D — integration | `14`, then the projects |

### Weekly machine-level pass
Once a week, take one pointer/memory concept and verify it in the compiler's output
via `c3/`: write the C, run `run.sh`, read the assembly until the mental model and
the emitted code agree. This is what converts "I know the rule" into "I know the
mechanism" — the whole point of pairing K&R with CSAPP.

### Endgame: projects last, and only last
Don't open `projects/` until Phases A–C are clean under sanitizers. `ls_clone` proves
your I/O and dynamic-array ownership; `malloc_clone`'s five stages prove your memory
model. Do them without the reference implementation open; consult guides only for
specifications, not solutions.

---

## 4. Spaced re-dos (the retention multiplier)

C misconceptions decay fast, and you've already *solved* many of these files once —
a solved file reads like trivia. So:

- **Redo, don't reread.** Every 3–7 days, delete your working code for 2–3 older
  exercises (keep the file in git; the commits are your answer key) and reimplement
  from the prompt only.
- **Trigger the bugs on purpose.** In `10_memory_safety_sanitizers.c` and
  `13_valgrind_fundamentals.c`, use the `--trigger-*` flags regularly. Learning to
  *read* an ASan report beats learning to avoid needing one.
- **Quiz from the journal.** Turn each `PROGRESS.md` "thought X, actual Y" line into
  a prediction question and re-answer it cold before starting a new session.
- **Escape hatch rule.** Stuck >20 minutes on a Phase A/B file? Read only the
  `.md` guide's *concept* section (never the solution), then retry. Stuck on a
  project? Write the failing case down and move to the drill layer instead —
  don't burn the session.

---

## 5. Definition of done

An exercise is finished when you can, **cold and without the file open**:

1. state the invariant it teaches (e.g. "`memcpy` does not null-terminate"),
2. predict the output/behaviour of a novel variation of it,
3. run it clean under both ASan and valgrind.

Three "no"s in a row on step 1 across a topic = that topic goes back on the
diagnostic list. The curriculum in `Exercises/Exercises.md` is a living document —
rewrite its phases from your latest misses, not from its original ones.
