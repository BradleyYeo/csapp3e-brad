void cond(long a, long *p) {
  if (p && a > *p)
    *p = a;
}
/*
code:
  void cond(long a, long *p)
  a in %rdi, p in %rsi
cond:
  testq %rsi, %rsi
  je .L1
  cmpq %rdi, (%rsi)
  jge .L1
  movq %rdi, (%rsi)
.L1:
  rep; ret
*/

// goto version
void goto_cond(long a, long *p) {
  if (p == 0)
    goto done;
  if (*p >= a)
    goto done;
  *p = a;
  done:
    return;
  }

void goto_x(long x, long y) {
  if (x == 0)
    goto done;
  if (y >= 10)
    goto done;
  y = 10;
done:
  return;
}