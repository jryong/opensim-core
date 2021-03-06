/**********************************************************************

 GCVSPL

 Purpose:
 *******

  Natural B-spline data smoothing subroutine, using the Generali-
  zed Cross-Validation and Mean-Squared Prediction Error Criteria
  of Craven & Wahba (1979).

 Acknowledgements:
 **********

  This code was originally implemented in Fortran by Herman Woltring,
  and then translated to C by Dwight Meglan, using the f2c program.
  It was then integrated into the Dynamics Pipeline and tested by
  Kelly Rooney and B.J. Fregly, at the University of Florida.

 Remarks:
 *******

  (1) GVCSPL calculates a natural spline of order 2*M (degree
  2*M-1) which smoothes or interpolates a given set of data
  points, using statistical considerations to determine the
  amount of smoothing required (Craven & Wahba, 1979). If the
  error variance is a priori known, it should be supplied to
  the routine in VAR. The degree of smoothing is then deter-
  mined to minimize an unbiased estimate of the true mean
  squared error. On the other hand, if the error variance is
  not known, VAR should be set to a negative number. The
  routine then determines the degree of smoothing to minimize
  the generalized cross validation function. This is asympto-
  tically the same as minimizing the true mean squared error
  (Craven & Wahba, 1979). In this case, an estimate of the
  error variance is returned in VAR which may be compared
  with any a priori, approximate estimates. In either case,
  an estimate of the true mean square error is returned in
  WK(4).

  (2) The number of arithmetic operations and the amount of
  storage required are both proportional to n, so very large
  datasets may be accomodated. The data points do not have
  to be equidistant in the independant variable X or uniformly
  weighted in the dependant variable Y.

  (3) When VAR is a priori known, any value of N.ge.2*M is
  acceptable. It is advisable, however, for N to be rather
  large (if M.eq.2, about 20) when VAR is unknown. If the
  degree of smoothing done by GCVSPL when VAR is unknown
  is not satisfactory, the user should try specifying the
  degree of smoothing by setting VAR to a reasonable value.

  (4) GCVSPL calculates the spline coefficient array C(N);
  this array can be used to calculate the spline function
  value and any of its derivatives up to the degree 2*M-1
  at any argument T within the knot range, using subroutines
  SPLDER and SEARCH, and the knot array X(N). Since the
  spline is constrained at its Mth derivative, only the
  lower spline derivatives will tend to be reliable estim-
  ates of the underlying signal's true derivatives.

  (5) GCVSPL combines elements of subroutine CRVO5 by Utreras 
  (1980), subroutine SMOOTH by Lyche et al. (1983), and
  subroutine CUBGCV by Hutchinson (1985). The trace of the
  influence matrix is assessed in a similar way as described
  by Hutchinson & de Hoog (1985). The major difference is
  that the present approach utilizes non-symmetrical B-spline
  design matrices as described by Lyche et al. (1983); there-
  fore, the original algorithm by Erisman & Tinney (1975) has
  been used, rather than the symmetrical version adopted by
  Hutchinson & de Hoog.

 References:
 **********

  P. Craven & G. Wahba (1979), Smoothing noisy data with
  spline functions. Numerische Mathematik 31, 377-403.

  A.M. Erisman & W.F. Tinney (1975), On computing certain
  elements of the inverse of a sparse matrix. Communications
  of the ACM 18(3), 177-179.

  M.F. Hutchinson & F.R. de Hoog (1985), Smoothing noisy data
  with spline functions. Numerische Mathematik 47(1) [in print].

  M.F. Hutchinson (1985), Subroutine CUBGCV. CSIRO Division of
  Mathematics and Statistics, P.O. Box 1965, Canberra, ACT 2601,
  Australia.

  T. Lyche, L.L. Schumaker, & K. Sepehrnoori (1983), Fortran
  subroutines for computing smoothing and interpolating natural
  splines. Advances in Engineering Software 5(1), 2-5.

  F. Utreras (1980), Un paquete de programas para ajustar curvas
  mediante funciones spline. Informe Tecnico MA-80-B-209, Depar-
  tamento de Matematicas, Faculdad de Ciencias Fisicas y Matema-
  ticas, Universidad de Chile, Santiago.

**********************************************************************/

#include "f2c.h"

typedef double dbl;
typedef long int inl;

int gcvspl(dbl *, dbl *, inl *, dbl *, dbl *, inl *, inl *,
	 		  inl *, inl *, dbl *, dbl *, inl *, dbl *, inl *);

dbl splder(inl *, inl *, inl *, dbl *, dbl *, dbl *, inl *, dbl *);


/* Table of constant values */

static doublereal c_b6 = 1e-15;

int gcvspl(doublereal *x, doublereal *y, integer *ny,
	        doublereal *wx, doublereal *wy, integer *m, integer *n, integer *k,
	        integer *md, doublereal *val, doublereal *c, integer *nc, doublereal *
	        wk, integer *ier)
{
    static integer m2 = 0;
    static integer nm1 = 0;
    static doublereal el = 0.;

    /* System generated locals */
    integer y_dim1, y_offset, c_dim1, c_offset, i__1;

    /* Local variables */
    static integer nm2m1, nm2p1;
    extern doublereal splc(integer *, integer *, integer *, doublereal *,
	    integer *, doublereal *, doublereal *, integer *, doublereal *,
	    doublereal *, doublereal *, doublereal *, integer *, doublereal *,
	     doublereal *, doublereal *, doublereal *, doublereal *);
    extern /* Subroutine */ int prep(integer *, integer *, doublereal *,
	    doublereal *, doublereal *, doublereal *);
    static integer i, j;
    static doublereal alpha;
    extern /* Subroutine */ int basis(integer *, integer *, doublereal *,
	    doublereal *, doublereal *, doublereal *);
    static doublereal r1, r2, r3, r4;
    static integer ib;
    static doublereal gf2, gf1, gf3, gf4;
    static integer iwe;
    static doublereal err;

    /* Parameter adjustments */
    --wk;

    c_dim1 = *nc;
    c_offset = c_dim1 + 1;
    c -= c_offset;

    --wy;
    --wx;

    y_dim1 = *ny;
    y_offset = y_dim1 + 1;
    y -= y_offset;

    --x;

    /* Function Body */

    *ier = 0;
    if (abs(*md) > 4 || *md == 0 || abs(*md) == 1 && *val < 0. || abs(*md) == 3 &&
       *val < 0. || abs(*md) == 4 && (*val < 0. || *val > (doublereal) (*n - *m)))
    {
       *ier = 3;
       return 0;
    }
    if (*md > 0) {
       m2 = *m << 1;
       nm1 = *n - 1;
    } else {
       if (m2 != *m << 1 || nm1 != *n - 1) {
          *ier = 3;
          return 0;
       }
    }
    if (*m <= 0 || *n < m2) {
       *ier = 1;
       return 0;
    }
    if (wx[1] <= 0.) {
       *ier = 2;
    }
    i__1 = *n;
    for (i = 2; i <= i__1; ++i) {
       if (wx[i] <= 0. || x[i - 1] >= x[i]) {
          *ier = 2;
       }
       if (*ier != 0) {
          return 0;
       }
    }
    i__1 = *k;
    for (j = 1; j <= i__1; ++j) {
       if (wy[j] <= 0.) {
          *ier = 2;
       }
       if (*ier != 0) {
          return 0;
       }
    }


    nm2p1 = *n * (m2 + 1);
    nm2m1 = *n * (m2 - 1);
    ib = nm2p1 + 7;
    iwe = ib + nm2m1;

    if (*md > 0) {
       basis(m, n, &x[1], &wk[ib], &r1, &wk[7]);
       prep(m, n, &x[1], &wx[1], &wk[iwe], &el);
       el /= r1;
    }
    if (abs(*md) != 1) {
       goto L20;
    }
    r1 = *val;
    goto L100;


L20:
    if (*md < -1) {
       r1 = wk[4];
    } else {
       r1 = 1. / el;
    }
    r2 = r1 * 2.;
    gf2 = splc(m, n, k, &y[y_offset], ny, &wx[1], &wy[1], md, val, &r2, &
       c_b6, &c[c_offset], nc, &wk[1], &wk[ib], &wk[iwe], &el, &wk[7]);
L40:
    gf1 = splc(m, n, k, &y[y_offset], ny, &wx[1], &wy[1], md, val, &r1, &
       c_b6, &c[c_offset], nc, &wk[1], &wk[ib], &wk[iwe], &el, &wk[7]);
    if (gf1 > gf2) {
       goto L50;
    }
    if (wk[4] <= 0.) {
       goto L100;
    }
    r2 = r1;
    gf2 = gf1;
    r1 /= 2.;
    goto L40;
L50:
    r3 = r2 * 2.;
L60:
    gf3 = splc(m, n, k, &y[y_offset], ny, &wx[1], &wy[1], md, val, &r3, &
       c_b6, &c[c_offset], nc, &wk[1], &wk[ib], &wk[iwe], &el, &wk[7]);
    if (gf3 > gf2) {
       goto L70;
    }
    if (wk[4] >= 999999999999999.88) {
       goto L100;
    }
    r2 = r3;
    gf2 = gf3;
    r3 *= 2.;
    goto L60;
L70:
    r2 = r3;
    gf2 = gf3;
    alpha = (r2 - r1) / 1.618033983;
    r4 = r1 + alpha;
    r3 = r2 - alpha;
    gf3 = splc(m, n, k, &y[y_offset], ny, &wx[1], &wy[1], md, val, &r3, &
       c_b6, &c[c_offset], nc, &wk[1], &wk[ib], &wk[iwe], &el, &wk[7]);
    gf4 = splc(m, n, k, &y[y_offset], ny, &wx[1], &wy[1], md, val, &r4, &
       c_b6, &c[c_offset], nc, &wk[1], &wk[ib], &wk[iwe], &el, &wk[7]);
L80:
    if (gf3 <= gf4) {
       r2 = r4;
       gf2 = gf4;
       err = (r2 - r1) / (r1 + r2);
       if (err * err + 1. == 1. || err <= 1e-6) {
          goto L90;
       }
       r4 = r3;
       gf4 = gf3;
       alpha /= 1.618033983;
       r3 = r2 - alpha;
       gf3 = splc(m, n, k, &y[y_offset], ny, &wx[1], &wy[1], md, val, &r3, &
          c_b6, &c[c_offset], nc, &wk[1], &wk[ib], &wk[iwe], &el, &wk[7]
          );
    } else {
       r1 = r3;
       gf1 = gf3;
       err = (r2 - r1) / (r1 + r2);
       if (err * err + 1. == 1. || err <= 1e-6) {
          goto L90;
       }
       r3 = r4;
       gf3 = gf4;
       alpha /= 1.618033983;
       r4 = r1 + alpha;
       gf4 = splc(m, n, k, &y[y_offset], ny, &wx[1], &wy[1], md, val, &r4, &
          c_b6, &c[c_offset], nc, &wk[1], &wk[ib], &wk[iwe], &el, &wk[7]
          );
    }
    goto L80;
L90:
    r1 = (r1 + r2) * .5;


L100:
    gf1 = splc(m, n, k, &y[y_offset], ny, &wx[1], &wy[1], md, val, &r1, &c_b6,
               &c[c_offset], nc, &wk[1], &wk[ib], &wk[iwe], &el, &wk[7]);

   return 0;

}



/* BASIS.FOR, 1985-06-03 */

int basis(integer *m, integer *n, doublereal *x, doublereal *b, doublereal *bl,
          doublereal *q)
{
    /* System generated locals */
    integer b_dim1, b_offset, q_offset, i__1, i__2, i__3, i__4;
    doublereal d__1;

    /* Local variables */
    static integer nmip1, i, j, k, l;
    static doublereal u, v, y;
    static integer j1, j2, m2, ir, mm1, mp1;
    static doublereal arg;

    /* Parameter adjustments */
    q_offset = 1 - *m;
    q -= q_offset;

    b_dim1 = *m - 1 - (1 - *m) + 1;
    b_offset = 1 - *m + b_dim1;
    b -= b_offset;

    --x;

    if (*m == 1) {
       i__1 = *n;
       for (i = 1; i <= i__1; ++i) {
          b[i * b_dim1] = 1.;
       }
       *bl = 1.;
       return 0;
    }

    mm1 = *m - 1;
    mp1 = *m + 1;
    m2 = *m << 1;
    i__1 = *n;
    for (l = 1; l <= i__1; ++l) {
       i__2 = *m;
       for (j = -mm1; j <= i__2; ++j) {
          q[j] = 0.;
       }
       q[mm1] = 1.;
       if (l != 1 && l != *n) {
          q[mm1] = 1. / (x[l + 1] - x[l - 1]);
       }
       arg = x[l];
       i__2 = m2;
       for (i = 3; i <= i__2; ++i) {
          ir = mp1 - i;
          v = q[ir];
          if (l < i) {
             i__3 = i;
             for (j = l + 1; j <= i__3; ++j) {
                u = v;
                v = q[ir + 1];
                q[ir] = u + (x[j] - arg) * v;
                ++ir;
             }
          }
          i__3 = l - i + 1;
          j1 = max(i__3,1);
          i__3 = l - 1, i__4 = *n - i;
          j2 = min(i__3,i__4);
          if (j1 <= j2) {
             if (i < m2) {
                i__3 = j2;
                for (j = j1; j <= i__3; ++j) {
                   y = x[i + j];
                   u = v;
                   v = q[ir + 1];
                   q[ir] = u + (v - u) * (y - arg) / (y - x[j]);
                   ++ir;
                }
             } else {
                i__3 = j2;
                for (j = j1; j <= i__3; ++j) {
                   u = v;
                   v = q[ir + 1];
                   q[ir] = (arg - x[j]) * u + (x[i + j] - arg) * v;
                   ++ir;
                }
             }
          }
          nmip1 = *n - i + 1;
          if (nmip1 < l) {
             i__3 = l - 1;
             for(j = nmip1; j <= i__3; ++j) {
                u = v;
                v = q[ir + 1];
                q[ir] = (arg - x[j]) * u + v;
                ++ir;
             }
          }
       }
       i__2 = mm1;
       for (j = -mm1; j <= i__2; ++j) {
          b[j + l * b_dim1] = q[j];
       }
    }

    i__1 = mm1;
    for (i = 1; i <= i__1; ++i) {
       i__2 = mm1;
       for (k = i; k <= i__2; ++k) {
          b[-k + i * b_dim1] = 0.;
          b[k + (*n + 1 - i) * b_dim1] = 0.;
       }
    }

    *bl = 0.;
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
       i__2 = mm1;
       for (k = -mm1; k <= i__2; ++k) {
          *bl += (d__1 = b[k + i * b_dim1], abs(d__1));
       }
    }
    *bl /= *n;

    return 0;
}



/* PREP.FOR, 1985-07-04 */

int prep(integer *m, integer *n, doublereal *x, doublereal *w, doublereal *we,
         doublereal *el)
{
    /* System generated locals */
    integer i__1, i__2, i__3;
    doublereal d__1;

    /* Local variables */
    static doublereal f;
    static integer i, j, k, l;
    static doublereal y, f1;
    static integer i1, i2, m2;
    static doublereal ff;
    static integer jj, jm, kl, nm, ku;
    static doublereal wi;
    static integer n2m, mp1, i2m1, inc, i1p1, m2m1, m2p1;

    /* Parameter adjustments */
    --we;
    --w;
    --x;

    /* Function Body */
    m2 = *m << 1;
    mp1 = *m + 1;
    m2m1 = m2 - 1;
    m2p1 = m2 + 1;
    nm = *n - *m;
    f1 = -1.;
    if (*m != 1) {
       i__1 = *m;
       for (i = 2; i <= i__1; ++i) {
          f1 = -f1 * i;
       }
       i__1 = m2m1;
       for (i = mp1; i <= i__1; ++i) {
          f1 *= i;
       }
    }

    i1 = 1;
    i2 = *m;
    jm = mp1;
    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
       inc = m2p1;
       if (j > nm) {
          f1 = -f1;
          f = f1;
       } else {
          if (j < mp1) {
             inc = 1;
             f = f1;
          } else {
             f = f1 * (x[j + *m] - x[j - *m]);
          }
       }
       if (j > mp1) {
          ++i1;
       }
       if (i2 < *n) {
          ++i2;
       }
       jj = jm;
       ff = f;
       y = x[i1];
       i1p1 = i1 + 1;
       i__2 = i2;
       for (i = i1p1; i <= i__2; ++i) {
          ff /= y - x[i];
       }
       we[jj] = ff;
       jj += m2;
       i2m1 = i2 - 1;
       if (i1p1 <= i2m1) {
          i__2 = i2m1;
          for (l = i1p1; l <= i__2; ++l) {
             ff = f;
             y = x[l];
             i__3 = l - 1;
             for (i = i1; i <= i__3; ++i) {
                ff /= y - x[i];
             }
             i__3 = i2;
             for (i = l + 1; i <= i__3; ++i) {
                ff /= y - x[i];
             }
             we[jj] = ff;
             jj += m2;
          }
       }
       ff = f;
       y = x[i2];
       i__2 = i2m1;
       for (i = i1; i <= i__2; ++i) {
          ff /= y - x[i];
       }
       we[jj] = ff;
       jj += m2;
       jm += inc;
    }

    kl = 1;
    n2m = m2p1 * *n + 1;
    i__1 = *m;
    for (i = 1; i <= i__1; ++i) {
       ku = kl + *m - i;
       i__2 = ku;
       for (k = kl; k <= i__2; ++k) {
          we[k] = 0.;
          we[n2m - k] = 0.;
       }
       kl += m2p1;
    }

    jj = 0;
    *el = 0.;
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
       wi = w[i];
       i__2 = m2p1;
       for (j = 1; j <= i__2; ++j) {
          ++jj;
          we[jj] /= wi;
          *el += (d__1 = we[jj], abs(d__1));
       }
    }
    *el /= *n;

    return 0;
}



/* SPLC.FOR, 1985-12-12 */

doublereal splc(integer *m, integer *n, integer *k, doublereal *y, integer *
	             ny, doublereal *wx, doublereal *wy, integer *mode, doublereal *val,
	             doublereal *p, doublereal *eps, doublereal *c, integer *nc,
	             doublereal *stat, doublereal *b, doublereal *we, doublereal *el,
	             doublereal *bwe)
{
    /* System generated locals */
    integer y_dim1, y_offset, c_dim1, c_offset, b_dim1, b_offset, we_dim1,
	    we_offset, bwe_dim1, bwe_offset, i__1, i__2, i__3, i__4;
    doublereal ret_val, d__1;

    /* Local variables */
    static integer i, j, l;
    extern doublereal trinv(doublereal *, doublereal *, integer *, integer *)
	    ;
    static doublereal dp;
    static integer km;
    static doublereal dt;
    static integer kp;
    extern /* Subroutine */ int bandet(doublereal *, integer *, integer *),
	    bansol(doublereal *, doublereal *, integer *, doublereal *,
	    integer *, integer *, integer *, integer *);
    static doublereal pel, esn, trn;

/* ***  Check on p-value */

    /* Parameter adjustments */
    bwe_dim1 = *m - (-(*m)) + 1;
    bwe_offset = -(*m) + bwe_dim1;
    bwe -= bwe_offset;

    we_dim1 = *m - (-(*m)) + 1;
    we_offset = -(*m) + we_dim1;
    we -= we_offset;

    b_dim1 = *m - 1 - (1 - *m) + 1;
    b_offset = 1 - *m + b_dim1;
    b -= b_offset;

    --stat;

    c_dim1 = *nc;
    c_offset = c_dim1 + 1;
    c -= c_offset;

    --wy;
    --wx;

    y_dim1 = *ny;
    y_offset = y_dim1 + 1;
    y -= y_offset;

    /* Function Body */
    dp = *p;
    stat[4] = *p;
    pel = *p * *el;
    if (pel < *eps) {
       dp = *eps / *el;
       stat[4] = 0.;
    }
    if (pel * *eps > 1.) {
       dp = 1. / (*el * *eps);
       stat[4] = dp;
    }
    
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
       i__2 = *m, i__3 = i - 1;
       km = -min(i__2,i__3);
       i__2 = *m, i__3 = *n - i;
       kp = min(i__2,i__3);
       i__2 = kp;
       for (l = km; l <= i__2; ++l) {
          if (abs(l) == *m) {
             bwe[l + i * bwe_dim1] = dp * we[l + i * we_dim1];
          } else {
             bwe[l + i * bwe_dim1] = b[l + i * b_dim1] + dp * we[l + i *
                we_dim1];
          }
       }
    }

    bandet(&bwe[bwe_offset], m, n);
    bansol(&bwe[bwe_offset], &y[y_offset], ny, &c[c_offset], nc, m, n, k);
    stat[3] = trinv(&we[we_offset], &bwe[bwe_offset], m, n) * dp;
    trn = stat[3] / *n;

    esn = 0.;
    i__1 = *k;
    for (j = 1; j <= i__1; ++j) {
       i__2 = *n;
       for (i = 1; i <= i__2; ++i) {
          dt = -y[i + j * y_dim1];
          i__3 = *m - 1, i__4 = i - 1;
          km = -min(i__3,i__4);
          i__3 = *m - 1, i__4 = *n - i;
          kp = min(i__3,i__4);
          i__3 = kp;
          for (l = km; l <= i__3; ++l) {
             dt += b[l + i * b_dim1] * c[i + l + j * c_dim1];
          }
          esn += dt * dt * wx[i] * wy[j];
       }
    }
    esn /= *n * *k;

    stat[6] = esn / trn;
    stat[1] = stat[6] / trn;
    stat[2] = esn;
    if (abs(*mode) != 3) {
       stat[5] = stat[6] - esn;
       if (abs(*mode) == 1) {
          ret_val = 0.;
       }
       if (abs(*mode) == 2) {
          ret_val = stat[1];
       }
       if (abs(*mode) == 4) {
          ret_val = (d__1 = stat[3] - *val, abs(d__1));
       }
    } else {
       stat[5] = esn - *val * (trn * 2. - 1.);
       ret_val = stat[5];
    }

    return ret_val;
}



/* BANDET.FOR, 1985-06-03 */

int bandet(doublereal *e, integer *m, integer *n)
{
    /* System generated locals */
    integer e_dim1, e_offset, i__1, i__2, i__3, i__4;

    /* Local variables */
    static integer i, k, l;
    static doublereal di, dl;
    static integer mi, km, lm;
    static doublereal du;

    /* Parameter adjustments */

    e_dim1 = *m - (-(*m)) + 1;
    e_offset = -(*m) + e_dim1;
    e -= e_offset;

    /* Function Body */
    if (*m <= 0) {
       return 0;
    }
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
       di = e[i * e_dim1];
       i__2 = *m, i__3 = i - 1;
       mi = min(i__2,i__3);
       if (mi >= 1) {
          i__2 = mi;
          for (k = 1; k <= i__2; ++k) {
             di -= e[-k + i * e_dim1] * e[k + (i - k) * e_dim1];
          }
          e[i * e_dim1] = di;
       }
       i__2 = *m, i__3 = *n - i;
       lm = min(i__2,i__3);
       if (lm >= 1) {
          i__2 = lm;
          for (l = 1; l <= i__2; ++l) {
             dl = e[-l + (i + l) * e_dim1];
             i__3 = *m - l, i__4 = i - 1;
             km = min(i__3,i__4);
             if (km >= 1) {
                du = e[l + i * e_dim1];
                i__3 = km;
                for (k = 1; k <= i__3; ++k) {
                   du -= e[-k + i * e_dim1] * e[l + k + (i - k) * e_dim1]
                      ;
                   dl -= e[-l - k + (l + i) * e_dim1] * e[k + (i - k) *
                      e_dim1];
                }
                e[l + i * e_dim1] = du;
             }
             e[-l + (i + l) * e_dim1] = dl / di;
          }
       }
    }

    return 0;
}



/* BANSOL.FOR, 1985-12-12 */

int bansol(doublereal *e, doublereal *y, integer *ny,
	         doublereal *c, integer *nc, integer *m, integer *n, integer *k)
{
    /* System generated locals */
    integer e_dim1, e_offset, y_dim1, y_offset, c_dim1, c_offset, i__1, i__2, i__3, i__4;

    /* Local variables */
    static doublereal d;
    static integer i, j, l, mi, nm1;

/* ***  Check on special cases: M=0, M=1, M>1 */

    /* Parameter adjustments */

    c_dim1 = *nc;
    c_offset = c_dim1 + 1;
    c -= c_offset;

    y_dim1 = *ny;
    y_offset = y_dim1 + 1;
    y -= y_offset;

    e_dim1 = *m - (-(*m)) + 1;
    e_offset = -(*m) + e_dim1;
    e -= e_offset;

    /* Function Body */
    nm1 = *n - 1;
    if ((i__1 = *m - 1) < 0) {
       goto L10;
    } else if (i__1 == 0) {
       goto L40;
    } else {
       goto L80;
    }

L10:
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
       i__2 = *k;
       for (j = 1; j <= i__2; ++j) {
          c[i + j * c_dim1] = y[i + j * y_dim1] / e[i * e_dim1];
       }
    }
    return 0;

L40:
    i__1 = *k;
    for (j = 1; j <= i__1; ++j) {
       c[j * c_dim1 + 1] = y[j * y_dim1 + 1];
       i__2 = *n;
       for (i = 2; i <= i__2; ++i) {
          c[i + j * c_dim1] = y[i + j * y_dim1] - e[i * e_dim1 - 1] * c[i -
             1 + j * c_dim1];
       }
       c[*n + j * c_dim1] /= e[*n * e_dim1];
       for (i = nm1; i >= 1; --i) {
          c[i + j * c_dim1] = (c[i + j * c_dim1] - e[i * e_dim1 + 1] * c[i
             + 1 + j * c_dim1]) / e[i * e_dim1];
       }
    }
    return 0;


L80:
    i__1 = *k;
    for (j = 1; j <= i__1; ++j) {
       c[j * c_dim1 + 1] = y[j * y_dim1 + 1];
       i__2 = *n;
       for (i = 2; i <= i__2; ++i) {
          i__3 = *m, i__4 = i - 1;
          mi = min(i__3,i__4);
          d = y[i + j * y_dim1];
          i__3 = mi;
          for (l = 1; l <= i__3; ++l) {
             d -= e[-l + i * e_dim1] * c[i - l + j * c_dim1];
          }
          c[i + j * c_dim1] = d;
       }
       c[*n + j * c_dim1] /= e[*n * e_dim1];
       for (i = nm1; i >= 1; --i) {
          i__2 = *m, i__3 = *n - i;
          mi = min(i__2,i__3);
          d = c[i + j * c_dim1];
          i__2 = mi;
          for (l = 1; l <= i__2; ++l) {
             d -= e[l + i * e_dim1] * c[i + l + j * c_dim1];
          }
          c[i + j * c_dim1] = d / e[i * e_dim1];
       }
    }
    return 0;
}



/* TRINV.FOR, 1985-06-03 */

doublereal trinv(doublereal *b, doublereal *e, integer *m, integer *n)
{
    /* System generated locals */
    integer b_dim1, b_offset, e_dim1, e_offset, i__1, i__2, i__3;
    doublereal ret_val;

    /* Local variables */
    static integer i, j, k;
    static doublereal dd, dl;
    static integer mi;
    static doublereal du;
    static integer mn, mp;

/* ***  Assess central 2*M+1 bands of E**-1 and store in array E */

    /* Parameter adjustments */

    e_dim1 = *m - (-(*m)) + 1;
    e_offset = -(*m) + e_dim1;
    e -= e_offset;

    b_dim1 = *m - (-(*m)) + 1;
    b_offset = -(*m) + b_dim1;
    b -= b_offset;

    /* Function Body */
    e[*n * e_dim1] = 1. / e[*n * e_dim1];
    for (i = *n - 1; i >= 1; --i) {
       i__1 = *m, i__2 = *n - i;
       mi = min(i__1,i__2);
       dd = 1. / e[i * e_dim1];
       i__1 = mi;
       for (k = 1; k <= i__1; ++k) {
          e[k + *n * e_dim1] = e[k + i * e_dim1] * dd;
          e[-k + e_dim1] = e[-k + (k + i) * e_dim1];
       }
       dd += dd;
       for (j = mi; j >= 1; --j) {
          du = 0.;
          dl = 0.;
          i__1 = mi;
          for (k = 1; k <= i__1; ++k) {
             du -= e[k + *n * e_dim1] * e[j - k + (i + k) * e_dim1];
             dl -= e[-k + e_dim1] * e[k - j + (i + j) * e_dim1];
          }
          e[j + i * e_dim1] = du;
          e[-j + (j + i) * e_dim1] = dl;
          dd -= e[j + *n * e_dim1] * dl + e[-j + e_dim1] * du;
       }
       e[i * e_dim1] = dd * .5;
    }

    dd = 0.;
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
       i__2 = *m, i__3 = i - 1;
       mn = -min(i__2,i__3);
       i__2 = *m, i__3 = *n - i;
       mp = min(i__2,i__3);
       i__2 = mp;
       for (k = mn; k <= i__2; ++k) {
          dd += b[k + i * b_dim1] * e[-k + (k + i) * e_dim1];
       }
    }
    ret_val = dd;
    i__1 = *m;
    for (k = 1; k <= i__1; ++k) {
       e[k + *n * e_dim1] = 0.;
       e[-k + e_dim1] = 0.;
    }
    return ret_val;
}



/* SPLDER.FOR, 1985-06-11 */

doublereal splder(integer *ider, integer *m, integer *n, doublereal *t,
	               doublereal *x, doublereal *c, integer *l, doublereal *q)
{
    /* System generated locals */
    integer i__1, i__2;
    doublereal ret_val;

    /* Local variables */
    static integer lk1i1;
    static doublereal xjki;
    static integer i, j, k;
    static doublereal z;
    static integer i1, j1, k1, j2, m2, ii, jj, ki, jl, lk, mi, nk, lm, ml, jm, ir, ju;
    extern /* Subroutine */ int search(integer *, doublereal *, doublereal *, integer *);
    static doublereal tt;
    static integer lk1, mp1, m2m1, jin, nki, npm, lk1i, nki1;

/* ***  Derivatives of IDER.ge.2*M are always zero */

    /* Parameter adjustments */

    --q;
    --c;
    --x;

    /* Function Body */
    m2 = *m << 1;
    k = m2 - *ider;
    if (k < 1) {
       ret_val = 0.;
       return ret_val;
    }

    search(n, &x[1], t, l);

    tt = *t;
    mp1 = *m + 1;
    npm = *n + *m;
    m2m1 = m2 - 1;
    k1 = k - 1;
    nk = *n - k;
    lk = *l - k;
    lk1 = lk + 1;
    lm = *l - *m;
    jl = *l + 1;
    ju = *l + m2;
    ii = *n - m2;
    ml = -(*l);
    i__1 = ju;
    for (j = jl; j <= i__1; ++j) {
       if (j >= mp1 && j <= npm) {
          q[j + ml] = c[j - *m];
       } else {
          q[j + ml] = 0.;
       }
    }

    if (*ider > 0) {
       jl -= m2;
       ml += m2;
       i__1 = *ider;
       for (i = 1; i <= i__1; ++i) {
          ++jl;
          ++ii;
          j1 = max(1,jl);
          j2 = min(*l,ii);
          mi = m2 - i;
          j = j2 + 1;
          if (j1 <= j2) {
             i__2 = j2;
             for (jin = j1; jin <= i__2; ++jin) {
                --j;
                jm = ml + j;
                q[jm] = (q[jm] - q[jm - 1]) / (x[j + mi] - x[j]);
             }
          }
          if (jl >= 1) {
             goto L6;
          }
          i1 = i + 1;
          j = ml + 1;
          if (i1 <= ml) {
             i__2 = ml;
             for (jin = i1; jin <= i__2; ++jin) {
                --j;
                q[j] = -q[j - 1];
             }
          }
L6:
          ;
       }
       i__1 = k;
       for (j = 1; j <= i__1; ++j) {
          q[j] = q[j + *ider];
       }
    }

    if (k1 >= 1) {
       i__1 = k1;
       for (i = 1; i <= i__1; ++i) {
          nki = nk + i;
          ir = k;
          jj = *l;
          ki = k - i;
          nki1 = nki + 1;
          if (*l >= nki1) {
             i__2 = *l;
             for (j = nki1; j <= i__2; ++j) {
                q[ir] = q[ir - 1] + (tt - x[jj]) * q[ir];
                --jj;
                --ir;
             }
          }
          lk1i = lk1 + i;
          j1 = max(1,lk1i);
          j2 = min(*l,nki);
          if (j1 <= j2) {
             i__2 = j2;
             for (j = j1; j <= i__2; ++j) {
                xjki = x[jj + ki];
                z = q[ir];
                q[ir] = z + (xjki - tt) * (q[ir - 1] - z) / (xjki - x[jj])
                   ;
                --ir;
                --jj;
             }
          }
          if (lk1i <= 0) {
             jj = ki;
             lk1i1 = 1 - lk1i;
             i__2 = lk1i1;
             for (j = 1; j <= i__2; ++j) {
                q[ir] += (x[jj] - tt) * q[ir - 1];
                --jj;
                --ir;
             }
          }
       }
    }

    z = q[k];
    if (*ider > 0) {
       i__1 = m2m1;
       for (j = k; j <= i__1; ++j) {
          z *= j;
       }
    }
    ret_val = z;
    return ret_val;
}



/* SEARCH.FOR, 1985-06-03 */

int search(integer *n, doublereal *x, doublereal *t,	integer *l)
{
    static integer il, iu;

    /* Parameter adjustments */

    --x;

    /* Function Body */

    if (*t < x[1]) {
       *l = 0;
       return 0;
    }
    if (*t >= x[*n]) {
       *l = *n;
       return 0;
    }
    *l = max(*l,1);
    if (*l >= *n) {
       *l = *n - 1;
    }

    if (*t >= x[*l]) {
       goto L5;
    }
    --(*l);
    if (*t >= x[*l]) {
       return 0;
    }

    il = 1;
L3:
    iu = *l;
L4:
    *l = (il + iu) / 2;
    if (iu - il <= 1) {
       return 0;
    }
    if (*t < x[*l]) {
       goto L3;
    }
    il = *l;
    goto L4;
L5:
    if (*t < x[*l + 1]) {
       return 0;
    }
    ++(*l);
    if (*t < x[*l + 1]) {
       return 0;
    }
    il = *l + 1;
    iu = *n;
    goto L4;

}

