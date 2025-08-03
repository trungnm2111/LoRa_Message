/** @file crc.c
*
* @brief Compact CRC generator for embedded systems, with brute force and table-
* driven algorithm options. Supports CRC-CCITT, CRC-16, and CRC-32 standards.
*
* @par
* COPYRIGHT NOTICE: (c) 2000, 2018 Michael Barr. This software is placed in the
* public domain and may be used for any purpose. However, this notice must not
* be changed or removed. No warranty is expressed or implied by the publication
* or distribution of this source code.
*/


#include "crc.h"

crc_t g_crc_table[CRC_TABLE_SIZE];

/*!
* @brief Compute the reflection of a set of data bits around its center.
* @param[in] data The data bits to be reflected.
* @param[in] num2 The number of bits.
* @return The reflected data.
*/
uint32_t reflect (uint32_t data, uint8_t n_bits)
{
	uint32_t reflection = 0x00000000;
	
	// NOTE: For efficiency sake, n_bits is not verified to be <= 32.
	
	// Reflect the data about the center bit.
	//
	for (uint8_t bit = 0; bit < n_bits; ++bit)
	{
		// If the LSB bit is set, set the reflection of it.
		//
		if (data & 0x01)
		{
			reflection |= (1 << ((n_bits - 1) - bit));
		}
		
		data = (data >> 1);
	}
	
	return (reflection);
} /* reflect() */

/*!
* @brief Initialize the lookup table for byte-by-byte CRC acceleration.
*
* @par
* This function must be run before crc_fast() or the table stored in ROM.
*/
void crc_init (void)
{
	// Compute the remainder of each possible dividend.
	//
	for (crc_t dividend = 0; dividend < CRC_TABLE_SIZE; ++dividend)
	{
		// Start with the dividend followed by zeros.
		//
		crc_t remainder = dividend << (WIDTH - BITS_PER_BYTE);
		
		// Perform modulo-2 division, a bit at a time.
		//
		for (int bit = BITS_PER_BYTE; bit > 0; --bit)
		{
			// Try to divide the current data bit.
			//
			if (remainder & TOPBIT)
			{
				remainder = (remainder << 1) ^ POLYNOMIAL;
			}
			else
			{
				remainder = (remainder << 1);
			}
		}
		
		// Store the result into the table.
		//
		g_crc_table[dividend] = remainder;
	}
	
} /* crc_init() */

/*!
* @brief Compute the CRC of an array of bytes, bit-by-bit.
* @param[in] p_message A pointer to the array of data bytes to be CRC'd.
* @param[in] n_bytes The number of bytes in the array of data.
* @return The CRC of the array of data.
*/

crc_t crc_slow (uint8_t const * const p_message, int n_bytes)
{
	crc_t remainder = INITIAL_REMAINDER;
	
	// Perform modulo-2 division, one byte at a time.
	//
	for (int byte = 0; byte < n_bytes; ++byte)
	{
		// Bring the next byte into the remainder.
		//
		remainder ^= (REFLECT_DATA(p_message[byte]) << (WIDTH - BITS_PER_BYTE));
		
		// Perform modulo-2 division, one bit at a time.
		//
		for (int bit = BITS_PER_BYTE; bit > 0; --bit)
		{
			// Try to divide the current data bit.
			//
			if (remainder & TOPBIT)
			{
				remainder = (remainder << 1) ^ POLYNOMIAL;
			}
			else
			{
				remainder = (remainder << 1);
			}
		}
	}
	
	// The final remainder is the CRC result.
	//
	return (REFLECT_REMAINDER(remainder) ^ FINAL_XOR_VALUE);
} /* crc_slow() */

/*!
* @brief Compute the CRC of an array of bytes, byte-by-byte.
* @param[in] p_message A pointer to the array of data bytes to be CRC'd.
* @param[in] n_bytes The number of bytes in the array of data.
* @return The CRC of the array of data.
*/
crc_t crc_fast (uint8_t const * const p_message, int n_bytes)
{
	crc_t remainder = INITIAL_REMAINDER;
	
	// Divide the message by the polynomial, a byte at a time.
	//
	for (int byte = 0; byte < n_bytes; ++byte)
	{
		uint8_t data = REFLECT_DATA(p_message[byte]) ^		\
						(remainder >> (WIDTH - BITS_PER_BYTE));
		remainder = g_crc_table[data] ^ (remainder << BITS_PER_BYTE);
	}
	
	// The final remainder is the CRC.
	//
	return (REFLECT_REMAINDER(remainder) ^ FINAL_XOR_VALUE);
} /* crc_fast() */

/*** end of file ***/