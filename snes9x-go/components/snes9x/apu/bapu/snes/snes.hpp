#ifndef __SNES_HPP
#define __SNES_HPP

#include "../../../snes9x.h"
#include "../../resampler.h"

#if defined(__GNUC__)
  #define inline        inline
  #define alwaysinline  inline __attribute__((always_inline))
#elif defined(_MSC_VER)
  #define inline        inline
  #define alwaysinline  inline __forceinline
#else
  #define inline        inline
  #define alwaysinline  inline
#endif

#define debugvirtual

namespace SNES
{
struct Processor
{
    unsigned frequency;
    int32 clock;
};

#include "bapu/dsp/blargg_endian.h"
#include "../smp/smp.hpp"
#include "../dsp/sdsp.hpp"
} // namespace SNES

#endif
