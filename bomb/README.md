This is an x86-64 bomb for self-study students.

## Running the Bomb Lab on M1 Mac

Because M1 Macs use ARM64 architecture, the bomb binary (which is compiled for x86-64 Linux) won't run natively. We have provided a Dockerfile to emulate an x86-64 environment using Rosetta 2.

### Docker Setup
```
brew install colima docker qemu lima-additional-guestagents
```

# Build image in colima docker
- Run Once: `docker build -t bomb-lab .`

```bash
colima start --arch x86_64 --cpu 2 --memory 4
docker run -it --rm --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -v "$(pwd)":/app bomb-lab
```

# Phase 1

## Setup and Debugging

Once inside the container, `gdb` is already installed. Start the debugger with your bomb:

```bash
gdb ./bomb
```

Inside `gdb`, use these commands to set up for Phase 1:

```gdb
break phase_1
run ./answer.txt
disas phase_1
x/s 0x402400 # Examine memory `x` at 0x402400 as a string `s` in gdb
```

- When reversing, it helps to start reading from `explode_bomb` and work your way up to understand what triggers failure.

## Analyzing the Assembly

```
Dump of assembler code for function phase_1:
=> 0x0000000000400ee0 <+0>:     sub    $0x8,%rsp
   0x0000000000400ee4 <+4>:     mov    $0x402400,%esi
   0x0000000000400ee9 <+9>:     call   0x401338 <strings_not_equal>
   0x0000000000400eee <+14>:    test   %eax,%eax
   0x0000000000400ef0 <+16>:    je     0x400ef7 <phase_1+23>
   0x0000000000400ef2 <+18>:    call   0x40143a <explode_bomb>
   0x0000000000400ef7 <+23>:    add    $0x8,%rsp
   0x0000000000400efb <+27>:    ret
```

- The code prepares arguments for `strings_not_equal`.
- In the Linux x86-64 calling convention, your input string is automatically passed in `%rdi` as the first argument.
- The second argument is passed in `%esi` (or `%rsi`). The instruction `mov $0x402400,%esi` puts a memory address into this register.
- Inspecting this memory address with `x/s 0x402400` reveals the target string.
- `strings_not_equal` compares your input string to the secret string. It returns `0` in `%eax` if they match.
- `test %eax,%eax` checks if the result is zero. If it is (`je`), it jumps over `explode_bomb` to safety.

# Phase 2

## Reading Inputs

```zsh
=> 0x0000000000400efc <+0>:     push   %rbp
   0x0000000000400efd <+1>:     push   %rbx
   0x0000000000400efe <+2>:     sub    $0x28,%rsp
   0x0000000000400f02 <+6>:     mov    %rsp,%rsi
   0x0000000000400f05 <+9>:     call   0x40145c <read_six_numbers>
```

- `read_six_numbers` indicates exactly six numbers are expected as input.
- The stack pointer (`%rsp`) is passed as the second argument (`%rsi`), meaning your six numbers are stored in an array on the stack starting at `%rsp`.
- You can print all 6 numbers as decimal integers at any time using `x/6wd $rsp`.

## Initial Check

```zsh
   0x0000000000400f0a <+14>:    cmpl   $0x1,(%rsp)
   0x0000000000400f0e <+18>:    je     0x400f30 <phase_2+52>
   0x0000000000400f10 <+20>:    call   0x40143a <explode_bomb>
```

- `cmpl $0x1,(%rsp)` compares the first element of your array (at `%rsp`) to 1.
- If it is equal to 1, it jumps to `<+52>` (`je`), bypassing the first `explode_bomb`.
- Therefore, the first number in the sequence must be 1.

## Loop Setup

```zsh
   0x0000000000400f30 <+52>:    lea    0x4(%rsp),%rbx
   0x0000000000400f35 <+57>:    lea    0x18(%rsp),%rbp
   0x0000000000400f3a <+62>:    jmp    0x400f17 <phase_2+27>
```

- After jumping to `<+52>`, the code sets up a loop.
- `%rbx` is set to `0x4(%rsp)` (the address of the *second* number in your array).
- `%rbp` is set to `0x18(%rsp)` (the address just past the *sixth* number, acting as the loop boundary).
- It then jumps back to `<+27>` to begin the loop.

## The For Loop

```zsh
   0x0000000000400f17 <+27>:    mov    -0x4(%rbx),%eax
   0x0000000000400f1a <+30>:    add    %eax,%eax
   0x0000000000400f1c <+32>:    cmp    %eax,(%rbx)
   0x0000000000400f1e <+34>:    je     0x400f25 <phase_2+41>
   0x0000000000400f20 <+36>:    call   0x40143a <explode_bomb>
   0x0000000000400f25 <+41>:    add    $0x4,%rbx
   0x0000000000400f29 <+45>:    cmp    %rbp,%rbx
   0x0000000000400f2c <+48>:    jne    0x400f17 <phase_2+27>
   0x0000000000400f2e <+50>:    jmp    0x400f3c <phase_2+64>
```

- `mov -0x4(%rbx),%eax`: Moves the previous number in the array into `%eax`. (Since `%rbx` points to the current number, `-0x4(%rbx)` points to the previous one).
- `add %eax,%eax`: Doubles the value in `%eax`.
- `cmp %eax,(%rbx)`: Compares this doubled value to the current number.
- `je 0x400f25`: If they match, skip the bomb and continue.
- `add $0x4,%rbx`: Advance `%rbx` to the next number in the array.
- `cmp %rbp,%rbx` / `jne`: Loop until `%rbx` reaches the boundary set in `%rbp`.
- **Conclusion**: Each number must be twice the previous number. The sequence is `1 2 4 8 16 32`.

### Debugging Tip
- Track loop registers by placing a breakpoint at `<+27>` (`b *0x400f17`).
- Check what your pointers are holding using `info registers rbx rax`.

# Phase 3

## Debugging Setup

- Set breakpoints with `b phase_3` and `b explode_bomb`.
- Run your input file: `run answer.txt`.
- View the assembly: `disas phase_3`.

## Input Formatting

```bash
0x0000000000400f47 <+4>:     lea    0xc(%rsp),%rcx
0x0000000000400f4c <+9>:     lea    0x8(%rsp),%rdx
0x0000000000400f51 <+14>:    mov    $0x4025cf,%esi
```

- These instructions prepare arguments before reading input.
- Running `x/s 0x4025cf` in gdb reveals the format string `"%d %d"`. This confirms `phase_3` expects two integer numbers.
- The first number is stored at `0x8(%rsp)` and the second at `0xc(%rsp)`.

## Input Validation

```bash
0x0000000000400f5b <+24>:    call   0x400bf0 <__isoc99_sscanf@plt>
0x0000000000400f60 <+29>:    cmp    $0x1,%eax
0x0000000000400f63 <+32>:    jg     0x400f6a <phase_3+39>
0x0000000000400f65 <+34>:    call   0x40143a <explode_bomb>
```

- `sscanf` returns the number of items matched.
- `%eax` must be greater than 1 (`jg`) to skip the bomb, meaning you must provide at least 2 numbers.

## The Jump Table (Switch Statement)

```bash
0x0000000000400f6a <+39>:    cmpl   $0x7,0x8(%rsp)
0x0000000000400f6f <+44>:    ja     0x400fad <phase_3+106>
0x0000000000400f71 <+46>:    mov    0x8(%rsp),%eax
0x0000000000400f75 <+50>:    jmp    *0x402470(,%rax,8)
```

- The code compares your first number (`0x8(%rsp)`) to `0x7`.
- If it is Above (`ja`) 7, the bomb explodes. Therefore, the first number must be between 0 and 7 (inclusive).
- The `jmp *0x402470(,%rax,8)` instruction signifies a jump table, which is how C `switch` statements are compiled.
- You can inspect the jump table using `x/8gx 0x402470` to see where each valid input routes to.

## Executing the Switch Case

- If you input `2` as your first number, the CPU calculates the offset and jumps directly to the corresponding case logic (e.g., `<+64>`).

```bash
0x0000000000400f83 <+64>:    mov    $0x2c3,%eax
0x0000000000400f88 <+69>:    jmp    0x400fbe <phase_3+123>
```

- It moves the literal value `0x2c3` (707 in decimal) into `%eax`.
- It then jumps immediately to the final target check at `<+123>`.

## Final Verification

```bash
0x0000000000400fbe <+123>:   cmp    0xc(%rsp),%eax
0x0000000000400fc2 <+127>:   je     0x400fc9 <phase_3+134>
0x0000000000400fc4 <+129>:   call   0x40143a <explode_bomb>
```

- The code compares your second number (`0xc(%rsp)`) against the value that was just loaded into `%eax`.
- For the bomb not to explode, they must be equal (`je`).
- Based on our example switch case, the answer is `2 707`.

# Phase 4

## Input Validation

```bash
0x0000000000401024 <+24>:    call   0x400bf0 <__isoc99_sscanf@plt>
0x0000000000401029 <+29>:    cmp    $0x2,%eax
0x000000000040102c <+32>:    jne    0x401035 <phase_4+41>
0x000000000040102e <+34>:    cmpl   $0xe,0x8(%rsp)
0x0000000000401033 <+39>:    jbe    0x40103a <phase_4+46>
0x0000000000401035 <+41>:    call   0x40143a <explode_bomb>
```

- `sscanf` returns the total number of items successfully read. It must return 2, meaning you need exactly two numbers.
- The first number (at `0x8(%rsp)`) must be less than or equal to `0xe` (14). So, the first number is between 0 and 14 inclusive.

## Setting Up func4

```bash
0x000000000040103a <+46>:    mov    $0xe,%edx
0x000000000040103f <+51>:    mov    $0x0,%esi
0x0000000000401044 <+56>:    mov    0x8(%rsp),%edi
0x0000000000401048 <+60>:    call   0x400fce <func4>
```

- The code prepares arguments for `func4`:
  - Argument 1 (`%edi`): Your first input number
  - Argument 2 (`%esi`): 0 (Low boundary)
  - Argument 3 (`%edx`): 14 (High boundary)

## Understanding func4

- `func4` is a recursive function that performs a binary search.

```bash
0x400fd2 <+4>:  mov    %edx,%eax   ; eax = high
0x400fd4 <+6>:  sub    %esi,%eax   ; eax = high - low
0x400fd6 <+8>:  mov    %eax,%ecx   ; ecx = high - low
0x400fd8 <+10>: shr    $0x1f,%ecx  ; Shifts sign-bit to find if negative (safeguard)
0x400fdb <+13>: add    %ecx,%eax   ; Adjusts for rounding
0x400fdd <+15>: sar    $1,%eax     ; eax = (high - low) / 2  <-- BIT SHIFT IS DIVISION!
0x400fdf <+17>: lea    (%rax,%rsi,1),%ecx ; ecx = ((high - low) / 2) + low
```

### The LEA Instruction

- The `lea` (Load Effective Address) instruction is frequently used by compilers to perform fast math rather than memory lookups.
- The syntax format is: `Offset(Base, Index, Scale)`.
- Mathematically, it calculates: `Result = Base + (Index * Scale) + Offset`.
- In `0x400fdf <+17>: lea (%rax,%rsi,1),%ecx`:
  - Base (`%rax`): Holds `(high - low) / 2`
  - Index (`%rsi`): Holds the `low` boundary (0)
  - Scale (`1`): Multiplier for the index
  - Offset (`0`): Omitted, defaults to 0
- The calculation simplifies to `%ecx = %rax + (%rsi * 1)`.
- This means it calculates the midpoint of our binary search: `((high - low) / 2) + low` and stores it in `%ecx`.

### Recursive Binary Search Logic

```bash
0x0000000000400fe4 <+22>:    jle    0x400ff2 <func4+36>
0x0000000000400fe6 <+24>:    lea    -0x1(%rcx),%edx
0x0000000000400fe9 <+27>:    call   0x400fce <func4>
0x0000000000400fee <+32>:    add    %eax,%eax
0x0000000000400ff0 <+34>:    jmp    0x401007 <func4+57>
0x0000000000400ff2 <+36>:    mov    $0x0,%eax
0x0000000000400ff7 <+41>:    cmp    %edi,%ecx
0x0000000000400ff9 <+43>:    jge    0x401007 <func4+57>
0x0000000000400ffb <+45>:    lea    0x1(%rcx),%esi
0x0000000000400ffe <+48>:    call   0x400fce <func4>
```

- The function compares our input (`%edi`) to the computed midpoint (`%ecx`).
- If they match exactly, the recursion stops and it returns 0.
- Otherwise, it recursively calls itself on the upper or lower half, adding the result to `%eax` and potentially shifting it, returning a non-zero value.

## Final Verification

```bash
0x0000000000401048 <+60>:    call   0x400fce <func4>
0x000000000040104d <+65>:    test   %eax,%eax
0x000000000040104f <+67>:    jne    0x401058 <phase_4+76>
```

- `func4` must return 0 in `%eax` to pass. This only happens if our first number is found at the exact midpoint (7) without recursing into a branch that adds non-zero values to `%eax`.

```bash
0x0000000000401051 <+69>:    cmpl   $0x0,0xc(%rsp)
```

- The second number (at `0xc(%rsp)`) must be equal to 0.
- Therefore, the answer for phase 4 is `7 0`.


# Phase 5

## Learning Objective
- Deconstruct how characters are represented in memory.
- Understand how bitwise operations mask data.
- Learn how lookup tables map input space to output space.

## Step 1: Input Length Verification
- The assembly code validating input length:
```bash
0x000000000040107a <+24>:    call   0x40131b <string_length>
0x000000000040107f <+29>:    cmp    $0x6,%eax
0x0000000000401082 <+32>:    je     0x4010d2 <phase_5+112>
```
- Analysis:
  - Calls [string_length](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/bomb/bomb.asm) to count characters.
  - Compares the length in `%eax` to `0x6`.
  - Jumps to the loop initialization if length is exactly 6.
- Key Takeaway: The input string must be exactly 6 characters long.

## Step 2: Character Retrieval and Stack Buffering
- The instructions fetching and loading characters:
```bash
0x000000000040108b <+41>:    movzbl (%rbx,%rax,1),%ecx
0x000000000040108f <+45>:    mov    %cl,(%rsp)
0x0000000000401092 <+48>:    mov    (%rsp),%rdx
```
- Analysis:
  - [movzbl](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/bomb/bomb.asm) fetches the character at index `%rax` from the input string starting at `%rbx`, zero-extending it to 32 bits in `%ecx`.
  - [mov](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/bomb/bomb.asm) writes the 1-byte character from `%cl` onto the stack at address `(%rsp)`.
  - The next [mov](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/bomb/bomb.asm) loads 8 bytes from `(%rsp)` into the 64-bit register `%rdx`.

## Step 3: Bitwise Masking
- The instruction extracting character bits:
```bash
0x0000000000401096 <+52>:    and    $0xf,%edx
```
- Analysis:
  - Performs a bitwise AND on register `%edx` with `0xf` (`0000 1111` in binary).
  - This strips the upper bits of the character's ASCII byte, keeping only the lowest 4 bits.
  - Mathematically, this is equivalent to `ASCII_value % 16`.
  - Consequently, multiple different characters will produce the exact same 4-bit index.

## Step 4: Table Indexing
- The instruction mapping indices to target characters:
```bash
0x0000000000401099 <+55>:    movzbl 0x4024b0(%rdx),%edx
```
- Analysis:
  - Treats the masked value in `%rdx` as an offset/index into the character array at memory address `0x4024b0`.
  - Inspecting this address reveals the 16-character string: `"maduiersnfotvbyl"`.

## Step 5: Final Verification
- The verification assembly instructions:
```bash
0x00000000004010b3 <+81>:    mov    $0x40245e,%esi
0x00000000004010b8 <+86>:    lea    0x10(%rsp),%rdi
0x00000000004010bd <+91>:    call   0x401338 <strings_not_equal>
0x00000000004010c2 <+96>:    test   %eax,%eax
0x00000000004010c4 <+98>:    je     0x4010d9 <phase_5+119>
```
- Analysis:
  - Prepares to compare the transformed string at `0x10(%rsp)` with the target string at address `0x40245e`.
  - Inspecting memory at `0x40245e` (`x/s 0x40245e`) reveals the target string: `"flyers"`.
  - The function [strings_not_equal](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/bomb/bomb.asm) must return 0 in `%eax` to bypass the bomb-exploding branch.
  - Therefore, the reconstructed string must equal `"flyers"`.

## Step 6: Why the ASCII Table is Required
- The instruction `and $0xf,%edx` filters out all but the lowest 4 bits of the character's binary value.
- The computer does not understand characters like 'a' or 'i' directly; it represents them using standard ASCII integer mappings.
- For example, lowercase letters range from `0x61` ('a') to `0x7a` ('z').
- To retrieve the correct target characters, we map them back to their indices in the table `"maduiersnfotvbyl"` and find characters with matching lower 4 bits.

## Step 7: Step-by-Step Translation of "ionefg" to "flyers"
- Goal: Pass `"ionefg"` as input to produce the target string `"flyers"`.
- Lookup Table: `0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15` -> `"m a d u i e r s n f o  t  v  b  y  l"`
- Walkthrough:
  - Input character 1: 'i'
    - ASCII hex value: `0x69`
    - Apply `and $0xf`: `0x69 & 0x0f = 0x09` (decimal 9)
    - Lookup table value at index 9: 'f'
  - Input character 2: 'o'
    - ASCII hex value: `0x6f`
    - Apply `and $0xf`: `0x6f & 0x0f = 0x0f` (decimal 15)
    - Lookup table value at index 15: 'l'
  - Input character 3: 'n'
    - ASCII hex value: `0x6e`
    - Apply `and $0xf`: `0x6e & 0x0f = 0x0e` (decimal 14)
    - Lookup table value at index 14: 'y'
  - Input character 4: 'e'
    - ASCII hex value: `0x65`
    - Apply `and $0xf`: `0x65 & 0x0f = 0x05` (decimal 5)
    - Lookup table value at index 5: 'e'
  - Input character 5: 'f'
    - ASCII hex value: `0x66`
    - Apply `and $0xf`: `0x66 & 0x0f = 0x06` (decimal 6)
    - Lookup table value at index 6: 'r'
  - Input character 6: 'g'
    - ASCII hex value: `0x67`
    - Apply `and $0xf`: `0x67 & 0x0f = 0x07` (decimal 7)
    - Lookup table value at index 7: 's'
- Output Result: The final transformed string is `"flyers"`, which matches the target string and passes the phase.

# Phase 6

## Input Validation and Structure

```bash
0x0000000000401106 <+18>:    call   0x40145c <read_six_numbers>
```

- Like Phase 2, this phase expects six integers.

```bash
0x0000000000401117 <+35>:    mov    0x0(%r13),%eax
0x000000000040111b <+39>:    sub    $0x1,%eax
0x000000000040111e <+42>:    cmp    $0x5,%eax
0x0000000000401121 <+45>:    jbe    0x401128 <phase_6+52>
0x0000000000401123 <+47>:    call   0x40143a <explode_bomb>
```

- Each number is checked to see if subtracting 1 leaves it below or equal to 5 (using unsigned jump `jbe`).
- This means every number must be between 1 and 6.

```bash
0x0000000000401138 <+68>:    mov    (%rsp,%rax,4),%eax
0x000000000040113b <+71>:    cmp    %eax,0x0(%rbp)
0x000000000040113e <+74>:    jne    0x401145 <phase_6+81>
0x0000000000401140 <+76>:    call   0x40143a <explode_bomb>
```

- An inner loop compares each number against the subsequent numbers.
- If any two numbers are equal, the bomb explodes.
- **Conclusion**: The input must be a permutation of the numbers 1 through 6 with no repeats.

## Data Transformation

```bash
0x0000000000401160 <+108>:   mov    %ecx,%edx
0x0000000000401162 <+110>:   sub    (%rax),%edx
0x0000000000401164 <+112>:   mov    %edx,(%rax)
```

- `%ecx` holds `7`. This loop subtracts each of our inputs from 7.
- For example, if we input `1`, it becomes `7 - 1 = 6`.

## Linked List Traversal

```bash
0x0000000000401183 <+143>:   mov    $0x6032d0,%edx
```

- The value `0x6032d0` is placed into `%edx`. Inspecting this memory address in `gdb` reveals a linked list structure containing integer values.
- A complex set of loops reads each node of this linked list and rearranges it into an array of pointers on the stack based on our transformed input sequence.

## Final Verification

```bash
0x00000000004011df <+235>:   mov    0x8(%rbx),%rax
0x00000000004011e3 <+239>:   mov    (%rax),%eax
0x00000000004011e5 <+241>:   cmp    %eax,(%rbx)
0x00000000004011e7 <+243>:   jge    0x4011ee <phase_6+250>
0x00000000004011e9 <+245>:   call   0x40143a <explode_bomb>
```

- This final loop iterates through the newly arranged pointers.
- It compares the integer value at each node (`(%rax)`) with the integer value at the current node (`(%rbx)`).
- `jge` ensures the current node is greater than or equal to the next node.
- **Conclusion**: We need to sort the linked list in descending order. By examining the values of the nodes at `0x6032d0`, we can determine their descending sequence, map them to their original indices, and reverse the `7 - x` transformation to find the correct input.