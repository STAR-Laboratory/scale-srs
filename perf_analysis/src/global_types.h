#ifndef __GLOBAL_TYPES_H__

#define FALSE 0
#define TRUE  1

#define HIT   1
#define MISS  0


#define CLOCK_INC_FACTOR 4

#define MAX_UNS 0xffffffff

#define ASSERTM(cond, msg...) if(!(cond) ){ printf(msg); fflush(stdout);} assert(cond);
#define DBGMSG(cond, msg...) if(cond){ printf(msg); fflush(stdout);} 

#define SAT_INC(x,max)   (x<max)? x+1:x
#define SAT_DEC(x)       (x>0)? x-1:0


/* Renames -- Try to use these rather than built-in C types for portability */


typedef unsigned	    uns;
typedef unsigned char	    uns8;
typedef unsigned short	    uns16;
typedef unsigned	    uns32;
typedef unsigned long long  uns64;
typedef short		    int16;
typedef int		    int32;
typedef int long long	    int64;
typedef int		    Generic_Enum;


/* Conventions */
typedef uns64		    Addr;
typedef uns32		    Binary;
typedef uns8		    Flag;

typedef uns64               Counter;
typedef int64               SCounter;


/**************************************************************************************/

#define __GLOBAL_TYPES_H__
#endif  
