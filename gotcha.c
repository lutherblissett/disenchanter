#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>

#ifndef __CC65__
#define HAVE_FLOAT 1
#endif

#if  __STDC_VERSION>=199901L
#define HAVE_C99 1
#endif

#if __GNUC__ || HAVE_C99
#define HAVE_VLA 1
#endif

#ifdef HAVE_FLOAT
#include <math.h>
#endif
/*
 Some tests can crash the program. Define -DALLOW_CRASH=1 to enable these
*/

#ifndef ALLOW_CRASH
#define ALLOW_CRASH 0
#endif

int count=0;
int total=0;
int fail=0;

void expect(const char *info, const char *expr)
{
    printf("..%s\n   but '%s' is false.\n",info,expr);
    fflush(stdout);
    count++;
}
#define EXPECT(INFO,EXPR) if (fail=0, total++,!(EXPR)) expect((fail=1,(INFO)),#EXPR)

/* stack check..How can I do this better? */
ptrdiff_t check_grow(int k, int *p)
{
    if (p==0) p=&k;
    if (k==0) return &k-p;
    else return check_grow(k-1,p);
}

#define BITS_PER_INT (sizeof(int)*CHAR_BIT)

int bits_per_int=BITS_PER_INT;
int int_max=INT_MAX;
int int_min=INT_MIN;

/* for 21 - left to right */
int ltr_result=0;
unsigned ltr_fun(int k)
{
    ltr_result=ltr_result*10+k;
    return 1;
}
int gobble_args(int dummy, ...)
{
    return dummy;
}

#if ALLOW_CRASH
#if HAVE_FLOAT
float sum_variadic_floats(unsigned int n, ...)
{
    float v = 0;
    va_list ap;
    va_start(ap, n);
    while (n--)
        v += va_arg(ap, float);
    va_end(ap);
    return v;
}
#endif

char sum_variadic_chars(unsigned int n, ...)
{
    char v = 0;
    va_list ap;
    va_start(ap, n);
    while (n--)
        v += va_arg(ap, char);
    va_end(ap);
    return v;
}
#endif

int main()
{
    printf("We like to think that:\n");
    /* characters */
    EXPECT("00 we have ASCII",('A'==65));
    EXPECT("01 A-Z is in a block",('Z'-'A')+1==26);
    EXPECT("02 big letters come before small letters",('A'<'a'));
    EXPECT("03 a char is 8 bits",CHAR_BIT==8);
    EXPECT("04 a char is signed",CHAR_MIN==SCHAR_MIN);

    /* integers */
    EXPECT("05 int has the size of pointers",sizeof(int)==sizeof(void*));
    /* not true for Windows-64 */
    EXPECT("05a long has at least the size of pointers",sizeof(long)>=sizeof(void*));

    EXPECT("06 integers are 2-complement and wrap around",(int_max+1)==(int_min));
    EXPECT("07 integers are 2-complement and *always* wrap around",(INT_MAX+1)==(INT_MIN));
    EXPECT("08 overshifting is okay",(1<<bits_per_int)==0);
    EXPECT("09 overshifting is *always* okay",(1<<BITS_PER_INT)==0);
    {
        int t;
        EXPECT("09a minus shifts backwards",(t=-1,(15<<t)==7));
    }
    /* pointers */
    /* Suggested by jalf */
    EXPECT("10 void* can store function pointers",sizeof(void*)>=sizeof(void(*)()));
    /* execution */
    EXPECT("11 Detecting how the stack grows is easy",check_grow(5,0)!=check_grow(6,0));
    EXPECT("12 the stack grows downwards",check_grow(5,0)<0);

    {
        int t;
        /* suggested by jk */
        EXPECT("13 The smallest bits come always first",(t=0x1234,0x34==*(char*)&t));
    }
    {
        /* Suggested by S.Lott */
        int a[2]={0,0};
        int i=0;
        EXPECT("14 i++ is strictly left to right",(i=0,a[i++]=i,a[0]==1));
    }
    {
        struct {
            char c;
            int i;
        } char_int;
        EXPECT("15 structs are packed",sizeof(char_int)==(sizeof(char)+sizeof(int)));
    }

    {
        /* CERT:MEM04-C */
        EXPECT("16 malloc()=NULL means out of memory",(malloc(0)!=NULL));
    }

    /* suggested by David Thornley */
    EXPECT("17 size_t is unsigned int",sizeof(size_t)==sizeof(unsigned int));
    /* this is true for C99, but unspec. for C90. */
    EXPECT("18 a%b has the same sign as a",((-10%3)==-1) && ((10%-3)==1));

    /* suggested by nos */
    EXPECT("19-1 char<short",sizeof(char)<sizeof(short));
    EXPECT("19-2 short<int",sizeof(short)<sizeof(int));
    EXPECT("19-3 int<long",sizeof(int)<sizeof(long));
    EXPECT("20 ptrdiff_t and size_t have the same size",(sizeof(ptrdiff_t)==sizeof(size_t)));
#if ALLOW_CRASH
    {
        /* suggested by R. */
        /* this crashes on TC 3.0++, compact. */
        char buf[10];
        EXPECT("21 You can use snprintf to append a string",
               (snprintf(buf,10,"OK"),snprintf(buf,10,"%s!!",buf),strcmp(buf,"OK!!")==0));
    }
#endif
    /* suggested by Prasoon Saurav */
    EXPECT("21 Evaluation for +,* is left to right",
           (ltr_fun(1)*ltr_fun(2)+ltr_fun(3)*ltr_fun(4),ltr_result==1234));
    if (fail) printf("ltr_result is %d in this case\n",ltr_result);
    ltr_result=0;

    EXPECT("21a Function Arguments are evaluated right to left",
           (gobble_args(0,ltr_fun(1),ltr_fun(2),ltr_fun(3),ltr_fun(4)),ltr_result==4321));
    if (fail) printf("ltr_result is %d in this case\n",ltr_result);

    {
#ifdef __STDC_IEC_559__
        int STDC_IEC_559_is_defined=1;
#else
        /* This either means, there is no FP support
         *or* the compiler is not C99 enough to define  __STDC_IEC_559__
         *or* the FP support is not IEEE compliant.
         */
        int STDC_IEC_559_is_defined=0;
#endif
        EXPECT("22 floating point is always IEEE",STDC_IEC_559_is_defined);
    }
#if ALLOW_CRASH
#if HAVE_FLOAT
    EXPECT("23 floats can be used in variadics",sum_variadic_floats(3,2.0f,4.0f,8.0f)==14.0f);
#endif
    EXPECT("24 char can be used in variadics",sum_variadic_chars(3,(char)2,(char)4,(char)8)==14);
#endif

    {
        /* CERT:ARR36-C,CERT:ARR37-C */
        struct {
            int int1;
            char c;
            int int2;
        } var;
        ptrdiff_t diff;
        EXPECT("25 pointer arithmetic works outside arrays",(diff=&var.int2-&var.int1, &var.int1+diff==&var.int2));
    }
    {
        /* This is not just a packing problem: */
        struct data
        {
            char data[17];
        };
        struct data p1;
        struct data p2;
        ptrdiff_t diff=&p1-&p2;
        EXPECT("25a pointer arithmetic works outside arrays",(diff=&p1-&p2, &p2+diff==&p1));
    }
#if HAVE_VLA
    {
        /* This can happen with C99. */
        int i;
        EXPECT("26 sizeof() does not evaluate its arguments",(i=10,sizeof(char[((i=20),10)]),i==10));
    }
#endif
#if HAVE_FLOAT
    /* suggested by dan04. We need netter numbers */
    EXPECT("27 pow() gives exact results for integer arguments", pow(2,4) == 16);
#endif
    printf("From what I can say with my puny test cases, you are %d%% mainstream\n",100-(100*count)/total);
    return 0;
}
