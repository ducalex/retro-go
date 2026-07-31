#ifndef __AUDIOLIB_ASSERT_H
#define __AUDIOLIB_ASSERT_H

#ifdef NDEBUG
    #define ROTT_ASSERT(f)
#else
    #define ROTT_ASSERT(f) \
        if (!(f)) _Assert(__FILE__, __LINE__)
    extern void _Assert(char *strFile, unsigned uLine);
#endif

#endif
