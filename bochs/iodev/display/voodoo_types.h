/////////////////////////////////////////////////////////////////////////
// $Id: voodoo_types.h 13327 2017-10-23 20:01:25Z vruppert $
/////////////////////////////////////////////////////////////////////////

#include <math.h>

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef Bit32u offs_t;

/* poly_param_extent describes information for a single parameter in an extent */
typedef struct _poly_param_extent poly_param_extent;
struct _poly_param_extent
{
  float   start;            /* parameter value at starting X,Y */
  float   dpdx;           /* dp/dx relative to starting X */
};


#define MAX_VERTEX_PARAMS         6
/* poly_extent describes start/end points for a scanline, along with per-scanline parameters */
typedef struct _poly_extent poly_extent;
struct _poly_extent
{
  Bit16s    startx;           /* starting X coordinate (inclusive) */
  Bit16s    stopx;            /* ending X coordinate (exclusive) */
  poly_param_extent param[MAX_VERTEX_PARAMS]; /* starting and dx values for each parameter */
};

#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif


/* an rgb_t is a single combined R,G,B (and optionally alpha) value */
typedef Bit32u rgb_t;

/* an rgb15_t is a single combined 15-bit R,G,B value */
typedef Bit16u rgb15_t;

/* macros to assemble rgb_t values */
#define MAKE_ARGB(a,r,g,b)  ((((rgb_t)(a) & 0xff) << 24) | (((rgb_t)(r) & 0xff) << 16) | (((rgb_t)(g) & 0xff) << 8) | ((rgb_t)(b) & 0xff))
#define MAKE_RGB(r,g,b)   (MAKE_ARGB(255,r,g,b))

/* macros to extract components from rgb_t values */
#define RGB_ALPHA(rgb)    (((rgb) >> 24) & 0xff)
#define RGB_RED(rgb)    (((rgb) >> 16) & 0xff)
#define RGB_GREEN(rgb)    (((rgb) >> 8) & 0xff)
#define RGB_BLUE(rgb)   ((rgb) & 0xff)

/* common colors */
#define RGB_BLACK     (MAKE_ARGB(255,0,0,0))
#define RGB_WHITE     (MAKE_ARGB(255,255,255,255))

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    rgb_to_rgb15 - convert an RGB triplet to
    a 15-bit OSD-specified RGB value
-------------------------------------------------*/

BX_CPP_INLINE rgb15_t rgb_to_rgb15(rgb_t rgb)
{
  return ((RGB_RED(rgb) >> 3) << 10) | ((RGB_GREEN(rgb) >> 3) << 5) | ((RGB_BLUE(rgb) >> 3) << 0);
}


/*-------------------------------------------------
    rgb_clamp - clamp an RGB component to 0-255
-------------------------------------------------*/

BX_CPP_INLINE Bit8u rgb_clamp(Bit32s value)
{
  if (value < 0)
    return 0;
  if (value > 255)
    return 255;
  return value;
}


/*-------------------------------------------------
    pal1bit - convert a 1-bit value to 8 bits
-------------------------------------------------*/

BX_CPP_INLINE Bit8u pal1bit(Bit8u bits)
{
  return (bits & 1) ? 0xff : 0x00;
}


/*-------------------------------------------------
    pal2bit - convert a 2-bit value to 8 bits
-------------------------------------------------*/

BX_CPP_INLINE Bit8u pal2bit(Bit8u bits)
{
  bits &= 3;
  return (bits << 6) | (bits << 4) | (bits << 2) | bits;
}


/*-------------------------------------------------
    pal3bit - convert a 3-bit value to 8 bits
-------------------------------------------------*/

BX_CPP_INLINE Bit8u pal3bit(Bit8u bits)
{
  bits &= 7;
  return (bits << 5) | (bits << 2) | (bits >> 1);
}


/*-------------------------------------------------
    pal4bit - convert a 4-bit value to 8 bits
-------------------------------------------------*/

BX_CPP_INLINE Bit8u pal4bit(Bit8u bits)
{
  bits &= 0xf;
  return (bits << 4) | bits;
}


/*-------------------------------------------------
    pal5bit - convert a 5-bit value to 8 bits
-------------------------------------------------*/

BX_CPP_INLINE Bit8u pal5bit(Bit8u bits)
{
  bits &= 0x1f;
  return (bits << 3) | (bits >> 2);
}


/*-------------------------------------------------
    pal6bit - convert a 6-bit value to 8 bits
-------------------------------------------------*/

BX_CPP_INLINE Bit8u pal6bit(Bit8u bits)
{
  bits &= 0x3f;
  return (bits << 2) | (bits >> 4);
}


/*-------------------------------------------------
    pal7bit - convert a 7-bit value to 8 bits
-------------------------------------------------*/

BX_CPP_INLINE Bit8u pal7bit(Bit8u bits)
{
  bits &= 0x7f;
  return (bits << 1) | (bits >> 6);
}


#define WORK_MAX_THREADS      16

/* rectangles describe a bitmap portion */
typedef struct _rectangle rectangle;
struct _rectangle
{
  int       min_x;      /* minimum X, or left coordinate */
  int       max_x;      /* maximum X, or right coordinate (inclusive) */
  int       min_y;      /* minimum Y, or top coordinate */
  int       max_y;      /* maximum Y, or bottom coordinate (inclusive) */
};

/* Standard MIN/MAX macros */
#ifndef MIN
#define MIN(x,y)      ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y)      ((x) > (y) ? (x) : (y))
#endif


BX_CPP_INLINE float u2f(Bit32u v)
{
  union {
    float ff;
    Bit32u vv;
  } u;
  u.vv = v;
  return u.ff;
}


#define ACCESSING_BITS_0_15       ((mem_mask & 0x0000ffff) != 0)
#define ACCESSING_BITS_16_31      ((mem_mask & 0xffff0000) != 0)

// constants for expression endianness
enum endianness_t
{
  ENDIANNESS_LITTLE,
  ENDIANNESS_BIG
};
const endianness_t ENDIANNESS_NATIVE = ENDIANNESS_LITTLE;
#define ENDIAN_VALUE_LE_BE(endian,leval,beval)  (((endian) == ENDIANNESS_LITTLE) ? (leval) : (beval))
#define NATIVE_ENDIAN_VALUE_LE_BE(leval,beval)  ENDIAN_VALUE_LE_BE(ENDIANNESS_NATIVE, leval, beval)
#define BYTE4_XOR_LE(a)         ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,3))

#define BYTE_XOR_LE(a)          ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,1))

#define profiler_mark_start(x)  do { } while (0)
#define profiler_mark_end()   do { } while (0)


/* Highly useful macro for compile-time knowledge of an array size */
#define ARRAY_LENGTH(x)   (sizeof(x) / sizeof(x[0]))

#define mul_32x32_shift _mul_32x32_shift
BX_CPP_INLINE Bit32s _mul_32x32_shift(Bit32s a, Bit32s b, Bit8s shift)
{
  Bit64s tmp = (Bit64s)a * (Bit64s)b;
  tmp >>= shift;
  Bit32s result = (Bit32s)tmp;

  return result;
}


BX_CPP_INLINE rgb_t rgba_bilinear_filter(rgb_t rgb00, rgb_t rgb01, rgb_t rgb10, rgb_t rgb11, Bit8u u, Bit8u v)
{
  Bit32u ag0, ag1, rb0, rb1;

  rb0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
  rb1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);
  rgb00 >>= 8;
  rgb01 >>= 8;
  rgb10 >>= 8;
  rgb11 >>= 8;
  ag0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
  ag1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);

  rb0 = (rb0 & 0x00ff00ff) + ((((rb1 & 0x00ff00ff) - (rb0 & 0x00ff00ff)) * v) >> 8);
  ag0 = (ag0 & 0x00ff00ff) + ((((ag1 & 0x00ff00ff) - (ag0 & 0x00ff00ff)) * v) >> 8);

  return ((ag0 << 8) & 0xff00ff00) | (rb0 & 0x00ff00ff);
}

typedef struct _poly_vertex poly_vertex;
struct _poly_vertex
{
  float   x;              /* X coordinate */
  float   y;              /* Y coordinate */
  float   p[MAX_VERTEX_PARAMS];   /* interpolated parameter values */
};


typedef struct _tri_extent tri_extent;
struct _tri_extent
{
  Bit16s    startx;           /* starting X coordinate (inclusive) */
  Bit16s    stopx;            /* ending X coordinate (exclusive) */
};

/* tri_work_unit is a triangle-specific work-unit */
typedef struct _tri_work_unit tri_work_unit;
struct _tri_work_unit
{
  tri_extent      extent[8]; /* array of scanline extents */
};
