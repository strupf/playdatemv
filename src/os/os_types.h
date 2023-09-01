// =============================================================================
// Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
// =============================================================================

#ifndef OS_TYPES_H
#define OS_TYPES_H

#include "binary_literal.h"

// this is to enable editing Playdate specific stuff
// and disable Visual Studio's definitions
#if 0
#undef TARGET_DESKTOP
#define TARGET_PD
#define TARGET_EXTENSION 1
#endif

#if defined(TARGET_DESKTOP)
// RAYLIB ======================================================================
#include "include/raylib.h"
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define os_time       (float)GetTime
#define ASSERT        assert
#define STATIC_ASSERT static_assert
#define PRINTF        printf
#elif defined(TARGET_PD)
// PLAYDATE ====================================================================
#include "pd_api.h"
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#define os_time PD_elapsedtime

extern PlaydateAPI *PD;
extern void (*PD_log)(const char *fmt, ...);
extern float (*PD_elapsedtime)();
#define ASSERT(E)
#define STATIC_ASSERT(E, M)

#if defined(TARGET_PD_HW)
#include <arm_acle.h>  // cortex m7 simd intrinsics?
#define PRINTF(C, ...) // disable printf calls
#else
#define PRINTF PD_log
#endif
//
#endif
// =============================================================================

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;
typedef int8_t         i8;
typedef int16_t        i16;
typedef int32_t        i32;
typedef int64_t        i64;
typedef uint8_t        u8;
typedef uint16_t       u16;
typedef uint32_t       u32;
typedef uint64_t       u64;
typedef int8_t         bool8;
typedef int16_t        bool16;
typedef int32_t        bool32;

typedef u8  flags8;
typedef u16 flags16;
typedef u32 flags32;

// 32-bit SIMD types
typedef i32 i16x2;
typedef u32 u16x2;
typedef i32 i8x4;
typedef u32 u8x4;

#define I64_MAX INT64_MAX
#define I64_MIN INT64_MIN
#define U64_MAX UINT64_MAX
#define U64_MIN 0
#define I32_MAX INT32_MAX
#define I32_MIN INT32_MIN
#define U32_MAX UINT32_MAX
#define U32_MIN 0
#define I16_MAX INT16_MAX
#define I16_MIN INT16_MIN
#define U16_MAX UINT16_MAX
#define U16_MIN 0
#define I8_MAX  INT8_MAX
#define I8_MIN  INT8_MIN
#define U8_MAX  UINT8_MAX
#define U8_MIN  0

#define NOT_IMPLEMENTED  ASSERT(0);
#define GLUE2(A, B)      A##B
#define GLUE(A, B)       GLUE2(A, B)
#define ARRLEN(A)        (sizeof(A) / sizeof(A[0]))
#define MAX(A, B)        ((A) >= (B) ? (A) : (B))
#define MIN(A, B)        ((A) <= (B) ? (A) : (B))
#define ABS(A)           ((A) >= 0 ? (A) : -(A))
#define SGN(A)           ((0 < (A)) - (0 > (A)))
#define CLAMP(X, LO, HI) ((X) > (HI) ? (HI) : ((X) < (LO) ? (LO) : (X)))
#define SWAP(T, a, b)          \
        do {                   \
                T tmp_ = a;    \
                a      = b;    \
                b      = tmp_; \
        } while (0)
#define FILE_AND_LINE__(A, B) A "|" #B
#define FILE_AND_LINE_(A, B)  FILE_AND_LINE__(A, B)
#define FILE_AND_LINE         FILE_AND_LINE_(__FILE__, __LINE__)
#define ALIGNAS               _Alignas

#define char_isdigit(C)   ('0' <= (C) && (C) <= '9')
#define char_digit        char_isdigit
#define char_digit_1_9(C) ('1' <= (C) && (C) <= '9')

static int char_hex_to_int(char c)
{
        switch (c) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'A':
        case 'a': return 10;
        case 'B':
        case 'b': return 11;
        case 'C':
        case 'c': return 12;
        case 'D':
        case 'd': return 13;
        case 'E':
        case 'e': return 14;
        case 'F':
        case 'f': return 15;
        }
        return 0;
}

static bool32 char_matches_any(const char c, const char *chars)
{
        for (const char *cc = chars; *cc != '\0'; cc++) {
                if (*cc == c) return 1;
        }
        return 0;
}

static bool32 streq(const char *str1, const char *str2)
{
        ASSERT(str1 && str2);
        for (int i = 0; str1[i] != '\0' && str2[i] != '\0'; i++) {
                if (str1[i] != str2[i]) return 0;
        }
        return 1;
}

static int os_strlen(const char *s)
{
        ASSERT(s);
        int l = 0;
        while (s[l] != '\0') {
                l++;
        }
        return l;
}

// copies src to dst INCLUDING 0 character
static char *os_strcpy(char *dst, const char *src)
{
        ASSERT(dst && src);
        for (int i = 0;; i++) {
                dst[i] = src[i];
                if (src[i] == '\0') break;
        }
        return dst;
}

// copies src to dst EXCLUDING 0 character
static char *os_strncpy(char *dst, const char *src)
{
        ASSERT(dst && src);
        for (int i = 0;; i++) {
                if (src[i] == '\0') break;
                dst[i] = src[i];
        }
        return dst;
}

static char *os_strcat(char *s1, const char *s2)
{
        ASSERT(s1 && s2);
        int i = 0;
        while (s1[i] != '\0') {
                i++;
        }
        for (int k = 0;; k++) {
                s1[i + k] = s2[k];
                if (s2[k] == '\0') break;
        }
        return s1;
}

static char *os_strcat_i32(char *s1, i32 value)
{
        if (!s1) return s1;

        if (value == 0) {
                *s1 = '0';
                return s1;
        }

        char *s = s1;
        while (*s != '\0') {
                s++;
        }

        i32 num = value;
        if (num < 0) {
                num  = -num;
                *s++ = '-';
        }

        for (i32 temp = num; temp > 0; temp /= 10) {
                s++;
        }

        *s = '\0';
        for (i32 temp = num; temp > 0; temp /= 10) {
                *--s = temp % 10 + '0';
        }
        return s1;
}

static i32 os_i32_from_str(const char *s)
{
        int i = 0;
        while (1) {
                char c = s[i];
                if (c == '\0') return 0;
                if (char_matches_any(c, " \n\t"))
                        i++;
                else
                        break;
        }

        i32 res = 0;
        i32 sig = 1;
        switch (s[i]) {
        case '-': sig = -1;
        case '+': i++; break;
        }

        while (1) {
                char c = s[i];
                if (!char_digit(c)) break;
                res *= 10;
                res += (int)(c - '0');
                i++;
        }
        return (res * sig);
}

#endif