# C Curriculum — Numbered Execution Sequence

One folder, one order: **work the files by ascending number**. The sequence was built
by merging the K&R chapter exercises (`krNNN` = K&R exercise N-NN) with the systems
drills, and deleting duplicate-concept files (earlier drafts of the same exercise,
triplicated helper files, drills that re-taught an identical concept).

Before anything else, read [`_resources/FUNDAMENTALS.md`](_resources/FUNDAMENTALS.md)
(pointer notation, spiral rule, standard types).

Build everything in WSL:

```bash
sudo apt update && sudo apt install -y clang make build-essential valgrind python3
make            # compile all
make test       # run the strict drills + Python suites
make test-sanitizers
```

Single file: `make 20_pointer_traversal && ./20_pointer_traversal`

---

## Stage A — Language foundations (K&R Ch. 1–3)

Control flow, character I/O, arrays, bit arithmetic. All single-file.

| # | File | Concept |
|---|---|---|
| 01 | `01_hello_world.c` | program structure, first compile |
| 02 | `02_kr103_temp_f_to_c.c` | while loops, float formatting |
| 03 | `03_kr105_temp_for_loop.c` | for loops, #define constants |
| 04 | `04_kr106_eof_value.c` | getchar(), EOF, expression values |
| 05 | `05_kr108_count_blanks.c` | character classification, counters |
| 06 | `06_kr110_escape_sequences.c` | switch, multi-char detection |
| 07 | `07_kr112_one_word_per_line.c` | IN/OUT state logic |
| 08 | `08_kr114_char_histogram.c` | arrays as frequency tables, scaled output |
| 09 | `09_kr119_reverse_line.c` | arrays, indices, functions |
| 10 | `10_kr120_detab.c` | column counting, tab stops |
| 11 | `11_kr122_fold_lines.c` | line buffers, wrap logic |
| 12 | `12_kr123_remove_comments.c` | lexical state, quoting edge cases |
| 13 | `13_kr201_type_ranges.c` | limits.h, signedness, overflow |
| 14 | `14_kr203_htoi.c` | base conversion, digit parsing |
| 15 | `15_kr206_setbits.c` | bitwise masking algebra |
| 16 | `16_kr208_rightrot.c` | rotation without modulo |
| 17 | `17_kr301_binsearch_count.c` | **repair target — does not compile**; fix syntax, add comparison counter |
| 18 | `18_kr303_expand_ranges.c` | string scanning, lookahead, boundary cases |
| 19 | `19_kr305_itob.c` | generic base printing, character digits |

## Stage B — Pointers and strings (drills 20–23 + K&R Ch. 4–5)

| # | File | Concept |
|---|---|---|
| 20 | `20_pointer_traversal.{c,md}` | cursor traversal, reverse scans, two-pointer lockstep |
| 21 | `21_anchored_matching.{c,md}` | separate position-predicate from cursor-scanner |
| 22 | `22_enum_state_machine.{c,md}` | enums, status codes — define errors out of existence |
| 23 | `23_char_group_slicing.{c,md}` | pointer subtraction, memcpy null-termination invariant |
| 24 | `24_kr401_strrindex.c` | rightmost substring search |
| 25 | `25_kr503_strcat.c` | **repair target** — remove malloc for in-place concat, `const char *t`, fix `getstrlen` UB (see REMEDIATION Phase A) |
| 26 | `26_kr504_strend.c` | suffix match with end-pointer arithmetic, no `[]` |
| 27 | `27_kr505_strn_family.c` | bounded string ops, length-limit discipline |
| 28 | `28_kr501_getch_ungetch.c` | pushback buffering (`make 28_kr501_getch_ungetch` links `_resources/getch.c`) |

## Stage C — Memory model and lifetime

| # | File | Concept |
|---|---|---|
| 29 | `29_pointer_decay.c` | **repair target** — array decay in parameters, add `assert()` stride checks (REMEDIATION Phase A) |
| 30 | `30_pointer_arithmetic_strides.{c,md}` | type scaling, `void*` hazards, `uintptr_t` |
| 31 | `31_process_memory_layout.{c,md}` | text/data/BSS/heap/stack segments, growth directions |
| 32 | `32_getline_ingestion.{c,md}` | heap-owned input buffers, O(1) newline strip, ownership |
| 33 | `33_kr507_sortlines.c` | **repair target** — replace static `allocbuf` with dynamic allocation, geometric growth (REMEDIATION Phase B) |
| 34 | `34_dynamic_memory_lifecycle.{c,md}` | safe `realloc` idiom, nullify-after-free, deep clone |
| 35 | `35_memory_safety_sanitizers.{c,md}` | ASan/UBSan, `--trigger-*` flags, read the reports |
| 36 | `36_bump_allocator.{c,md}` | arena allocation, power-of-two alignment, scoped rollback |
| 37 | `37_valgrind_fundamentals.{c,md}` | Memcheck A/V-bits, four leak kinds, `make valgrind-37` |
| 38 | `38_kr803_malloc_flushbuf.c` | design stdio buffering + `malloc`/`_allocbuf` — capstone of Stage C |

## Stage D — Multi-file programs, structs, recursion

| # | File | Concept |
|---|---|---|
| 39 | `39_calculator/` | RPN calculator split across files; **repair target** — encapsulate global `stack.c` arrays into a heap `Stack` struct with init/push/pop/free (REMEDIATION Phase B). Build: `clang -std=gnu17 39_calculator/*.c -o calc` |
| 40 | `40_kr602_getword_groups.c` | structs, functions returning `char *`, keyword grouping |
| 41 | `41_recursion_patterns.{c,md}` | recursion structure, base cases, stack depth |
| 42 | `42_macro_fundamentals.{c,md}` | preprocessor, macro hygiene, `do-while(0)` |

## Stage E — Systems and POSIX

| # | File | Concept |
|---|---|---|
| 43 | `43_kr706_compare_files.c` | stdio file handling (`_resources/test1.txt`, `test2.txt` are inputs) |
| 44 | `44_kr801_fcopy.c` | **repair target** — fd order in `fcopy`, loop `write()` for partials (REMEDIATION Phase C) |
| 45 | `45_mmap_search.{c,md}` | zero-copy mapping, bounded scanning, page faults |
| 46 | `46_filesystem_inode_ops.{c,md}` | inodes, hard links, `sbrk` heap break |
| 47 | `47_pipe_pingpong.{c,md}` | full-duplex IPC, descriptor-close invariants, latency measurement |
| 48 | `48_process_pipeline_exec.{c,md}` | fork/exec/waitpid/dup2 pipeline orchestration |

## Stage F — Transfer and reasoning

| # | File | Concept |
|---|---|---|
| 49 | `49_hoare_logic_refactoring.{py,md}` | specs, loop invariants, refinement (`python3 49_hoare_logic_refactoring.py`) |
| 50 | `50_pipelining_hazards.{c,md}` | hazard units vs data/control hazards |
| 51 | `51_python_pointer_chasing.{py,md}` | references vs pointers, transfer check |

**Graduation:** `../projects/ls_clone` and `../projects/malloc_clone` — nothing here
is considered done until those build clean under sanitizers.

---

## `_resources/` (reference, not exercises)

- `FUNDAMENTALS.md` — syntax/reference reading layer
- `DRILLS_INDEX.md` — original tutor-drill curriculum index ⚠️ its exercise numbers
  (01–20) are the *old* numbering; the table above is authoritative
- `REMEDIATION.md` — the Phase A–D repair curriculum; its file paths are stale —
  map `pointers.c`→29, `exercise_5-3.c`→25, `5.c`/`exercise_5-4.c`→27/26,
  `diagnostic/2.c`,`Chapter4/stack.c`→39, `diagnostic/3.c`,`Chapter5/7.c`→33/38,
  `Chapter8/1.c`→44
- `getch.c`, `util.h` — shared pushback helpers (28, 40)
- `test1.txt`, `test2.txt` — inputs for 43/44

## What was deleted (duplicate concepts)

Earlier drafts superseded by a polished version of the *same* exercise (e.g.
`Chapter1/19.c` vs `exercise_1-19.c`), triplicated `getch.c`/`util.h` copies,
1-4/1-5 vs 1-3 temperature-table variants, 1-7 (same EOF concept as 1-6),
1-13 (subsumed by 1-14 histogram), 1-21 entab (same column logic as 1-20/1-22),
2-8 anybase (overlaps 3-5 itob), 3-1 part 2 (duplicate binsearch draft),
4-4 `my_getch` and 5-10 `expr` (both are the calculator again), 4-13 recursive
`itoa` (overlaps 3-5), 4-14 swap macro (overlap with drill 42), 5-11 `#ifdef`
tab-stops (re-does 1-20/1-21), 5-13 `tail` (same line-pointer concept as 5-7),
7-4 minscanf draft, duplicate `03_bump_allocator.md` and `06_valgrind_fundamentals.md`
guides, and scratch `test.c`/`test.txt`.
