#ifndef _WL_ATMOS_H_
#define _WL_ATMOS_H_

#if defined(USE_STARSKY) || defined(USE_RAIN) || defined(USE_SNOW)
    void Init3DPoints();
#endif

#ifdef USE_STARSKY
    void DrawStarSky(byte *vbuf, uint32_t vbufPitch);
#endif

#ifdef USE_RAIN
    void DrawRain(byte *vbuf, uint32_t vbufPitch);
#endif

#ifdef USE_SNOW
    void DrawSnow(byte *vbuf, uint32_t vbufPitch);
#endif

#endif
