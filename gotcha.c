#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
int count=0;
int total=0;
void expect(const char *info, const char *expr)
{
    printf("..%s\n   but '%s' is false.\n",info,expr);
    count++;
}
#define EXPECT(INFO,EXPR) if (total++,!(EXPR)) expect(INFO,#EXPR)

// stack check..How can I do this better?
int check_grow(int k, int *p)
{
    if (p==0) p=&k;
    if (k==0) return &k-p;
    else return check_grow(k-1,p);
}
#define BITS_PER_INT (sizeof(int)*CHAR_BIT)

int bits_per_int=BITS_PER_INT;
int int_max=INT_MAX;
int int_min=INT_MIN;

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
    EXPECT("06 integers are 2-complement and wrap around",(int_max+1)==(int_min));
    EXPECT("07 integers are 2-complement and *always* wrap around",(INT_MAX+1)==(INT_MIN));
    EXPECT("08 overshifting is okay",(1<<bits_per_int)==0);
    EXPECT("09 overshifting is *always* okay",(1<<BITS_PER_INT)==0);
    {
        int t=-1;
        EXPECT("09a minus shifts backwards",(15<<t)==7);
    }
    /* pointers */
    EXPECT("10 void* can store function pointers",sizeof(void*)>=sizeof(void(*)()));
    /* execution */
    EXPECT("11 Detecting how the stack is easy",check_grow(5,0)!=0);
    EXPECT("12 the stack grows downwards",check_grow(5,0)<0);

    {
        int t;
        EXPECT("13 The smallest bits come always first",(t=0x1234,0x34==*(char*)&t));
    }
    {
        int a[2]={0,0};
        int i=0;
        EXPECT("14 i++ is left to right",(i=0,a[i++]=i,a[0]==1));
    }
    {
        // this could also be "everything's aligned to sizeof" instead
        struct {
            char c;
            int i;
        } char_int;
        EXPECT("15 structs are packed",sizeof(char_int)==(sizeof(char)+sizeof(int)));
    }
    {
        EXPECT("16 malloc()=NULL means out of memory",(malloc(0)!=NULL));
    }

    EXPECT("17 size_t == unsigned int",sizeof(size_t)==sizeof(unsigned int));
    EXPECT("18 a%b has the same sign as a",((-10%3)==-1) && ((10%-3)==1));

    printf("From what I can say with my puny test cases, you are %d%% mainstream\n",100-(100*count)/total);
    return 0;
}
