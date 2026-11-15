#include<stdio.h>
#include<time.h>

int main() {
    int a [5] = {1,2,3,4,5};
    clock_t start, end;
    start = clock();
    binarySearch(4,5,a);
    end = clock();
    printf("start %ld", start);
    printf("end %ld", end);
    printf("Clock time in seconds is %lf", ((double)end - start)/CLOCKS_PER_SEC);

    return 0;
}

int binarySearch(int x, int n, int a[]) {
    int low = 0;
    int high = n - 1;

    

    return -1;
}

