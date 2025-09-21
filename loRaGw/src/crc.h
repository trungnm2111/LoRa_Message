/** @file crc.h
*
* @brief Compact CRC library for embedded systems for CRC-CCITT, CRC-16, CRC-32.
*
* @par
* COPYRIGHT NOTICE: (c) 2000, 2018 Michael Barr. This software is placed in the
* public domain and may be used for any purpose. However, this notice must not
* be changed or removed. No warranty is expressed or implied by the publication
* or distribution of this source code.
*/

#ifndef CRC_H
#define CRC_H

// Compile-time selection of the desired CRC algorithm.
//

#include <stdint.h>
#define CRC_32 // Chọn chuẩn CRC_32

#if defined(CRC_CCITT)

#define CRC_NAME "CRC-CCITT"
typedef uint16_t crc_t;

#elif defined(CRC_16)

#define CRC_NAME "CRC-16"
typedef uint16_t crc_t;

#elif defined(CRC_32)

#define CRC_NAME "CRC-32"
typedef uint32_t crc_t;

#else
	
#error "One of CRC_CCITT, CRC_16, or CRC_32 must be #define'd."

#endif


// Algorithmic parameters based on CRC elections made in crc.h.
//
#define BITS_PER_BYTE 8
#define WIDTH (BITS_PER_BYTE * sizeof(crc_t))
#define TOPBIT (1 << (WIDTH - 1))

// Allocate storage for the byte-wide CRC lookup table.

#define CRC_TABLE_SIZE 256
extern crc_t g_crc_table[CRC_TABLE_SIZE];

// Further algorithmic configuration to support the selected CRC standard.
//
#if defined(CRC_CCITT)

#define POLYNOMIAL ((crc_t) 0x1021)
#define INITIAL_REMAINDER ((crc_t) 0xFFFF)
#define FINAL_XOR_VALUE ((crc_t) 0x0000)
#define REFLECT_DATA(X) (X)
#define REFLECT_REMAINDER(X) (X)

#elif defined(CRC_16)

#define POLYNOMIAL ((crc_t) 0x8005)
#define INITIAL_REMAINDER ((crc_t) 0x0000)
#define FINAL_XOR_VALUE ((crc_t) 0x0000)
#define REFLECT_DATA(X) ((uint8_t) reflect((X), BITS_PER_BYTE))
#define REFLECT_REMAINDER(X) ((crc_t) reflect((X), WIDTH))

#elif defined(CRC_32)

#define POLYNOMIAL ((crc_t) 0x04C11DB7)
#define INITIAL_REMAINDER ((crc_t) 0xFFFFFFFF)
#define FINAL_XOR_VALUE ((crc_t) 0xFFFFFFFF)
#define REFLECT_DATA(X) ((uint8_t) reflect((X), BITS_PER_BYTE))
#define REFLECT_REMAINDER(X) ((crc_t) reflect((X), WIDTH))

#endif

// Public API functions provided by the Compact CRC library.
//

void crc_init(void);
crc_t crc_slow(uint8_t const * const p_message, int n_bytes);
crc_t crc_fast(uint8_t const * const p_message, int n_bytes);
uint32_t reflect (uint32_t data, uint8_t n_bits);

#endif // CRC_H

/*** end of file ***/