# Binary Encodings of Data

## Mental Models and Theoretical Foundations

### Information Storage and Representation
- At the hardware level, all computer systems store and manipulate information as binary digits (bits).
- Individual bits are grouped into bytes (8 bits = 2 hex nibbles = values $0$ to $255$).
- Hexadecimal notation provides a compact representation: each hex character represents exactly 4 bits (a nibble).

### Signed Integer Representations: Two's Complement
- Unsigned integers: $B2U_w(\vec{x}) = \sum_{i=0}^{w-1} x_i 2^i$.
- Two's complement signed integers: $B2T_w(\vec{x}) = -x_{w-1} 2^{w-1} + \sum_{i=0}^{w-2} x_i 2^i$.
- The most significant bit ($x_{w-1}$) has negative weight ($-2^{w-1}$). For 8-bit numbers, bit 7 has weight $-128$.
- Two's complement negation: to compute $-x$, invert all bits ($\sim x$) and add 1 ($+ 1$).

### Arithmetic Overflow Detection
- Unsigned overflow occurs when addition yields a carry out of the most significant bit.
- Signed overflow occurs when adding two positive numbers yields a negative result, or adding two negative numbers yields a positive result.
- Hardware rule: Signed overflow occurs if and only if the carry into the sign bit differs from the carry out of the sign bit:
  $$\text{Overflow} = \text{CarryIn} \oplus \text{CarryOut}$$

### Byte Ordering: Big-Endian vs. Little-Endian
- In a multi-byte word (such as a 32-bit integer `0x12345678`):
  - **Big-Endian**: Most significant byte is stored at the lowest memory address (`12 34 56 78`). Standard for network protocols (TCP/IP).
  - **Little-Endian**: Least significant byte is stored at the lowest memory address (`78 56 34 12`). Standard for x86-64 and modern ARM processors.

### IEEE 754 Floating Point Standard
- Representation: $V = (-1)^s \times M \times 2^E$.
  - Sign ($s$): 1 bit.
  - Exponent ($e$): 8 bits (single precision) with Bias = $2^{k-1} - 1 = 127$. Actual exponent $E = e - 127$.
  - Fraction / Mantissa ($f$): 23 bits. Normalized significand $M = 1 + f$.
- As the magnitude of floating-point numbers increases, the gap between consecutive representable numbers increases proportionally ($\text{gap} = 2^{E - 23}$).

### Variable-Length Character Encodings (UTF-8)
- ASCII uses 7 bits ($0x00$ to $0x7F$) and maps directly to 1 byte in UTF-8.
- Multi-byte Unicode characters use leading prefix bits to indicate byte length:
  - 1 byte: `0xxxxxxx`
  - 2 bytes: `110xxxxx 10xxxxxx`
  - 3 bytes: `1110xxxx 10xxxxxx 10xxxxxx`
  - 4 bytes: `11110xxx 10xxxxxx 10xxxxxx 10xxxxxx`

## Textbook Reference
- **CS:APP3e**: Chapter 2 (*Representing and Manipulating Information*), Sections 2.1–2.4.

## Problem Set and Exercise Breakdown

### Exercise 1: Hexadecimal Inspection (`hellohex`)
- File: [hellohex](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/02-binary-encodings/hellohex) (17 bytes).
- Question: How many hexadecimal characters are needed to represent a 17-byte file?
- Verification command:
  ```bash
  xxd -p hellohex
  ```

### Exercise 2: Two's Complement Conversion
- Determine the 8-bit two's complement binary representation of decimal values:
  - `127`
  - `-128`
  - `-1`
  - `1`
  - `-14`
- Given binary `0b10000011` and `0b11000100`, compute their decimal values.

### Exercise 3: Byte Ordering and Network Protocols (`9001`, `tcpheader`)
- File [9001](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/02-binary-encodings/9001):
  - Inspect raw bytes: `xxd -p 9001`
  - Determine whether the byte sequence represents 9001 in Big-Endian or Little-Endian.
- File [tcpheader](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/02-binary-encodings/tcpheader):
  - A captured 20-byte TCP header.
  - Parse the bytes according to standard RFC 793 TCP header layout:
    - Bytes 0-1: Source port
    - Bytes 2-3: Destination port
    - Bytes 4-7: Sequence number (32-bit unsigned integer, network byte order / Big-Endian)
    - Bytes 8-11: Acknowledgment number (32-bit unsigned integer)

### Exercise 4: BMP Image File Format (`image1.bmp`, `image2.bmp`)
- Windows BMP headers contain:
  - Offset `0x00`: Signature `'BM'` (`0x42 0x4D`).
  - Offset `0x0A`: 4-byte offset to start of pixel data (Little-Endian).
  - Offset `0x12`: 4-byte image width in pixels.
  - Offset `0x16`: 4-byte image height in pixels.
- Use `xxd` or a custom script to extract the dimensions of [image1.bmp](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/02-binary-encodings/image1.bmp) and [image2.bmp](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/02-binary-encodings/image2.bmp).

### Exercise 5: Unicode and UTF-8 Encodings (`snowman`)
- File [snowman](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/02-binary-encodings/snowman).
- Inspect bytes: `xxd -p snowman`.
- Determine how many bytes the Unicode character `☃` (U+2603) occupies in UTF-8, and decode the binary bit prefixes.

## Active Recall Checkpoint
- Why does two's complement arithmetic allow identical hardware logic for unsigned and signed addition?
- What is the difference between casting a 32-bit int to float vs bitwise interpreting its memory as float?
- In network programming, why must multi-byte integers be converted using `ntohl()` / `htonl()`?
