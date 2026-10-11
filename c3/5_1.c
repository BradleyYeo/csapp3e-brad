/* Swap value x at xp with value y at yp */
void swap(long *xp, long *yp) {
   *xp = *xp + *yp; /* x+y */
   *yp = *xp - *yp; /* x+y-y = x */
   *xp = *xp - *yp; /* x+y-x = y */

}