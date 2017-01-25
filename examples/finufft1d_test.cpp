#include <math.h>
#include "../src/utils.h"
#include "../src/finufft1d.h"
#include "../src/dirft1d.h"
#include <stdio.h>

// C++ stuff
#include <iostream>
#include <iomanip>
#include <vector>

// how big a problem to switch to checking one mode...
#define BIGPROB 1e8

int main(int argc, char* argv[])
/* Test executable for nufft1d.

   Example: finufft1d_test 1000000 1000000 1e-12

 All complex arith done by hand for now. Barnett 1/22/17
*/
{
  BIGINT M = 1e6, N = 1e6;    // defaults: M = # srcs, N = # modes out
  double y, tol = 1e-6;          // default
  int isign = +1;
  if (argc>1) { sscanf(argv[1],"%lf",&y); N = (BIGINT)y; }
  if (argc>2) { sscanf(argv[2],"%lf",&y); M = (BIGINT)y; }
  if (argc>3) {
    sscanf(argv[3],"%lf",&tol);
    if (tol<=0.0) { printf("tol must be positive!\n"); return 1; }
  }
  if (argc==1 || argc>4) {
    fprintf(stderr,"Usage: finufft1d_test [Nmodes [Nsrc [tol]]]\n");
    return 1;
  }
  cout << scientific << setprecision(15);

  double *x = (double *)malloc(sizeof(double)*M);        // NU pts
  for (BIGINT j=0; j<M; ++j) x[j] = M_PI*randm11();
  dcomplex* c = (dcomplex*)malloc(sizeof(dcomplex)*M);   // strengths 
  dcomplex* F = (dcomplex*)malloc(sizeof(dcomplex)*N);   // mode ampls

  printf("1d type-1:\n"); // -------------- type 1
  for (BIGINT j=0; j<M; ++j) c[j] = crandm11();
  CNTime timer; timer.start();
  int ier = finufft1d1(M,x,(double*)c,isign,tol,N,(double*)F);
  //for (int j=0;j<N;++j) cout<<F[j]<<endl;
  double t=timer.elapsedsec();
  if (ier!=0) {
    printf("error (ier=%d)!\n",ier); return 1;
  } else
    printf("\t%ld NU pts to %ld modes in %.3g s \t%.3g NU pts/s\n",M,N,t,M/t);

  if (M*N>BIGPROB) {          // too big for direct, check one mode
    BIGINT nt = N/2 - 7;      // arbitrary choice of mode near the top
    dcomplex Ft = {0,0};
    for (BIGINT j=0; j<M; ++j)
      Ft += c[j] * exp(ima*((double)(isign*nt))*x[j]);
    Ft /= M;
    printf("one mode: rel err in F[%ld] is %.3g\n",nt,abs(Ft-F[N/2+nt])/infnorm(N,F));
  } else {                 // direct eval
    dcomplex* Ft = (dcomplex*)malloc(sizeof(dcomplex)*N);
    dirft1d1(M,x,c,isign,N,Ft);
    printf("dirft1d: rel l2-err of result F is %.3g\n",relerrtwonorm(N,F,Ft));
    free(Ft);
  }

  printf("1d type-2:\n"); // -------------- type 2
  for (BIGINT m=0; m<N; ++m) F[m] = crandm11();
  timer.restart();
  ier = finufft1d2(M,x,(double*)c,isign,tol,N,(double*)F);
  //for (int j=0;j<M;++j) cout<<c[j]<<endl;
  t=timer.elapsedsec();
  if (ier!=0) {
    printf("error (ier=%d)!\n",ier); return 1;
  } else
    printf("\t%ld modes to %ld NU pts in %.3g s \t%.3g NU pts/s\n",N,M,t,M/t);

  if (M*N>BIGPROB) {          // too big for direct, check one targ
    BIGINT jt = M/2;        // arbitrary choice of targ pt
    BIGINT kmin = -round(((double)N-0.5)/2);
    dcomplex ct = {0,0};
    for (BIGINT m=kmin; m<=(N-1)/2; ++m)
      ct += F[jt] * exp(ima*((double)(isign*m))*x[jt]);
    printf("one mode: rel err in c[%ld] is %.3g\n",jt,abs(ct-c[jt])/infnorm(M,c));
  } else {                 // direct eval
    dcomplex* ct = (dcomplex*)malloc(sizeof(dcomplex)*M);
    dirft1d2(M,x,ct,isign,N,F);
    printf("dirft1d: rel l2-err of result c is %.3g\n",relerrtwonorm(M,c,ct));
    free(ct);
  }

  // --- todo: type 3



  free(x); free(c); free(F);
  return ier;
}