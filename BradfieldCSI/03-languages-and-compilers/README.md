# Languages and Compilers

## Overview and Curriculum Progression

This module explores language implementation and runtime systems: from lexical scanning, recursive-descent parsing, AST evaluation, and bytecode compilation, to operating system threads, shared memory synchronization, and concurrency patterns.

```
01-ast-eval-and-compiler ──► 02-lexical-scanning ──► 03-parsing ──► 04-threads-and-shared-memory ──► 05-concurrency-patterns
(Expression trees, VM)       (DFA tokens, Lexer)     (Grammar, AST) (POSIX pthreads, race conds) (Mutexes, channels, sync)
```

## Exercise Breakdown

### 1. AST Evaluation and Bytecode Compilation (`01-ast-eval-and-compiler`)
- Directory: [01-ast-eval-and-compiler/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers/01-ast-eval-and-compiler)
- Progression:
  - `01-eval`: Tree-walk evaluation of parsed arithmetic expressions.
  - `02-refactor`: Refactoring AST structures into clean abstractions.
  - `03-compiler`: Lowering high-level AST nodes into linear stack-machine bytecodes executed by a virtual machine.

### 2. Lexical Scanning (`02-lexical-scanning`)
- Directory: [02-lexical-scanning/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers/02-lexical-scanning)
- Concepts: Token classification, finite automata, lookahead characters, handling string and number literals, error reporting with line/column coordinates.

### 3. Parsing and Syntax Trees (`03-parsing`)
- Directory: [03-parsing/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers/03-parsing)
- Concepts: Context-free grammars (BNF/EBNF), operator precedence, left-recursion elimination, recursive-descent parsing.

### 4. Threads and Shared Memory (`04-threads-and-shared-memory`)
- Directory: [04-threads-and-shared-memory/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers/04-threads-and-shared-memory)
- Concepts: POSIX threads (`pthread_create`, `pthread_join`), race conditions in shared memory counters (`badcnt.c`), parallel matrix multiplication with thread pools.

### 5. Concurrency Patterns (`05-concurrency-patterns`)
- Directory: [05-concurrency-patterns/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers/05-concurrency-patterns)
- Concepts: Mutex locks, condition variables, channel synchronization, atomic operations, lock-free patterns.
