#include <assert.h>
#include <stdio.h>
#include <time.h>

int binarySearch(int x, int n, int a[]) {
  int low = 0;
  int high = n - 1;
  while (low <= high) {
    int mid = (low + high) / 2;
    if (a[mid] == x) {
      return mid;
    } else if (a[mid] < x) {
      low = mid + 1;
    } else if (a[mid] > x) {
      high = mid - 1;
    }
  }
  return -1;
}

int main() {
  int a[5] = {1, 2, 3, 4, 5};
  int evens[4] = {2, 4, 6, 8};
  int single[1] = {42};

  /* Core & boundary test cases */
  assert(binarySearch(4, 5, a) == 3);  /* catches returning 0 instead of mid */
  assert(binarySearch(1, 5, a) == 0);  /* first element */
  assert(binarySearch(5, 5, a) == 4);  /* last element: catches low < high & low = mid hang */
  assert(binarySearch(0, 5, a) == -1); /* missing below min */
  assert(binarySearch(6, 5, a) == -1); /* missing above max */
  assert(binarySearch(5, 4, evens) == -1); /* missing in middle */
  assert(binarySearch(42, 1, single) == 0); /* single-element hit: catches low < high */
  assert(binarySearch(7, 1, single) == -1); /* single-element miss */
  assert(binarySearch(4, 0, a) == -1);      /* empty array */

  clock_t start, end;
  start = clock();
  assert(binarySearch(4, 5, a) == 3);
  end = clock();
  printf("start %ld\n", start);
  printf("end %ld\n", end);
  printf("Clock time in seconds is %lf\n",
         ((double)end - start) / CLOCKS_PER_SEC);
  printf("[PASS] 18_kr301_binsearch_count tests passed.\n");

  return 0;
}
