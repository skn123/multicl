const char *cl_source_ratt2 =
"#ifdef K_DOUBLE_PRECISION\n"
"#define DOUBLE_PRECISION\n"
"#pragma OPENCL EXTENSION cl_khr_fp64: enable\n"
"#elif AMD_DOUBLE_PRECISION\n"
"#define DOUBLE_PRECISION\n"
"#pragma OPENCL EXTENSION cl_amd_fp64: enable\n"
"#endif\n"
"\n"
"// Macros to explicitly control precision of the constants, otherwise\n"
"// known to cause problems for some Compilers\n"
"#ifdef DOUBLE_PRECISION\n"
"#define CPREC(a) a\n"
"#else\n"
"#define CPREC(a) a##f\n"
"#endif\n"
"\n"
"//replace divisions by multiplication with the reciprocal\n"
"#define REPLACE_DIV_WITH_RCP 1\n"
"\n"
"//Call the appropriate math function based on precision\n"
"#ifdef DOUBLE_PRECISION\n"
"#define real double\n"
"#if REPLACE_DIV_WITH_RCP\n"
"#define DIV(x,y) ((x)*(1.0/(y)))\n"
"#else\n"
"#define DIV(x,y) ((x)/(y))\n"
"#endif\n"
"#define POW pow\n"
"#define EXP exp\n"
"#define EXP10 exp10\n"
"#define EXP2 exp2\n"
"#define MAX fmax\n"
"#define MIN fmin\n"
"#define LOG log\n"
"#define LOG10 log10\n"
"#else\n"
"#define real float\n"
"#if REPLACE_DIV_WITH_RCP\n"
"#define DIV(x,y) ((x)*(1.0f/(y)))\n"
"#else\n"
"#define DIV(x,y) ((x)/(y))\n"
"#endif\n"
"#define POW pow\n"
"#define EXP exp\n"
"#define EXP10 exp10\n"
"#define EXP2 exp2\n"
"#define MAX fmax\n"
"#define MIN fmin\n"
"#define LOG log\n"
"#define LOG10 log10\n"
"#endif\n"
"//Kernel indexing macros\n"
"#define thread_num (get_global_id(0))\n"
"#define idx2(p,z) (p[(((z)-1)*(N_GP)) + thread_num])\n"
"#define idx(x, y) ((x)[(y)-1])\n"
"#define C(q)     idx2(C, q)\n"
"#define Y(q)     idx2(Y, q)\n"
"#define RF(q)    idx2(RF, q)\n"
"#define EG(q)    idx2(EG, q)\n"
"#define RB(q)    idx2(RB, q)\n"
"#define RKLOW(q) idx2(RKLOW, q)\n"
"#define ROP(q)   idx(ROP, q)\n"
"#define WDOT(q)  idx2(WDOT, q)\n"
"#define RKF(q)   idx2(RKF, q)\n"
"#define RKR(q)   idx2(RKR, q)\n"
"#define A_DIM    (11)\n"
"#define A(b, c)  idx2(A, (((b)*A_DIM)+c) )\n"
"\n"
"\n"
"__kernel void\n"
"ratt2_kernel(__global const real* T, __global const real* RF, __global real* RB,\n"
"		__global const real* EG, const real TCONV)\n"
"{\n"
"\n"
"    const real TEMP = T[get_global_id(0)]*TCONV;\n"
"    const real ALOGT = LOG(TEMP);\n"
"#ifdef DOUBLE_PRECISION\n"
"    const real SMALL_INV = 1e300;\n"
"#else \n"
"    const real SMALL_INV = 1e+20f;\n"
"#endif\n"
"    const real RU=CPREC(8.31451e7);\n"
"    const real PATM = CPREC(1.01325e6);\n"
"    const real PFAC = DIV (PATM, (RU*(TEMP)));\n"
"    \n"
"    real rtemp_inv;\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(4)), (EG(3)*EG(5)));\n"
"    RB(1) = RF(1) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(1)*EG(3)), (EG(2)*EG(5)));\n"
"    RB(2) = RF(2) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(1)*EG(5)), (EG(2)*EG(6)));\n"
"    RB(3) = RF(3) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(5)*EG(5)), (EG(3)*EG(6)));\n"
"    RB(4) = RF(4) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(2)*PFAC), EG(1));\n"
"    RB(5) = RF(5) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(2)*PFAC), EG(1));\n"
"    RB(6) = RF(6) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(2)*PFAC), EG(1));\n"
"    RB(7) = RF(7) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(2)*PFAC), EG(1));\n"
"    RB(8) = RF(8) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(5)*PFAC), EG(6));\n"
"    RB(9) = RF(9) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(3)*PFAC), EG(5));\n"
"    RB(10) = RF(10) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(3)*EG(3)*PFAC), EG(4));\n"
"    RB(11) = RF(11) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(4)*PFAC), EG(7));\n"
"    RB(12) = RF(12) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(4)*PFAC), EG(7));\n"
"    RB(13) = RF(13) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(4)*PFAC), EG(7));\n"
"    RB(14) = RF(14) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(4)*PFAC), EG(7));\n"
"    RB(15) = RF(15) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(5)*EG(5)*PFAC), EG(8));\n"
"    RB(16) = RF(16) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(7)), (EG(3)*EG(6)));\n"
"    RB(17) = RF(17) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(7)), (EG(1)*EG(4)));\n"
"    RB(18) = RF(18) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(7)), (EG(5)*EG(5)));\n"
"    RB(19) = RF(19) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(3)*EG(7)), (EG(4)*EG(5)));\n"
"    RB(20) = RF(20) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(5)*EG(7)), (EG(4)*EG(6)));\n"
"    RB(21) = RF(21) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(7)*EG(7)), (EG(4)*EG(8)));\n"
"    RB(22) = RF(22) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(7)*EG(7)), (EG(4)*EG(8)));\n"
"    RB(23) = RF(23) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(8)), (EG(1)*EG(7)));\n"
"    RB(24) = RF(24) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(8)), (EG(5)*EG(6)));\n"
"    RB(25) = RF(25) * MIN(rtemp_inv, SMALL_INV);\n"
"}\n"
;
