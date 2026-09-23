#include <stdio.h>

/*print Celsius-Farenheit table
 for fahr = 0, 20, ..., 300*/

int main() {
  float fahr, celsius;
  float lower, upper, step;
  lower = 0;  /* lower limit of temperature scale */
  upper = 20; /* upper limit */
  step = 1;   /* step size */

  celsius = lower;
  // Added table heading
  printf("\tCelsius to Farenheit\n");
  while (celsius <= upper) {
    fahr = ((9.0 / 5.0) * celsius) + 32;
    printf("\t%3.0f %6.3f\n", celsius, fahr);
    celsius = celsius + step;
  }
}
