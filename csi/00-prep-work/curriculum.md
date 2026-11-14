# Scope and Timeline

- **Window**: 14 Nov 2026 to 4 Jan 2027 (52 days / ~7.5 weeks).
- **Daily Protocol**: Follow the predict -> implement -> sanitize (`make test-sanitizers` + `valgrind`) -> explain-the-diff -> log loop from [c-programming/README.md](c-programming/README.md).
- **Environment**: Build and run C curriculum drills in a Linux/ext4 environment (or WSL) per [c-programming/curriculum/README.md](c-programming/curriculum/README.md) so `make test-sanitizers`, `valgrind`, and inode/pipe drills (`45`–`48`) behave accurately.

## Inventory Summary

- **C Diagnostic**: 4 files in [c-programming/diagnostic/](c-programming/diagnostic).
- **C Curriculum**: 51 numbered items in [c-programming/curriculum/README.md](c-programming/curriculum/README.md) (`01`–`02` marked `DONE` in [c-programming/curriculum/Makefile](c-programming/curriculum/Makefile); 40 `EXERCISES`, 6 `REPAIR` targets, `39_calculator`, and 2 Python transfer scripts `49` and `51`).
- **C Projects**: [ls_clone.c](c-programming/projects/ls_clone.c) (+ 5 exercises in [ls_clone_guide.md](c-programming/projects/ls_clone_guide.md)) and 5 stages in [malloc_clone/](c-programming/projects/malloc_clone).
- **Go & Shell**: Existing implementations in [go-programming/](go-programming) and [wiki.sh](shell-scripting/wiki.sh); requires a verification pass and filling [coverage_test.go](go-programming/Exercises/Chapter7/eval_14/coverage_test.go) (currently 0 lines).

# Week-by-Week Execution Plan

## Week 1 (14 Nov – 20 Nov): Calibration & Stage A (K&R Ch. 1–3)

### Goals
- Read [FUNDAMENTALS.md](c-programming/curriculum/_resources/FUNDAMENTALS.md) and [REMEDIATION.md](c-programming/curriculum/_resources/REMEDIATION.md).
- Complete all 4 [c-programming/diagnostic/](c-programming/diagnostic) files cold; log misses in `PROGRESS.md`.
- Complete Stage A (`01`–`19`), including quick verification of `01`–`03` and fixing repair target `17_kr301_binsearch_count.c`.

### Schedule
- **14 Nov**: Diagnostic `1.c`–`4.c` + `FUNDAMENTALS.md`.
- **15 Nov**: Verify `01`–`05` under the 5-step loop.
- **16 Nov**: `06`–`09` (escape sequences, word-per-line state machine, histogram, reverse line).
- **17 Nov**: `10`–`14` (detab, fold lines, remove comments, `limits.h` type ranges, `htoi`).
- **18 Nov**: `15`–`17` (bitwise ops `setbits`/`rightrot`, fix repair target `17_kr301_binsearch_count.c`).
- **19 Nov**: `18`–`19` (`expand_ranges`, `itob`) + `make test` on Stage A.
- **20 Nov**: Begin Stage B with `20`–`21` (`pointer_traversal`, `anchored_matching`).

## Week 2 (21 Nov – 27 Nov): Stage B (Pointers & Strings, `20`–`28`)

### Goals
- Master pointer traversal, anchored matching, `memcpy` null-termination, and bounded string ops.
- Complete repair target `25_kr503_strcat.c` and write your own test `main` for `26_kr504_strend.c`.

### Schedule
- **21 Nov**: `22`–`25` (`enum_state_machine`, `char_group_slicing`, `strrindex`, repair `25_kr503_strcat.c` per Phase A).
- **22 Nov**: `26`–`28` (`strend` + test `main`, `strn` family, `getch`/`ungetch`) + spaced re-do of 2–3 Stage A files from memory.
- **23–27 Nov**: Buffer / review of any Stage B misses and spaced re-dos from `PROGRESS.md` before entering Stage C.

## Week 3 (28 Nov – 4 Dec): Stage C (Memory Model & Lifetime, `29`–`38`)

### Goals
- Master array decay, pointer strides, process memory segments, heap ownership, ASan/Valgrind, and arena allocation.
- Finish repair targets `29_pointer_decay.c` and `33_kr507_sortlines.c`, plus Stage C capstone `38_kr803_malloc_flushbuf.c`.

### Schedule
- **28–29 Nov**: `29`–`31` (repair `29_pointer_decay.c` with `assert()` strides, `30` strides, `31` memory layout).
- **30 Nov – 1 Dec**: `32`–`34` (`getline`, repair `33_kr507_sortlines.c` with geometric growth, `34` `realloc` lifecycle).
- **2–3 Dec**: `35`–`37` (intentionally trigger all `--trigger-*` flags in `35` and run `make valgrind-37`).
- **4 Dec**: `38_kr803_malloc_flushbuf.c` (stdio buffering + `_allocbuf`).

## Week 4 (5 Dec – 11 Dec): Stage D & Stage E (`39`–`48`)

### Goals
- Complete multi-file encapsulation, recursion, macros, and POSIX systems programming (`read`/`write`, `mmap`, inodes, `pipe`, `fork`/`exec`/`dup2`).
- Finish repair targets `39_calculator/` and `44_kr801_fcopy.c`.

### Schedule
- **5–6 Dec**: `39_calculator/` (encapsulate global stack into heap `Stack` struct) + `40`–`42`.
- **7–8 Dec**: `43`–`45` (`compare_files`, repair `44_kr801_fcopy.c` fd order and partial-write loop, `mmap_search`).
- **9–10 Dec**: `46`–`48` (inodes/hard links/`sbrk`, pipe ping-pong, process pipeline).
- **11 Dec**: Spaced re-do of 2–3 Stage C/D drills + run `make test` and `make test-sanitizers` across `01`–`48`.

## Week 5 (12 Dec – 18 Dec): Stage F, Go/Shell Audit & `ls_clone`

### Goals
- Complete Stage F (`49`–`51`), verify Go and Shell exercises, and complete the [ls_clone.c](c-programming/projects/ls_clone.c) architectural refactor.

### Schedule
- **12–13 Dec**: `49_hoare_logic_refactoring.py`, repair + complete `50_pipelining_hazards.c`, `51_python_pointer_chasing.py`.
- **14 Dec**: Audit [go-programming/](go-programming) (`go test ./...` across `Exercises/` and `xkcd/`), implement [coverage_test.go](go-programming/Exercises/Chapter7/eval_14/coverage_test.go), and test [wiki.sh](shell-scripting/wiki.sh).
- **15–18 Dec**: Complete the 5 exercises in [ls_clone_guide.md](c-programming/projects/ls_clone_guide.md) and refactor [ls_clone.c](c-programming/projects/ls_clone.c) (dynamic vector, UID/GID cache, overflow-safe comparator, `openat`/`fstatat`, clean ASan + Valgrind run).

## Week 6 (19 Dec – 25 Dec): `malloc_clone` Stages 1–3

### Goals
- Build the foundational allocators in [malloc_clone/](c-programming/projects/malloc_clone) with passing `mm_checkheap` invariants and clean sanitizers.

### Schedule
- **19–20 Dec**: `stage1_implicit_coalescer.c` (`mmap` slabs, prologue/epilogue, bit-stealing, 4-case coalescing, `mm_checkheap`).
- **21–22 Dec**: `stage2_explicit_freelist.c` (LIFO/address-ordered explicit freelist, pointer splicing, coalescing).
- **23–25 Dec**: `stage3_tcmalloc_pageheap.c` (page-level spans, radix/page map, span splitting/coalescing).

## Week 7 (26 Dec – 1 Jan): `malloc_clone` Stages 4–5

### Goals
- Complete the multi-threaded `tcmalloc` layers and stress-test concurrency under `-fsanitize=thread` / ASan.

### Schedule
- **26–28 Dec**: `stage4_tcmalloc_central_freelist.c` (size classes, central freelists, span transfer).
- **29 Dec – 1 Jan**: `stage5_tcmalloc_threadcache.c` (thread-local caches, batch refill/release, `-pthread` stress tests).

## Final Buffer & Verification (2 Jan – 4 Jan)

### Goals
- Close any remaining `PROGRESS.md` error-journal items and verify everything builds and passes cleanly.

### Schedule
- **2–3 Jan**: Cold re-do of your weakest 3–4 items from `PROGRESS.md` + full `make test`, `make test-sanitizers`, `make bench`, and `malloc_clone` checks.
- **4 Jan**: Final walkthrough of invariants (memory layout, pointer arithmetic, syscall error handling, allocator invariants) ready for CSAPP.
