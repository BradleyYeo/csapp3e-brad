#include <stdio.h>

char *create_greeting(const char *name) {
  char buffer[64]; // stack
  snprintf(buffer, sizeof(buffer), "Hello, %s!", name);
  return buffer;
}

/*
Explain what happens to the memory backing buffer after create_greeting returns,
and why dereferencing the return value in the caller produces undefined
behavior. Contrast the caller-allocates pattern (passing a destination buffer
and capacity) versus the callee-allocates pattern (malloc). Explain the
maintainability and resource-ownership tradeoffs between caller-allocates and
callee-allocates in C systems programming.
*/

/*
Memory leak is a failure to free allocated memory

When create_greeting returns, stack pointer moves up gets popped. The stack
pointer is now at first address of buf. idk why print will overrwite buffer,
help me visualise.

# Stack Frame Visualization

The stack grows **downward** (from high addresses to low addresses).

## Step 1: Inside `create_greeting("Brad")`
- `SP` moved **down** to allocate `create_greeting`'s frame (`buffer[64]`).

```
High Addr
+------------------------------------+
| main's stack frame                 |
+------------------------------------+ <- main's SP before call
| return address to main             |
| buffer[0..63] = "Hello, Brad!\0"   | <- 0x7ff0..0x7fb0
+------------------------------------+ <- SP (inside create_greeting)
Low Addr
```

## Step 2: `create_greeting` Returns
- `SP` moves **back up** to `main`'s frame.
- The bytes at `0x7ff0..0x7fb0` are not erased yet, but they are now **below
`SP`** (unowned stack space).
- `main` holds `char *ptr = 0x7fb0` (a dangling pointer).

```
High Addr
+------------------------------------+
| main's stack frame (ptr = 0x7fb0)  |
+------------------------------------+ <- SP (back in main)
| (stale: return addr)               |
| (stale: "Hello, Brad!\0")          | <- 0x7fb0 (ptr STILL points here!)
+------------------------------------+
Low Addr
```

## Step 3: `main` Calls `printf("%s\n", ptr)`
- Calling `printf` pushes `printf`'s return address and moves `SP` **down** into
the exact same address range (`0x7ff0..0x7fb0` and below).
- `printf` writes its saved registers and local variables over `0x7ff0..0x7fb0`.

```
High Addr
+------------------------------------+
| main's stack frame (ptr = 0x7fb0)  |
+------------------------------------+
| return address to main             |
| printf's saved registers & locals  | <- overwrites 0x7fb0!
| printf's internal formatting buf   |
+------------------------------------+ <- SP (inside printf)
Low Addr
```

## Result
- When `printf` dereferences `ptr` (`0x7fb0`) to read `%s`, it reads **its own
stack frame bytes** (saved registers / return address / formatting state)
instead of `"Hello, Brad!"`, printing garbage or crashing (`SIGSEGV`).
*/