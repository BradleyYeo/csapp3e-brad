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

Once inside the container, `gdb` is already installed. Start the debugger with your bomb:

```bash
gdb ./bomb
```

Inside `gdb`, use these commands to examine the secret string for Phase 1:

```gdb
break phase_1
run ./answer.txt
disas phase_1
x/s 0x402400 # Examine memory `x` at 0x402400 as a string `s` in gdb
```
- (gdb) run answer.txt
- start reading from exploding bomb and up
- In the Linux x86-64 calling convention, %esi holds the second argument passed to a function. Your input string automatically sits in %rdi or %esirun (the first argument).

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

# Phase 2
- `b phase_2`
- `b explode_bomb`
- For loop

```zsh
Dump of assembler code for function phase_2:
=> 0x0000000000400efc <+0>:     push   %rbp
   0x0000000000400efd <+1>:     push   %rbx
   0x0000000000400efe <+2>:     sub    $0x28,%rsp
   0x0000000000400f02 <+6>:     mov    %rsp,%rsi
   0x0000000000400f05 <+9>:     call   0x40145c <read_six_numbers>
   0x0000000000400f0a <+14>:    cmpl   $0x1,(%rsp)
   0x0000000000400f0e <+18>:    je     0x400f30 <phase_2+52>
   0x0000000000400f10 <+20>:    call   0x40143a <explode_bomb>
   0x0000000000400f15 <+25>:    jmp    0x400f30 <phase_2+52>
   0x0000000000400f17 <+27>:    mov    -0x4(%rbx),%eax
   0x0000000000400f1a <+30>:    add    %eax,%eax
   0x0000000000400f1c <+32>:    cmp    %eax,(%rbx)
   0x0000000000400f1e <+34>:    je     0x400f25 <phase_2+41>
   0x0000000000400f20 <+36>:    call   0x40143a <explode_bomb>
   0x0000000000400f25 <+41>:    add    $0x4,%rbx
   0x0000000000400f29 <+45>:    cmp    %rbp,%rbx
   0x0000000000400f2c <+48>:    jne    0x400f17 <phase_2+27>
   0x0000000000400f2e <+50>:    jmp    0x400f3c <phase_2+64>
   0x0000000000400f30 <+52>:    lea    0x4(%rsp),%rbx
   0x0000000000400f35 <+57>:    lea    0x18(%rsp),%rbp
   0x0000000000400f3a <+62>:    jmp    0x400f17 <phase_2+27>
   0x0000000000400f3c <+64>:    add    $0x28,%rsp
   0x0000000000400f40 <+68>:    pop    %rbx
   0x0000000000400f41 <+69>:    pop    %rbp
   0x0000000000400f42 <+70>:    ret
```

- First explode bomb is here so need to find out the value it must trigger to jump to line 52
`0x0000000000400f0e <+18>:    je     0x400f30 <phase_2+52>`
- read_six_numbers and phase_2 being called six times implies six numbers as input being expected
- `cmpl $0x1,(%rsp)` means compare if value at rsp is equal to 1
- `0x0000000000400f1a <+30>:    add    %eax,%eax` Doubles the value at eax
- `break *0x400f0a` Program is paused at beginning of phase_2
- `continue`
- Since you know your array sits at %rsp, you can print all 6 numbers as decimal integers at any time using `x/6wd $rsp`
- Track loop registers: Put a breakpoint at <+27> (b *0x400f17) and look at what your pointers are holding using `info registers rbx rax`.

# Phase 3
- `b phase_3`
- `b explode_bomb`
- run answer.txt
- `disas phase_3`
```bash
0x0000000000400f5b <+24>:    call   0x400bf0 <__isoc99_sscanf@plt>
0x0000000000400f60 <+29>:    cmp    $0x1,%eax
0x0000000000400f63 <+32>:    jg     0x400f6a <phase_3+39>
0x0000000000400f65 <+34>:    call   0x40143a <explode_bomb>
```
This means eax must be greater than 1 to skip over explode bomb (have 2 or more values).

```bash
0x0000000000400f47 <+4>:     lea    0xc(%rsp),%rcx
0x0000000000400f4c <+9>:     lea    0x8(%rsp),%rdx
0x0000000000400f51 <+14>:    mov    $0x4025cf,%esi
```
First item stored at 0x8, second item at 0xc
Run:
`x/s 0x4025cf`
0x4025cf:       "%d %d"
Shows that phase_3 expects two separate numbers

```bash
0x0000000000400f6a <+39>:    cmpl   $0x7,0x8(%rsp)
0x0000000000400f6f <+44>:    ja     0x400fad <phase_3+106>
0x0000000000400f71 <+46>:    mov    0x8(%rsp),%eax
0x0000000000400f75 <+50>:    jmp    *0x402470(,%rax,8)
```
Jump to the explosion if the value inside 0x8(%rsp) (your first number) is Above the constant $0x7. Meaning first number must be between 0-7 (inclusive). jmp instructions means a jump table is being used and there is a switch instruction in the code.

```bash
0x0000000000400fbe <+123>:   cmp    0xc(%rsp),%eax
0x0000000000400fc2 <+127>:   je     0x400fc9 <phase_3+134>
0x0000000000400fc4 <+129>:   call   0x40143a <explode_bomb>
```
Second number went to 0xC
Inspect Jump table at *0x402470
x/8gx 0x402470

```
0x0000000000400f83 <+64>:    mov    $0x2c3,%eax
0x0000000000400f88 <+69>:    jmp    0x400fbe <phase_3+123>
```
The CPU jumps directly to line <+64>.
It executes mov $0x2c3, %eax, dropping the literal value 0x2c3 into %eax.
It immediately jumps down to the final target check at <+123>.

Answer: 2 707

# Phase 4

```bash
0x0000000000401024 <+24>:    call   0x400bf0 <__isoc99_sscanf@plt>
0x0000000000401029 <+29>:    cmp    $0x2,%eax
0x000000000040102c <+32>:    jne    0x401035 <phase_4+41>
0x000000000040102e <+34>:    cmpl   $0xe,0x8(%rsp)
0x0000000000401033 <+39>:    jbe    0x40103a <phase_4+46>
0x0000000000401035 <+41>:    call   0x40143a <explode_bomb>
```

sscanf returns the total number of items it has read from input string, which means there must be at only 2 numbers in the solution to phase 4.

```bash
0x000000000040102e <+34>:    cmpl   $0xe,0x8(%rsp)
0x0000000000401033 <+39>:    jbe    0x40103a <phase_4+46>
```
First number must be between 0 and 14 (inclusive).

```bash
0x000000000040101f <+19>:    mov    $0x0,%eax
...
0x0000000000401048 <+60>:    call   0x400fce <func4>
0x000000000040104d <+65>:    test   %eax,%eax
0x000000000040104f <+67>:    jne    0x401058 <phase_4+76>
```
func4 must return 0

```bash
0x0000000000401051 <+69>:    cmpl   $0x0,0xc(%rsp)
```
Second number must equal to 0

Run `disas func4`

func4 is recursive as you can see it call itself from inside its own body.
```bash
0x0000000000400fe9 <+27>:    call   0x400fce <func4>
0x0000000000400ffe <+48>:    call   0x400fce <func4>
```

0x8(%rsp) same as scanf is in edi?, 0 in esi, 14 in edx
```bash
0x000000000040103a <+46>:    mov    $0xe,%edx
0x000000000040103f <+51>:    mov    $0x0,%esi
0x0000000000401044 <+56>:    mov    0x8(%rsp),%edi
0x0000000000401048 <+60>:    call   0x400fce <func4>
```

Offset(Base, Index, Scale)
Result = Base + (Index * Scale) + Offset
Result = rax + rsi
%rax (which is just the 64-bit full name for %eax) is 7
%rsi which is esi is 0
ecx is 7
```bash
0x0000000000400fdf <+17>:    lea    (%rax,%rsi,1),%ecx
```

```bash
0x400fd2 <+4>:  mov    %edx,%eax   ; eax = high
0x400fd4 <+6>:  sub    %esi,%eax   ; eax = high - low
0x400fd6 <+8>:  mov    %eax,%ecx   ; ecx = high - low
0x400fd8 <+10>: shr    $0x1f,%ecx  ; Shifts sign-bit to find if negative (safeguard)
0x400fdb <+13>: add    %ecx,%eax   ; Adjusts for rounding
0x400fdd <+15>: sar    $1,%eax     ; eax = (high - low) / 2  <-- BIT SHIFT IS DIVISION!
0x400fdf <+17>: lea    (%rax,%rsi,1),%ecx ; ecx = ((high - low) / 2) + low
```

func4 took your high boundary (14), added your low boundary (0), and divided the whole thing by 2 to get 7 performing binary search.

jle and jge means ecx must equal to 7.
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

# Phase 5

string in phase 5 must be at least 6 characters long.
```bash
0x000000000040107a <+24>:    call   0x40131b <string_length>
0x000000000040107f <+29>:    cmp    $0x6,%eax
0x0000000000401082 <+32>:    je     0x4010d2 <phase_5+112>
```

Grab the first character in a for loop which is 1 byte of data.
```bash
0x000000000040108b <+41>:    movzbl (%rbx,%rax,1),%ecx
0x000000000040108f <+45>:    mov    %cl,(%rsp)
```

Bitwise AND `0xf` which is `0000 1111` or % 16 in C
```bash
0x0000000000401092 <+48>:    mov    (%rsp),%rdx
0x0000000000401096 <+52>:    and    $0xf,%edx
```

Secret characters are hidden in `0x4024b0`
```bash
0x0000000000401099 <+55>:    movzbl 0x4024b0(%rdx),%edx
```

`"maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?"`

line 96 must return 0 to jump to safety, meaning string from loop must equal to `x/s 0x40245e`
```bash
0x00000000004010b3 <+81>:    mov    $0x40245e,%esi
0x00000000004010b8 <+86>:    lea    0x10(%rsp),%rdi
0x00000000004010bd <+91>:    call   0x401338 <strings_not_equal>
0x00000000004010c2 <+96>:    test   %eax,%eax
0x00000000004010c4 <+98>:    je     0x4010d9 <phase_5+119>
```