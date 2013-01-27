const char *cl_source_rdwdot6 =
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
"#define ROP2(a)  (RKF(a) - RKR (a))\n"
"\n"
"\n"
"__kernel void\n"
"rdwdot6_kernel (__global const real* RKF, __global const real* RKR,\n"
"		__global real* WDOT, const real rateconv, __global const real* molwt)\n"
"{\n"
" \n"
"    WDOT(11) = (-ROP2(29) -ROP2(30) -ROP2(31) -ROP2(32)\n"
"            -ROP2(33) +ROP2(34) -ROP2(39) +ROP2(40)\n"
"            +ROP2(42) +ROP2(43) +ROP2(45) +ROP2(46)\n"
"            +ROP2(47) -ROP2(56) +ROP2(61) +ROP2(65)\n"
"            +ROP2(66) +ROP2(70) +ROP2(88) +ROP2(95)\n"
"            +ROP2(108) +ROP2(109) +ROP2(109) +ROP2(110) +ROP2(110)\n"
"            +ROP2(111) +ROP2(112) +ROP2(113) +ROP2(113)\n"
"            +ROP2(117) +ROP2(119) +ROP2(120) +ROP2(123)\n"
"            +ROP2(128) +ROP2(136) +ROP2(143) +ROP2(147)\n"
"            +ROP2(154) +ROP2(164) +ROP2(179) +ROP2(189))*rateconv *molwt[10];\n"
"    WDOT(12) = (+ROP2(29) +ROP2(30) +ROP2(32) +ROP2(33)\n"
"            -ROP2(40) +ROP2(44) +ROP2(52) -ROP2(70)\n"
"            +ROP2(125) +ROP2(130))*rateconv *molwt[11];\n"
"}\n"
;
