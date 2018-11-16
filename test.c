/*
 * TINYEXPR - Tiny recursive descent parser and evaluation engine in C
 *
 * Copyright (c) 2015, 2016 Lewis Van Winkle
 *
 * http://CodePlea.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgement in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "tinyexpr.h"
#include <stdio.h>
#include "minctest.h"


typedef struct {
    const char *expr;
    double answer;
} test_case;

typedef struct {
    const char *expr1;
    const char *expr2;
} test_equ;



static void test_results() {
    test_case cases[] = {
        {"1", 1},
        {"1 ", 1},
        {"(1)", 1},

        {"pi", 3.14159},
        {"atan(1)*4 - pi", 0},
        {"e", 2.71828},

        {"2+1", 2+1},
        {"(((2+(1))))", 2+1},
        {"3+2", 3+2},

        {"3+2+4", 3+2+4},
        {"(3+2)+4", 3+2+4},
        {"3+(2+4)", 3+2+4},
        {"(3+2+4)", 3+2+4},

        {"3*2*4", 3*2*4},
        {"(3*2)*4", 3*2*4},
        {"3*(2*4)", 3*2*4},
        {"(3*2*4)", 3*2*4},

        {"3-2-4", 3-2-4},
        {"(3-2)-4", (3-2)-4},
        {"3-(2-4)", 3-(2-4)},
        {"(3-2-4)", 3-2-4},

        {"3/2/4", 3.0/2.0/4.0},
        {"(3/2)/4", (3.0/2.0)/4.0},
        {"3/(2/4)", 3.0/(2.0/4.0)},
        {"(3/2/4)", 3.0/2.0/4.0},

        {"(3*2/4)", 3.0*2.0/4.0},
        {"(3/2*4)", 3.0/2.0*4.0},
        {"3*(2/4)", 3.0*(2.0/4.0)},

        {"asin sin .5", 0.5},
        {"sin asin .5", 0.5},
        {"ln exp .5", 0.5},
        {"exp ln .5", 0.5},

        {"asin sin-.5", -0.5},
        {"asin sin-0.5", -0.5},
        {"asin sin -0.5", -0.5},
        {"asin (sin -0.5)", -0.5},
        {"asin (sin (-0.5))", -0.5},
        {"asin sin (-0.5)", -0.5},
        {"(asin sin (-0.5))", -0.5},

        {"log10 1000", 3},
        {"log10 1e3", 3},
        {"log10 1000", 3},
        {"log10 1e3", 3},
        {"log10(1000)", 3},
        {"log10(1e3)", 3},
        {"log10 1.0e3", 3},
        {"10^5*5e-5", 5},

#ifdef TE_NAT_LOG
        {"log 1000", 6.9078},
        {"log e", 1},
        {"log (e^10)", 10},
#else
        {"log 1000", 3},
#endif

        {"ln (e^10)", 10},
        {"100^.5+1", 11},
        {"100 ^.5+1", 11},
        {"100^+.5+1", 11},
        {"100^--.5+1", 11},
        {"100^---+-++---++-+-+-.5+1", 11},

        {"100^-.5+1", 1.1},
        {"100^---.5+1", 1.1},
        {"100^+---.5+1", 1.1},
        {"1e2^+---.5e0+1e0", 1.1},
        {"--(1e2^(+(-(-(-.5e0))))+1e0)", 1.1},

        {"sqrt 100 + 7", 17},
        {"sqrt 100 * 7", 70},
        {"sqrt (100 * 100)", 100},

        {"1,2", 2},
        {"1,2+1", 3},
        {"1+1,2+2,2+1", 3},
        {"1,2,3", 3},
        {"(1,2),3", 3},
        {"1,(2,3)", 3},
        {"-(1,(2,3))", -3},

        {"2^2", 4},
        {"pow(2,2)", 4},

        {"atan2(1,1)", 0.7854},
        {"atan2(1,2)", 0.4636},
        {"atan2(2,1)", 1.1071},
        {"atan2(3,4)", 0.6435},
        {"atan2(3+3,4*2)", 0.6435},
        {"atan2(3+3,(4*2))", 0.6435},
        {"atan2((3+3),4*2)", 0.6435},
        {"atan2((3+3),(4*2))", 0.6435},

        { "1>2", 0 },
        { "1>=2", 0 },
        { "1<2", 1 },
        { "1<=2", 1 },
        { "1==2", 0 },
        { "1!=2", 1 },
        { "1>1", 0 },
        { "1>=1", 1 },
        { "1<1", 0 },
        { "1<=1", 1 },
        { "1==1", 1 },
        { "1!=1", 0 },
        { "(3 > 2) * 5", 5 },
    };


    size_t i;
    for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i) {
        const char *expr = cases[i].expr;
        const double answer = cases[i].answer;

        int err;
        const double ev = te_interp(expr, &err);
        lok(!err);
        lfequal(ev, answer);

        if (err) {
            printf("FAILED: %s (%d)\n", expr, err);
        }
    }
}


static void test_syntax() {
    test_case errors[] = {
        {"", 1},
        {"1+", 2},
        {"1)", 2},
        {"(1", 2},
        {"1**1", 3},
        {"1*2(+4", 4},
        {"1*2(1+4", 4},
        {"a+5", 1},
        {"A+5", 1},
        {"Aa+5", 1},
        {"1^^5", 3},
        {"1**5", 3},
        {"sin(cos5", 8},
    };


    size_t i;
    for (i = 0; i < sizeof(errors) / sizeof(test_case); ++i) {
        const char *expr = errors[i].expr;
        const int e = (int)errors[i].answer;

        int err;
        const double r = te_interp(expr, &err);
        lequal(err, e);
        lok(r != r);

        te_expr *n = te_compile(expr, 0, 0, &err);
        lequal(err, e);
        lok(!n);

        if (err != e) {
            printf("FAILED: %s\n", expr);
        }

        const double k = te_interp(expr, 0);
        lok(k != k);
    }
}


static void test_nans() {

    const char *nans[] = {
        "0/0",
        "1%0",
        "1%(1%0)",
        "(1%0)%1",
        "fac(-1)",
        "ncr(2, 4)",
        "ncr(-2, 4)",
        "ncr(2, -4)",
        "npr(2, 4)",
        "npr(-2, 4)",
        "npr(2, -4)",
    };

    size_t i;
    for (i = 0; i < sizeof(nans) / sizeof(const char *); ++i) {
        const char *expr = nans[i];

        int err;
        const double r = te_interp(expr, &err);
        lequal(err, 0);
        lok(r != r);

        te_expr *n = te_compile(expr, 0, 0, &err);
        lok(n);
        lequal(err, 0);
        const double c = te_eval(n, NULL);
        lok(c != c);
        te_free(n);
    }
}


static void test_infs() {

    const char *infs[] = {
            "1/0",
            "log(0)",
            "pow(2,10000000)",
            "fac(300)",
            "ncr(300,100)",
            "ncr(300000,100)",
            "ncr(300000,100)*8",
            "npr(3,2)*ncr(300000,100)",
            "npr(100,90)",
            "npr(30,25)",
    };

    size_t i;
    for (i = 0; i < sizeof(infs) / sizeof(const char *); ++i) {
        const char *expr = infs[i];

        int err;
        const double r = te_interp(expr, &err);
        lequal(err, 0);
        lok(r == r + 1);

        te_expr *n = te_compile(expr, 0, 0, &err);
        lok(n);
        lequal(err, 0);
        const double c = te_eval(n, NULL);
        lok(c == c + 1);
        te_free(n);
    }
}


static void test_variables() {

    static double x, y, test;
    te_variable lookup[] = { TE_DEF_VARIABLE("x", x), TE_DEF_VARIABLE("y", y), TE_DEF_VARIABLE("te_st", test) };

    int err;

    te_expr *expr1 = te_compile("cos x + sin y", lookup, 2, &err);
    lok(expr1);
    lok(!err);

    te_expr *expr2 = te_compile("x+x+x-y", lookup, 2, &err);
    lok(expr2);
    lok(!err);

    te_expr *expr3 = te_compile("x*y^3", lookup, 2, &err);
    lok(expr3);
    lok(!err);

    te_expr *expr4 = te_compile("te_st+5", lookup, 3, &err);
    lok(expr4);
    lok(!err);

    for (y = 2; y < 3; ++y) {
        for (x = 0; x < 5; ++x) {
            double ev;

            ev = te_eval(expr1, NULL);
            lfequal(ev, cos(x) + sin(y));

            ev = te_eval(expr2, NULL);
            lfequal(ev, x+x+x-y);

            ev = te_eval(expr3, NULL);
            lfequal(ev, x*y*y*y);

            test = x;
            ev = te_eval(expr4, NULL);
            lfequal(ev, x+5);
        }
    }

    te_free(expr1);
    te_free(expr2);
    te_free(expr3);
    te_free(expr4);



    te_expr *expr5 = te_compile("xx*y^3", lookup, 2, &err);
    lok(!expr5);
    lok(err);

    te_expr *expr6 = te_compile("tes", lookup, 3, &err);
    lok(!expr6);
    lok(err);

    te_expr *expr7 = te_compile("sinn x", lookup, 2, &err);
    lok(!expr7);
    lok(err);

    te_expr *expr8 = te_compile("si x", lookup, 2, &err);
    lok(!expr8);
    lok(err);
}



#define cross_check(a, b) do {\
    if ((b)!=(b)) break;\
    expr = te_compile((a), lookup, 2, &err);\
    lfequal(te_eval(expr, NULL), (b));\
    lok(!err);\
    te_free(expr);\
}while(0)

static void test_functions() {

    static double x, y;
    te_variable lookup[] = { TE_DEF_VARIABLE("x", x), TE_DEF_VARIABLE("y", y) };

    int err;
    te_expr *expr;

    for (x = -5; x < 5; x += .2) {
        cross_check("abs x", fabs(x));
        cross_check("acos x", acos(x));
        cross_check("asin x", asin(x));
        cross_check("atan x", atan(x));
        cross_check("ceil x", ceil(x));
        cross_check("cos x", cos(x));
        cross_check("cosh x", cosh(x));
        cross_check("exp x", exp(x));
        cross_check("floor x", floor(x));
        cross_check("ln x", log(x));
        cross_check("log10 x", log10(x));
        cross_check("sin x", sin(x));
        cross_check("sinh x", sinh(x));
        cross_check("sqrt x", sqrt(x));
        cross_check("tan x", tan(x));
        cross_check("tanh x", tanh(x));
        cross_check("if(x>0, 1, 2)", (x > 0. ? 1. : 2.));

        for (y = -2; y < 2; y += .2) {
            if (fabs(x) < 0.01) break;
            cross_check("atan2(x,y)", atan2(x, y));
            cross_check("pow(x,y)", pow(x, y));
            cross_check("if(x!=y, x, y)", (x != y ? x : y));
        }
    }
}


#ifndef TE_WITHOUT_FUNCTION_0
static double sum0(void) {
    return 6;
}
#endif
static double sum1(double a) {
    return a * 2;
}
static double sum2(double a, double b) {
    return a + b;
}
static double sum3(double a, double b, double c) {
    return a + b + c;
}
#if TE_MAX_FUNCTION_ARITY >= 4
static double sum4(double a, double b, double c, double d) {
    return a + b + c + d;
}
#endif
#if TE_MAX_FUNCTION_ARITY >= 5
static double sum5(double a, double b, double c, double d, double e) {
    return a + b + c + d + e;
}
#endif
#if TE_MAX_FUNCTION_ARITY >= 6
static double sum6(double a, double b, double c, double d, double e, double f) {
    return a + b + c + d + e + f;
}
#endif
#if TE_MAX_FUNCTION_ARITY == 7
static double sum7(double a, double b, double c, double d, double e, double f, double g) {
    return a + b + c + d + e + f + g;
}
#endif


static void test_dynamic() {

    static double x, f;
    te_variable lookup[] = {
        TE_DEF_VARIABLE("x", x),
        TE_DEF_VARIABLE("f", f),
        TE_DEF_CONSTANT("six", 6),
#ifndef TE_WITHOUT_FUNCTION_0
        TE_DEF_FUNCTION("sum0", sum0, 0),
#endif
        TE_DEF_FUNCTION("sum1", sum1, 1),
        TE_DEF_FUNCTION("sum2", sum2, 2),
        TE_DEF_FUNCTION("sum3", sum3, 3),
#if TE_MAX_FUNCTION_ARITY >= 4
        TE_DEF_FUNCTION("sum4", sum4, 4),
#endif
#if TE_MAX_FUNCTION_ARITY >= 5
        TE_DEF_FUNCTION("sum5", sum5, 5),
#endif
#if TE_MAX_FUNCTION_ARITY >= 6
        TE_DEF_FUNCTION("sum6", sum6, 6),
#endif
#if TE_MAX_FUNCTION_ARITY == 7
        TE_DEF_FUNCTION("sum7", sum7, 7),
#endif
    };

    test_case cases[] = {
        {"x", 2},
        {"f+x", 7},
        {"x+x", 4},
        {"x+f", 7},
        {"f+f", 10},
        {"f+six", 11},
#ifndef TE_WITHOUT_FUNCTION_0
        {"f+sum0", 11},
        {"f+sum0()", 11},
        {"sum0+sum0", 12},
        {"sum0()+sum0", 12},
        {"sum0+sum0()", 12},
        {"sum0()+sum0()", 12},
        {"sum1 sum0", 12},
        {"sum1 sum0()", 12},
        {"sum1(sum0)", 12},
        {"sum1(sum0())", 12},
#endif
        {"sum1 f", 10},
        {"sum1 x", 4},
        {"sum1 six", 12},
        {"sum2 (sum1 x, x)", 6},
        {"sum2 (sum1(x), x)", 6},
        {"sum3 (sum1 x, x, 2)", 8},
        {"sum3 (sum1(x), x, 2)", 8},
        {"sum2(2,3)", 5},
        {"sum3(2,3,4)", 9},
#if TE_MAX_FUNCTION_ARITY >= 4
        {"sum4(2,3,4,5)", 14},
#endif
#if TE_MAX_FUNCTION_ARITY >= 5
        {"sum5(2,3,4,5,6)", 20},
#endif
#if TE_MAX_FUNCTION_ARITY >= 6
        {"sum6(2,3,4,5,6,7)", 27},
#endif
#if TE_MAX_FUNCTION_ARITY == 7
        {"sum7(2,3,4,5,6,7,8)", 35},
#endif
    };

    x = 2;
    f = 5;

    size_t i;
    for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i) {
        const char *expr = cases[i].expr;
        const double answer = cases[i].answer;

        int err;
        te_expr *ex = te_compile(expr, lookup, sizeof(lookup)/sizeof(te_variable), &err);
        lok(ex);
        lfequal(te_eval(ex, NULL), answer);
        te_free(ex);
    }
}


#ifndef TE_WITHOUT_CLOSURES
static double clo0(void *context) {
    if (context) return *((double*)context) + 6;
    return 6;
}
static double clo1(void *context, double a) {
    if (context) return *((double*)context) + a * 2;
    return a * 2;
}
static double clo2(void *context, double a, double b) {
    if (context) return *((double*)context) + a + b;
    return a + b;
}

static double cell(void *context, double a) {
    double *c = context;
    return c[(int)a];
}

static void test_closure() {

    static double extra;
    static double c[] = {5,6,7,8,9};

    te_variable lookup[] = {
      TE_DEF_CLOSURE("c0", clo0, 0, &extra),
      TE_DEF_CLOSURE("c1", clo1, 1, &extra),
      TE_DEF_CLOSURE("c2", clo2, 2, &extra),
      TE_DEF_CLOSURE("cell", cell, 1, c),
    };

    test_case cases[] = {
        {"c0", 6},
        {"c1 4", 8},
        {"c2 (10, 20)", 30},
    };

    size_t i;
    for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i) {
        const char *expr = cases[i].expr;
        const double answer = cases[i].answer;

        int err;
        te_expr *ex = te_compile(expr, lookup, sizeof(lookup)/sizeof(te_variable), &err);
        lok(ex);

        extra = 0;
        lfequal(te_eval(ex, NULL), answer + extra);

        extra = 10;
        lfequal(te_eval(ex, NULL), answer + extra);

        te_free(ex);
    }


    test_case cases2[] = {
        {"cell 0", 5},
        {"cell 1", 6},
        {"cell 0 + cell 1", 11},
        {"cell 1 * cell 3 + cell 4", 57},
    };

    for (i = 0; i < sizeof(cases2) / sizeof(test_case); ++i) {
        const char *expr = cases2[i].expr;
        const double answer = cases2[i].answer;

        int err;
        te_expr *ex = te_compile(expr, lookup, sizeof(lookup)/sizeof(te_variable), &err);
        lok(ex);
        lfequal(te_eval(ex, NULL), answer);
        te_free(ex);
    }
}
#endif

static void test_optimize() {

    test_case cases[] = {
        {"5+5", 10},
        {"pow(2,2)", 4},
        {"sqrt 100", 10},
        {"pi * 2", 6.2832},
    };

    size_t i;
    for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i) {
        const char *expr = cases[i].expr;
        const double answer = cases[i].answer;

        int err;
        te_expr *ex = te_compile(expr, 0, 0, &err);
        lok(ex);

        /* The answer should be know without
         * even running eval. */
        lfequal(ex->v.value, answer);
        lfequal(te_eval(ex, NULL), answer);

        te_free(ex);
    }
}

static void test_pow() {
#ifdef TE_POW_FROM_RIGHT
    test_equ cases[] = {
        {"2^3^4", "2^(3^4)"},
        {"-2^2", "-(2^2)"},
        {"--2^2", "(2^2)"},
        {"---2^2", "-(2^2)"},
        {"-(2)^2", "-(2^2)"},
        {"-(2*1)^2", "-(2^2)"},
        {"-2^2", "-4"},
        {"2^1.1^1.2^1.3", "2^(1.1^(1.2^1.3))"},
        {"-a^b", "-(a^b)"},
        {"-a^-b", "-(a^-b)"}
    };
#else
    test_equ cases[] = {
        {"2^3^4", "(2^3)^4"},
        {"-2^2", "(-2)^2"},
        {"--2^2", "2^2"},
        {"---2^2", "(-2)^2"},
        {"-2^2", "4"},
        {"2^1.1^1.2^1.3", "((2^1.1)^1.2)^1.3"},
        {"-a^b", "(-a)^b"},
        {"-a^-b", "(-a)^(-b)"}
    };
#endif

    static double a = 2, b = 3;

    te_variable lookup[] = { TE_DEF_VARIABLE("a", a), TE_DEF_VARIABLE("b", b) };

    size_t i;
    for (i = 0; i < sizeof(cases) / sizeof(test_equ); ++i) {
        const char *expr1 = cases[i].expr1;
        const char *expr2 = cases[i].expr2;

        te_expr *ex1 = te_compile(expr1, lookup, sizeof(lookup)/sizeof(te_variable), 0);
        te_expr *ex2 = te_compile(expr2, lookup, sizeof(lookup)/sizeof(te_variable), 0);

        lok(ex1);
        lok(ex2);

        double r1 = te_eval(ex1, NULL);
        double r2 = te_eval(ex2, NULL);

        fflush(stdout);
        lfequal(r1, r2);

        te_free(ex1);
        te_free(ex2);
    }

}

static void test_combinatorics() {
    test_case cases[] = {
            {"fac(0)", 1},
            {"fac(0.2)", 1},
            {"fac(1)", 1},
            {"fac(2)", 2},
            {"fac(3)", 6},
            {"fac(4.8)", 24},
            {"fac(10)", 3628800},

            {"ncr(0,0)", 1},
            {"ncr(10,1)", 10},
            {"ncr(10,0)", 1},
            {"ncr(10,10)", 1},
            {"ncr(16,7)", 11440},
            {"ncr(16,9)", 11440},
            {"ncr(100,95)", 75287520},

            {"npr(0,0)", 1},
            {"npr(10,1)", 10},
            {"npr(10,0)", 1},
            {"npr(10,10)", 3628800},
            {"npr(20,5)", 1860480},
            {"npr(100,4)", 94109400},
    };


    size_t i;
    for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i) {
        const char *expr = cases[i].expr;
        const double answer = cases[i].answer;

        int err;
        const double ev = te_interp(expr, &err);
        lok(!err);
        lfequal(ev, answer);

        if (err) {
            printf("FAILED: %s (%d)\n", expr, err);
        }
    }
}


int main(int argc, char *argv[])
{
    (void)argc; (void)argv;
    lrun("Results", test_results);
    lrun("Syntax", test_syntax);
    lrun("NaNs", test_nans);
    lrun("INFs", test_infs);
    lrun("Variables", test_variables);
    lrun("Functions", test_functions);
    lrun("Dynamic", test_dynamic);
#ifndef TE_WITHOUT_CLOSURES
    lrun("Closure", test_closure);
#endif
    lrun("Optimize", test_optimize);
    lrun("Pow", test_pow);
    lrun("Combinatorics", test_combinatorics);
    lresults();

    return lfails != 0;
}
