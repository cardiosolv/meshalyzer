//// HEADER GUARD ///////////////////////////
// If automatically generated, keep above
// comment as first line in file.
#ifndef __SHORT_FLOAT_H__
#define __SHORT_FLOAT_H__
//// HEADER GUARD ///////////////////////////

/*
ftp://ftp.fox-toolkit.org/pub/fasthalffloatconversion.pdf

Fast Half Float Conversions
Jeroen van der Zijp
November 2008
(Revised September 2010)

Use the dumpTables code to generate the corresponding C file.
*/


#include <cstdint>


extern const uint16_t basetable[512];
extern const uint16_t shifttable[512];
extern const uint32_t mantissatable[2048];
extern const uint16_t offsettable[64];
extern const uint32_t exponenttable[64];

typedef uint16_t short_float;

static inline short_float shortFromFloat(const float external_ff) {
  union {
    float as_float;
    uint32_t as_uint32;
  } c;
  c.as_float = external_ff;
  return basetable[(c.as_uint32>>23)&0x1ff]+((c.as_uint32&0x007fffff)>>shifttable[(c.as_uint32>>23)&0x1ff]);
}

static inline float floatFromShort(const short_float h) {
  union {
    float as_float;
    uint32_t as_uint32;
  } c;
  c.as_uint32 = mantissatable[offsettable[h>>10]+(h&0x3ff)]+exponenttable[h>>10];
  return c.as_float;
}

#define SHORT_FLOAT_MACHINE_EPS 0.0009765625

//// HEADER GUARD ///////////////////////////
#endif
//// HEADER GUARD ///////////////////////////
