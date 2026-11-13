# C Practice Workspace — How to Use These Exercises

This folder is not a course to read top-to-bottom. It is a **training gym** with five
stations, each built for a different job. The single biggest optimiser for learning C
here is *how you work each file*, not how many files you finish — so the protocol
comes first.

---

## 1. The stations

| Directory | What it is | Job it does for you |
|---|---|---|
| `diagnostic/` | Self-quizzing files (`1.c`–`4.c`) with questions embedded in comments, answers already written out | **Measure** — find what you don't actually know |
| `curriculum/` | K&R exercises and the systems drills **merged into one numbered sequence (01→51)**; `curriculum/README.md` is the authoritative order, `_resources/REMEDIATION.md` carries the Phase A–D repair notes | **Repair + Drill** — one path, duplicates removed |
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
   `make -C curriculum <name> && ./curriculum/<name>`
3. **Run under instrumentation.** Anything involving memory gets run *both* ways:
   `make -C curriculum test-sanitizers` and `valgrind --leak-check=full ./<binary>`.
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
[remediation plan](curriculum/_resources/REMEDIATION.md) was already built from an
earlier calibration; re-validate it against today's misses.

### Phases A–D: work `curriculum/` in ascending number, not K&R chapters linearly
The 01→51 sequence already encodes the remediation phases (K&R fundamentals →
pointer/string drills → memory model & lifetime → multi-file programs → POSIX →
transfer). Files marked **repair target** in `curriculum/README.md` carry concrete
Key Tasks from `_resources/REMEDIATION.md` — those are your acceptance criteria.
If a fresh diagnostic miss points somewhere the numbering doesn't stress, renumber.

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
- **Trigger the bugs on purpose.** In `35_memory_safety_sanitizers.c` and
  `37_valgrind_fundamentals.c`, use the `--trigger-*` flags regularly. Learning to
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
diagnostic list. The numbering in `curriculum/README.md` is a living document —
re-sequence it from your latest misses, not from its original ones.
