long test(long x, long y) {
  // x = rdi y = rsi
  long val = x * 8;
  if (y > 0) {  // testq	%rsi, %rsi
    if (x >= y) //   cmovge %rdx, %rax
      val = y & x; // andq %rsi, %rdx
    else
      val = y - x; // movq %rsi, %rax # %rax = y -> subq %rdi, %rax
  } else if (y <= -2) // cmpq   $-2, %rsi
    val = x + y;      // cmovle %rdi, %rax
  return val;
}