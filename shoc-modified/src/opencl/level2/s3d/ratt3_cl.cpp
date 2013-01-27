const char *cl_source_ratt3 =
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
"\n"
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
"__kernel void\n"
"ratt3_kernel(__global const real* T, __global const real* RF,\n"
"		__global real* RB, __global	const real* EG, const real TCONV)\n"
"{\n"
"\n"
"    real TEMP = T[get_global_id(0)]*TCONV;\n"
"    real ALOGT = LOG(TEMP);\n"
"#ifdef DOUBLE_PRECISION\n"
"    const real SMALL_INV = 1e+300;\n"
"#else \n"
"    const real SMALL_INV = 1e+20f;\n"
"#endif\n"
"\n"
"    const real RU=CPREC(8.31451e7);\n"
"    const real PATM = CPREC(1.01325e6);\n"
"    const real PFAC = DIV (PATM, (RU*(TEMP)));\n"
"    \n"
"    real rtemp_inv;\n"
"\n"
"    rtemp_inv = DIV ((EG(3)*EG(8)), (EG(5)*EG(7)));\n"
"    RB(26) = RF(26) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(5)*EG(8)), (EG(6)*EG(7)));\n"
"    RB(27) = RF(27) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(5)*EG(8)), (EG(6)*EG(7)));\n"
"    RB(28) = RF(28) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(3)*EG(14)*PFAC), EG(15));\n"
"    RB(29) = RF(29) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(5)*EG(14)), (EG(2)*EG(15)));\n"
"    RB(30) = RF(30) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(1)*EG(14)*PFAC), EG(17));\n"
"    RB(31) = RF(31) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(4)*EG(14)), (EG(3)*EG(15)));\n"
"    RB(32) = RF(32) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(7)*EG(14)), (EG(5)*EG(15)));\n"
"    RB(33) = RF(33) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(3)*EG(9)), (EG(2)*EG(14)));\n"
"    RB(34) = RF(34) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(5)*EG(9)), (EG(2)*EG(16)));\n"
"    RB(35) = RF(35) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(1)*EG(9)), (EG(2)*EG(10)));\n"
"    RB(36) = RF(36) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(6)*EG(9)), (EG(2)*EG(17)));\n"
"    RB(37) = RF(37) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(4)*EG(9)), (EG(3)*EG(16)));\n"
"    RB(38) = RF(38) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(9)*EG(14)*PFAC), EG(25));\n"
"    RB(39) = RF(39) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(9)*EG(15)), (EG(14)*EG(16)));\n"
"    RB(40) = RF(40) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(16)*PFAC), EG(17));\n"
"    RB(41) = RF(41) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(16)), (EG(1)*EG(14)));\n"
"    RB(42) = RF(42) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(3)*EG(16)), (EG(5)*EG(14)));\n"
"    RB(43) = RF(43) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(3)*EG(16)), (EG(2)*EG(15)));\n"
"    RB(44) = RF(44) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(5)*EG(16)), (EG(6)*EG(14)));\n"
"    RB(45) = RF(45) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(16)*PFAC), (EG(2)*EG(14)));\n"
"    RB(46) = RF(46) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(4)*EG(16)), (EG(7)*EG(14)));\n"
"    RB(47) = RF(47) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(2)*EG(10)*PFAC), EG(12));\n"
"    RB(48) = RF(48) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(1)*EG(10)), (EG(2)*EG(12)));\n"
"    RB(49) = RF(49) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"    rtemp_inv = DIV ((EG(3)*EG(10)), (EG(2)*EG(16)));\n"
"    RB(50) = RF(50) * MIN(rtemp_inv, SMALL_INV);\n"
"\n"
"}\n"
;
