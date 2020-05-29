#ifndef _ORIGA_TYPE_H_
#define _ORIGA_TYPE_H_

#include <stdlib.h>

typedef signed char BYTE;
typedef unsigned char UBYTE;
typedef unsigned char uint8_t;

typedef short WORD;
typedef unsigned short UWORD;
typedef unsigned short uint16_t;

typedef long LONG;
typedef unsigned long ULONG;
//typedef unsigned long uint32_t;

typedef unsigned char BOOL;

#define  BYTE_MAX (0xFF)
#define UBYTE_MAX (0xFFu)
#define  WORD_MAX (0xFFFF)
#define UWORD_MAX (0xFFFFu)
//#define  LONG_MAX (0xFFFFFFFF)
//#define ULONG_MAX (0xFFFFFFFFu)
#define  QUAD_MAX (0xFFFFFFFFFFFFFFFF)
#define UQUAD_MAX (0xFFFFFFFFFFFFFFFFu)


typedef UWORD gf2n_t[9];

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

//*** MACROS ***
#define LOBYTE( uwData )	( (UBYTE) ((uwData) & 0xFFu) )
#define HIBYTE( uwData )	( (UBYTE)(((uwData) >> 8) & 0xFFu) )


#endif
