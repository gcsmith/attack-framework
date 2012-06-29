/* S-box & inverse with MASKING, using all normal bases */
/* revised 2008 November 28 to correct mask re-use problem */
/* edited 2009 May 8 to fix typos (re-declared variables) */
/*   NOTE: requires two independent 8-bit masks: input & output */
/*   based on compact S-box using Canright algorithm */
/*   optimized using NOR gates and NAND gates */

/* NOTE: this was designed including NAND and NOR and MUX21I gates
   (as in Satoh's "standard library")
   but my compiler won't compile the correct syntax e.g. ~& and ~|
   so these are written in a logically equivalent form below.
   (in most cases could use AND and OR with nearby XNOR replaced by XOR)
*/

/* multiply in GF(2^2), shared factors, using normal basis [Omega^2,Omega] */
module GF_MULS_2 ( A, B, Q );
    input  [2:0] A;   /* shared factors include bit sum: sum hi lo */
    input  [2:0] B;
    output [1:0] Q;
    wire   abcd, p, q;
    
    assign abcd = ~(A[2] & B[2]);  /* note: ~& syntax for NAND won't compile */
    assign p = (~(A[1] & B[1])) ^ abcd;
    assign q = (~(A[0] & B[0])) ^ abcd;
    assign Q = { p, q };
endmodule

/* multiply & scale by N in GF(2^2), shared factors, basis [Omega^2,Omega] */
module GF_MULS_SCL_2 ( A, B, Q );
    input  [2:0] A;   /* shared factors include bit sum: sum hi lo */
    input  [2:0] B;
    output [1:0] Q;
    wire   t, p, q;
    
    assign t = ~(A[0] & B[0]);  /* note: ~& syntax for NAND won't compile */
    assign p = (~(A[2] & B[2])) ^ t;
    assign q = (~(A[1] & B[1])) ^ t;
    assign Q = { p, q };
endmodule

/* sums for shared factors, 2-bit -> 3 */
module FAC_2 ( a, Q );
    input  [1:0] a;
    output [2:0] Q;
    wire   sa;
    
    assign sa = a[1] ^ a[0];
/* output is three 1-bit shared factors: sum hi lo */
    assign Q = { sa, a };
endmodule

/* inverse in GF(2^4)/GF(2^2), using normal basis [alpha^8, alpha^2] */
module GF_INV_4 ( A, M, N, Q );
    input  [3:0] A;
    input  [3:0] M;   /* input mask */
    input  [3:0] N;   /* output mask */
    output [3:0] Q;
    wire   [1:0] a, b, m, n, c, cm, e, d, p, q, an, mb, mn, m2, dn, em, pm, qm;
    wire   [1:0] csa, csb, cst, qsa, qsb, dm, psa, psb; /* partial sums */
    wire   [2:0] af, bf, mf, nf, ef, df;  /* factors w/ bit sums */
    
    assign a = A[3:2];
    assign b = A[1:0];
    assign m = M[3:2];
    assign n = M[1:0];
    assign m2 = N[3:2];  // mask for 2-bit sums, indep of M
    FAC_2 afac(a, af);
    FAC_2 bfac(b, bf);
    FAC_2 mfac(m, mf);
    FAC_2 nfac(n, nf);
    GF_MULS_2 anmul(af, nf, an);
    GF_MULS_2 mbmul(mf, bf, mb);
    GF_MULS_2 mnmul(mf, nf, mn);
// YO! NEED TO DO SUMMATION BELOW IN SEQUENTIAL ORDER FOR SECURITY !!!!
/* optimize section below using NOR gates */
    assign cst = {   /* note: ~| syntax for NOR won't compile */
      ~(a[1] | b[1]) ^ (~(af[2] & bf[2])) ,
      ~(af[2] | bf[2]) ^ (~(a[0] & b[0])) }
        ^ m2 ;
/* end of NOR optimization */
    assign csa = cst ^ an ;
    assign csb = csa ^ mb ;
    assign cm = {   /* this includes mask switch */
      m[1] ^ nf[2] ,
      mf[2] ^ n[0] }
        ^ mn ^ m2 ;
    assign c   = csb ^ cm ;
    assign e = {  /* inverse masked by n (lo input mask) */
      c[0] ,
      c[1] };
    FAC_2 efac(e, ef);
    GF_MULS_2  qmul(ef, af, q);
    GF_MULS_2 emmul(ef, mf, em);
/* NOTE: to maintain masking,
 the output mask N must be added BEFORE p, q are added to other terms */
// YO! NEED TO DO SUMMATION BELOW IN SEQUENTIAL ORDER FOR SECURITY !!!!
    assign qsa = N[1:0] ^ an ;  /* mask terms for q (lo output) */
    assign qsb = qsa ^ em ;     /* mask terms for q (lo output) */
    assign qm = qsb ^ mn ;      /* mask terms for q (lo output) */
// YO! NEED TO DO SUMMATION BELOW IN SEQUENTIAL ORDER FOR SECURITY !!!!
    assign dm = m ^ n;  /* mask adjustment */
    assign d =  e ^ dm;  /* switch masks: n -> m (hi input mask) */
    FAC_2 dfac(d, df);
    GF_MULS_2  pmul(df, bf, p);
    GF_MULS_2 dnmul(df, nf, dn);
// YO! NEED TO DO SUMMATION BELOW IN SEQUENTIAL ORDER FOR SECURITY !!!!
    assign psa = N[3:2] ^ mb ; /* mask terms for p (hi output) */
    assign psb = psa ^ dn ;    /* mask terms for p (hi output) */
    assign pm = psb ^ mn;      /* mask terms for p (hi output) */
    assign Q = { (pm ^ p), (qm ^ q) };
endmodule

/* multiply in GF(2^4)/GF(2^2), shared factors, basis [alpha^8, alpha^2] */
module GF_MULS_4 ( A, B, Q );
    input  [8:0] A;   /* shared factors include bit sums: sum hi lo */
    input  [8:0] B;
    output [3:0] Q;
    wire   [1:0] ph, pl, p;
    
    GF_MULS_SCL_2 summul( A[8:6], B[8:6], p);
    GF_MULS_2 himul(A[5:3], B[5:3], ph);
    GF_MULS_2 lomul(A[2:0], B[2:0], pl);
    assign Q = { (ph ^ p), (pl ^ p) };
endmodule

/* sums for shared factors, 4-bit -> 9 */
module FAC_4 ( a, Q );
    input  [3:0] a;
    output [8:0] Q;
    wire   [1:0] sa;
    wire al, ah, aa;
    
    assign sa = a[3:2] ^ a[1:0];
    assign al = a[1] ^ a[0];
    assign ah = a[3] ^ a[2];
    assign aa = sa[1] ^ sa[0];
/* output is three 3-bit shared factors: sum hi lo */
    assign Q = { aa, sa, ah, a[3:2], al, a[1:0] };
endmodule

/* inverse in GF(2^8)/GF(2^4), using normal basis [d^16, d] */
module GF_INV_8 ( A, M, N, Q );
    input  [7:0] A;
    input  [7:0] M;   /* input mask */
    input  [7:0] N;   /* output mask */
    output [7:0] Q;
    wire   [3:0] a, b, m, n, o, c, d, e, p, q, m4, an, mb, mn, dn, em, pm, qm;
    wire   [3:0] csa, csb, cst, csm, qsa, qsb, psa, psb; /* partial sums */
    wire   [8:0] af, bf, mf, nf, ef, df;  /* factors w/ bit sums */
    wire c1, c2, c3;   /* for temp var */
    
    assign a = A[7:4];
    assign b = A[3:0];
    assign m = M[7:4];
    assign n = M[3:0];
    assign m4 = N[7:4];
    assign o = m ^ n;  /* to switch masks below; has useful bits */
    FAC_4 afac(a, af);
    FAC_4 bfac(b, bf);
    FAC_4 mfac(m, mf);
    FAC_4 nfac(n, nf);
    GF_MULS_4 anmul(af, nf, an);
    GF_MULS_4 mbmul(mf, bf, mb);
    GF_MULS_4 mnmul(mf, nf, mn);
// YO! NEED TO DO SUMMATION BELOW IN SEQUENTIAL ORDER FOR SECURITY !!!!
/* optimize section below using NOR gates */
    assign c1 = ~(af[5] & bf[5]);
    assign c2 = ~(af[6] & bf[6]);
    assign c3 = ~(af[8] & bf[8]);
    assign cst = {   /* note: ~| syntax for NOR won't compile */
      (~(af[6] | bf[6]) ^ (~(a[3] & b[3]))) ^ c1 ^ c3 ,
      (~(af[7] | bf[7]) ^ (~(a[2] & b[2]))) ^ c1 ^ c2 ,
      (~(af[2] | bf[2]) ^ (~(a[1] & b[1]))) ^ c2 ^ c3 ,
      (~(a[0] | b[0]) ^ (~(af[2] & bf[2]))) ^ (~(af[7] & bf[7])) ^ c2 }
         ^ m4 ;
/* end of NOR optimization */
    assign csa = cst ^ an ;
    assign csb = csa ^ mb ;
    assign csm = mn ^ {   /* this includes mask terms only */
      mf[6] ^ nf[6] ,
      mf[7] ^ nf[7] ,
      mf[2] ^ nf[2] ,
      o[0] };
    assign c   = csb ^ csm ;
    GF_INV_4 dinv( c, m4, m, d); /* inverse masked by m (hi input mask) */

    FAC_4 dfac(d, df);
    GF_MULS_4  pmul(df, bf, p);
    GF_MULS_4 dnmul(df, nf, dn);
// YO! NEED TO DO SUMMATION BELOW IN SEQUENTIAL ORDER FOR SECURITY !!!!
    assign psa = N[7:4] ^ mb ; /* mask terms for p (hi output) */
    assign psb = psa ^ dn ;    /* mask terms for p (hi output) */
    assign pm  = psb ^ mn ;    /* mask terms for p (hi output) */
    assign e = d ^ o;  /* switch masks: m -> n (lo input mask) */
    FAC_4 efac(e, ef);
    GF_MULS_4  qmul(ef, af, q);
    GF_MULS_4 emmul(ef, mf, em);
// YO! NEED TO DO SUMMATION BELOW IN SEQUENTIAL ORDER FOR SECURITY !!!!
    assign qsa = N[3:0] ^ an ; /* mask terms for q (lo output) */
    assign qsb = qsa ^ em ;    /* mask terms for q (lo output) */
    assign qm  = qsb ^ mn;     /* mask terms for q (lo output) */
    assign Q = { (pm ^ p), (qm ^ q) };
endmodule

/* S-box basis change with MASKING, using all normal bases */

/* MUX21I is an inverting 2:1 multiplexor */
module MUX21I ( A, B, s, Q );
    input        A;
    input        B;
    input        s;  /* selection switch */
    output       Q;
    assign Q = ~ ( s ? A : B );  /* mock-up for FPGA implementation */
endmodule

/* select and invert (NOT) byte, using MUX21I */
module SELECT_NOT_8 ( A, B, s, Q );
    input  [7:0] A;
    input  [7:0] B;
    input        s;  /* selection switch */
    output [7:0] Q;
    MUX21I m7(A[7],B[7],s,Q[7]);
    MUX21I m6(A[6],B[6],s,Q[6]);
    MUX21I m5(A[5],B[5],s,Q[5]);
    MUX21I m4(A[4],B[4],s,Q[4]);
    MUX21I m3(A[3],B[3],s,Q[3]);
    MUX21I m2(A[2],B[2],s,Q[2]);
    MUX21I m1(A[1],B[1],s,Q[1]);
    MUX21I m0(A[0],B[0],s,Q[0]);
endmodule

/* find either Sbox or its inverse in GF(2^8), by Canright Algorithm */
/* with MASKING: the input mask M and output mask N must be given */
module bSbox ( A, M, N, encrypt, Q );
    input  [7:0] A;
    input  [7:0] M;
    input  [7:0] N;
    input        encrypt;  /* 1 for Sbox, 0 for inverse Sbox */
    output [7:0] Q;
    wire   [7:0] B, C, D, E, F, G, H, V, W, X, Y, Z;
    wire R1, R2, R3, R4, R5, R6, R7, R8, R9;
    wire S1, S2, S3, S4, S5, S6, S7, S8, S9;
    wire T1, T2, T3, T4, T5, T6, T7, T8, T9;
    wire U1, U2, U3, U4, U5, U6, U7, U8, U9, U10;

/* change basis from GF(2^8) to GF(2^8)/GF(2^4)/GF(2^2) */
/* combine with bit inverse matrix multiply of Sbox */
assign  R1  = A[7] ^ A[5] ;
assign  R2  = A[7] ~^ A[4] ;
assign  R3  = A[6] ^ A[0] ;
assign  R4  = A[5] ~^  R3  ;
assign  R5  = A[4] ^  R4  ;
assign  R6  = A[3] ^ A[0] ;
assign  R7  = A[2] ^  R1  ;
assign  R8  = A[1] ^  R3  ;
assign  R9  = A[3] ^  R8  ;
assign B[7] =  R7  ~^  R8  ;
assign B[6] =  R5  ;
assign B[5] = A[1] ^  R4  ;
assign B[4] =  R1  ~^  R3  ;
assign B[3] = A[1] ^  R2  ^  R6  ;
assign B[2] = ~ A[0] ;
assign B[1] =  R4  ;
assign B[0] = A[2] ~^  R9  ;
assign Y[7] =  R2  ;
assign Y[6] = A[4] ^  R8  ;
assign Y[5] = A[6] ^ A[4] ;
assign Y[4] =  R9  ;
assign Y[3] = A[6] ~^  R2  ;
assign Y[2] =  R7  ;
assign Y[1] = A[4] ^  R6  ;
assign Y[0] = A[1] ^  R5  ;
    SELECT_NOT_8 sel_in( B, Y, encrypt, Z );

// convert masks also, but no additive constant for affine
assign  S1  = M[7] ~^ M[5] ;
assign  S2  = M[7] ~^ M[4] ;
assign  S3  = M[6] ~^ M[0] ;
assign  S4  = M[5]  ^  S3  ;
assign  S5  = M[4]  ^  S4  ;
assign  S6  = M[3]  ^ M[0] ;
assign  S7  = M[2]  ^  S1  ;
assign  S8  = M[1]  ^  S3  ;
assign  S9  = M[3]  ^  S8  ;
assign E[7] =  S7  ~^  S8  ;
assign E[6] =  S5   ;
assign E[5] = M[1]  ^  S4  ;
assign E[4] =  S1  ~^  S3  ;
assign E[3] = M[1]  ^  S2  ^  S6  ;
assign E[2] = ~ M[0] ;
assign E[1] =  S4   ;
assign E[0] = M[2]  ^  S9  ;
assign F[7] =  S2   ;
assign F[6] = M[4]  ^  S8  ;
assign F[5] = M[6] ~^ M[4] ;
assign F[4] =  S9   ;
assign F[3] = M[6]  ^  S2  ;
assign F[2] =  S7   ;
assign F[1] = M[4] ~^  S6  ;
assign F[0] = M[1]  ^  S5  ;
    SELECT_NOT_8 sel_Min( E, F, encrypt, V );

assign  T1  = N[7] ~^ N[5] ;
assign  T2  = N[7] ~^ N[4] ;
assign  T3  = N[6] ~^ N[0] ;
assign  T4  = N[5]  ^  T3  ;
assign  T5  = N[4]  ^  T4  ;
assign  T6  = N[3]  ^ N[0] ;
assign  T7  = N[2]  ^  T1  ;
assign  T8  = N[1]  ^  T3  ;
assign  T9  = N[3]  ^  T8  ;
assign G[7] =  T7  ~^  T8  ;
assign G[6] =  T5   ;
assign G[5] = N[1]  ^  T4  ;
assign G[4] =  T1  ~^  T3  ;
assign G[3] = N[1]  ^  T2  ^  T6  ;
assign G[2] = ~ N[0] ;
assign G[1] =  T4   ;
assign G[0] = N[2]  ^  T9  ;
assign H[7] =  T2   ;
assign H[6] = N[4]  ^  T8  ;
assign H[5] = N[6] ~^ N[4] ;
assign H[4] =  T9   ;
assign H[3] = N[6]  ^  T2  ;
assign H[2] =  T7   ;
assign H[1] = N[4] ~^  T6  ;
assign H[0] = N[1]  ^  T5  ;
    SELECT_NOT_8 sel_Mout( H, G, encrypt, W );

    GF_INV_8 inv( Z, V, W, C );

/* change basis back from GF(2^8)/GF(2^4)/GF(2^2) to GF(2^8) */
/* combine with matrix multiply of Sbox */
assign  U1  = C[7] ^ C[3] ;
assign  U2  = C[6] ^ C[4] ;
assign  U3  = C[6] ^ C[0] ;
assign  U4  = C[5] ~^ C[3] ;
assign  U5  = C[5] ~^  U1  ;
assign  U6  = C[5] ~^ C[1] ;
assign  U7  = C[4] ~^  U6  ;
assign  U8  = C[2] ^  U4  ;
assign  U9  = C[1] ^  U2  ;
assign  U10  =  U3  ^  U5  ;
assign D[7] =  U4  ;
assign D[6] =  U1  ;
assign D[5] =  U3  ;
assign D[4] =  U5  ;
assign D[3] =  U2  ^  U5  ;
assign D[2] =  U3  ^  U8  ;
assign D[1] =  U7  ;
assign D[0] =  U9  ;
assign X[7] = C[4] ~^ C[1] ;
assign X[6] = C[1] ^  U10  ;
assign X[5] = C[2] ^  U10  ;
assign X[4] = C[6] ~^ C[1] ;
assign X[3] =  U8  ^  U9  ;
assign X[2] = C[7] ~^  U7  ;
assign X[1] =  U6  ;
assign X[0] = ~ C[2] ;
    SELECT_NOT_8 sel_out( D, X, encrypt, Q );
endmodule

// GCS: /* test program: put Sbox output into register */
// GCS: /* with MASKING: the input mask M and output mask N must be given */
// GCS: /*   NOTE: these two 8-bit masks must be random and INDEPENDENT */
// GCS: module Sbox_m ( A, M, N, S, Si, CLK );
// GCS:     input  [7:0] A;
// GCS:     input  [7:0] M;
// GCS:     input  [7:0] N;
// GCS:     output [7:0] S;
// GCS:     output [7:0] Si;
// GCS:     input  CLK  /* synthesis syn_noclockbuf=1 */ ;
// GCS:     reg    [7:0] S;
// GCS:     reg    [7:0] Si;
// GCS:     wire   [7:0] s;
// GCS:     wire   [7:0] si;
// GCS:     
// GCS:     bSbox sbe(A, M, N, 1, s);
// GCS:     bSbox sbd(A, M, N, 0, si);
// GCS:     always @ (posedge CLK) begin
// GCS:     S <= s;
// GCS:     Si <= si;
// GCS:     end
// GCS: endmodule
