# Attack Lab

## Environment Setup (macOS / Colima)

### Prerequisites
```bash
brew install colima docker qemu lima-additional-guestagents
```

### Build & Run Container
- Build the x86_64 image:
```bash
docker build -t csapp-env .
```

- Run the interactive container with ptrace enabled for GDB:
```bash
colima start --arch x86_64 --cpu 2 --memory 4
docker run -it --rm --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -v "$(pwd)":/app -w /app/attacklab csapp-env
```

## Lab Structure

- `target1/ctarget`: Target binary for Code Injection (Phases 1-3)
- `target1/rtarget`: Target binary for Return-Oriented Programming (Phases 4-5)
- `target1/cookie.txt`: Unique 4-byte cookie for this target instance
- `target1/farm.c`: Gadget farm source code for ROP attacks
- `target1/hex2raw`: Converts hex byte strings into raw byte sequences for input

## Execution Notes
- Always run targets with the `-q` flag to prevent attempting to send scores to the CMU grading server:
```bash
./target1/ctarget -q
./target1/rtarget -q
```
- Test exploits by piping `hex2raw` output into the target:
```bash
./target1/hex2raw < exploit.txt | ./target1/ctarget -q
```
