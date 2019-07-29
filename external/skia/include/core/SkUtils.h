/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUtils_DEFINED
#define SkUtils_DEFINED

#include "SkTypes.h"

///////////////////////////////////////////////////////////////////////////////

// Determined empirically using bench/MemsetBench.cpp on a Nexus 7, Nexus 9, and desktop.
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2 || defined(SK_ARM_HAS_NEON)
    // Platforms where we can assume an autovectorizer will give us a good inline memset.
    #define SK_SMALL_MEMSET 1000
#else
    // Platforms like Chrome on ARMv7 that don't typically compile with NEON globally.
    #define SK_SMALL_MEMSET 10
#endif


/** Similar to memset(), but it assigns a 16bit value into the buffer.
    @param buffer   The memory to have value copied into it
    @param value    The 16bit value to be copied into buffer
    @param count    The number of times value should be copied into the buffer.
*/
void sk_memset16_large(uint16_t dst[], uint16_t value, int count);
inline void sk_memset16(uint16_t dst[], uint16_t value, int count) {
    if (count <= SK_SMALL_MEMSET) {
        for (int i = 0; i < count; i++) {
            dst[i] = value;
        }
    } else {
        sk_memset16_large(dst, value, count);
    }
}
typedef void (*SkMemset16Proc)(uint16_t dst[], uint16_t value, int count);
SkMemset16Proc SkMemset16GetPlatformProc();

/** Similar to memset(), but it assigns a 32bit value into the buffer.
    @param buffer   The memory to have value copied into it
    @param value    The 32bit value to be copied into buffer
    @param count    The number of times value should be copied into the buffer.
*/
void sk_memset32_large(uint32_t dst[], uint32_t value, int count);
inline void sk_memset32(uint32_t dst[], uint32_t value, int count) {
    if (count <= SK_SMALL_MEMSET) {
        for (int i = 0; i < count; i++) {
            dst[i] = value;
        }
    } else {
        sk_memset32_large(dst, value, count);
    }
}

typedef void (*SkMemset32Proc)(uint32_t dst[], uint32_t value, int count);
SkMemset32Proc SkMemset32GetPlatformProc();

#undef SK_SMALL_MEMSET

/** Similar to memcpy(), but it copies count 32bit values from src to dst.
    @param dst      The memory to have value copied into it
    @param src      The memory to have value copied from it
    @param count    The number of values should be copied.
*/
void sk_memcpy32(uint32_t dst[], const uint32_t src[], int count);
typedef void (*SkMemcpy32Proc)(uint32_t dst[], const uint32_t src[], int count);
SkMemcpy32Proc SkMemcpy32GetPlatformProc();

///////////////////////////////////////////////////////////////////////////////

#define kMaxBytesInUTF8Sequence     4

#ifdef SK_DEBUG
    int SkUTF8_LeadByteToCount(unsigned c);
#else
    #define SkUTF8_LeadByteToCount(c)   ((((0xE5 << 24) >> ((unsigned)c >> 4 << 1)) & 3) + 1)
#endif

inline int SkUTF8_CountUTF8Bytes(const char utf8[]) {
    SkASSERT(utf8);
    return SkUTF8_LeadByteToCount(*(const uint8_t*)utf8);
}

int         SkUTF8_CountUnichars(const char utf8[]);
int         SkUTF8_CountUnichars(const char utf8[], size_t byteLength);
SkUnichar   SkUTF8_ToUnichar(const char utf8[]);
SkUnichar   SkUTF8_NextUnichar(const char**);
SkUnichar   SkUTF8_PrevUnichar(const char**);

/** Return the number of bytes need to convert a unichar
    into a utf8 sequence. Will be 1..kMaxBytesInUTF8Sequence,
    or 0 if uni is illegal.
*/
size_t      SkUTF8_FromUnichar(SkUnichar uni, char utf8[] = NULL);

///////////////////////////////////////////////////////////////////////////////

#define SkUTF16_IsHighSurrogate(c)  (((c) & 0xFC00) == 0xD800)
#define SkUTF16_IsLowSurrogate(c)   (((c) & 0xFC00) == 0xDC00)

int SkUTF16_CountUnichars(const uint16_t utf16[]);
int SkUTF16_CountUnichars(const uint16_t utf16[], int numberOf16BitValues);
// returns the current unichar and then moves past it (*p++)
SkUnichar SkUTF16_NextUnichar(const uint16_t**);
// this guy backs up to the previus unichar value, and returns it (*--p)
SkUnichar SkUTF16_PrevUnichar(const uint16_t**);
size_t SkUTF16_FromUnichar(SkUnichar uni, uint16_t utf16[] = NULL);

size_t SkUTF16_ToUTF8(const uint16_t utf16[], int numberOf16BitValues,
                      char utf8[] = NULL);

inline bool SkUnichar_IsVariationSelector(SkUnichar uni) {
/*  The 'true' ranges are:
 *      0x180B  <= uni <=  0x180D
 *      0xFE00  <= uni <=  0xFE0F
 *      0xE0100 <= uni <= 0xE01EF
 */
    if (uni < 0x180B || uni > 0xE01EF) {
        return false;
    }
    if ((uni > 0x180D && uni < 0xFE00) || (uni > 0xFE0F && uni < 0xE0100)) {
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

class SkAutoTrace {
public:
    /** NOTE: label contents are not copied, just the ptr is
        retained, so DON'T DELETE IT.
    */
    SkAutoTrace(const char label[]) : fLabel(label) {
        SkDebugf("--- trace: %s Enter\n", fLabel);
    }
    ~SkAutoTrace() {
        SkDebugf("--- trace: %s Leave\n", fLabel);
    }
private:
    const char* fLabel;
};
#define SkAutoTrace(...) SK_REQUIRE_LOCAL_VAR(SkAutoTrace)

#endif
