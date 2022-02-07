///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2019, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// \brief Tiny printf, sprintf and snprintf implementation, optimized for speed on
//        embedded systems with a very limited resources.
//        Use this instead of bloated standard/newlib printf.
//        These routines are thread safe and reentrant.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _PRINTF_H_
#define _PRINTF_H_

#ifdef RG_REPLACE_PRINTF_FUNCTIONS
#define printf(...)         rg_fprintf(stdout, __VA_ARGS__)
#define vprintf(...)        rg_vfprintf(stdout, __VA_ARGS__)
#define fprintf(...)        rg_fprintf(__VA_ARGS__)
#define fvprintf(...)       rg_vfprintf(__VA_ARGS__)
#define sprintf(...)        rg_sprintf(__VA_ARGS__)
#define snprintf(...)       rg_snprintf(__VA_ARGS__)
#define vsnprintf(...)      rg_vsnprintf(__VA_ARGS__)
#endif

#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Tiny printf with output function
 * \param out An output function which takes one character and an argument pointer
 * \param arg An argument pointer for user data passed to output function
 * \param format A string that specifies the format of the output
 * \param va A value identifying a variable arguments list
 * \return The number of characters that are sent to the output function, not counting the terminating null character
 */
int rg_xprintf(void (*out)(int ch, void* arg, size_t idx, size_t maxlen), void* arg, const char* format, ...) __attribute__((format(printf,3,4)));
int rg_vxprintf(void (*out)(int ch, void* arg, size_t idx, size_t maxlen), void* arg, const char* format, va_list va);


/**
 * Tiny fprintf/rg_vfprintf implementation
 * \param fp A file pointer (FILE*)
 * \param format A string that specifies the format of the output
 * \param va A value identifying a variable arguments list
 * \return The number of characters that are sent to stdout, not counting the terminating null character
 */
int rg_fprintf(void *fp, const char* format, ...) __attribute__((format(printf,2,3)));
int rg_vfprintf(void *fp, const char* format, va_list va);


/**
 * Tiny sprintf implementation
 * Due to security reasons (buffer overflow) YOU SHOULD CONSIDER USING (V)SNPRINTF INSTEAD!
 * \param buffer A pointer to the buffer where to store the formatted string. MUST be big enough to store the output!
 * \param format A string that specifies the format of the output
 * \return The number of characters that are WRITTEN into the buffer, not counting the terminating null character
 */
int rg_sprintf(char* buffer, const char* format, ...) __attribute__((format(printf,2,3)));


/**
 * Tiny snprintf/vsnprintf implementation
 * \param buffer A pointer to the buffer where to store the formatted string
 * \param count The maximum number of characters to store in the buffer, including a terminating null character
 * \param format A string that specifies the format of the output
 * \param va A value identifying a variable arguments list
 * \return The number of characters that COULD have been written into the buffer, not counting the terminating
 *         null character. A value equal or larger than count indicates truncation. Only when the returned value
 *         is non-negative and less than count, the string has been completely written.
 */
int rg_snprintf(char* buffer, size_t count, const char* format, ...) __attribute__((format(printf,3,4)));
int rg_vsnprintf(char* buffer, size_t count, const char* format, va_list va);


#ifdef __cplusplus
}
#endif


#endif  // _PRINTF_H_
