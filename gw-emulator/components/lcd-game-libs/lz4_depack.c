
#include <stdlib.h>
#include <string.h>

#include "lz4_depack.h"
/*  original source code of lz4_depack() came from:
https://github.com/jibsen/blz4/blob/master/lz4_depack.c
please read the joined notice. This source is not altered.
*/


/*********************************/
/*
 * blz4 - Example of LZ4 compression with BriefLZ algorithms
 *
 * C depacker
 *
 * Copyright (c) 2018 Joergen Ibsen
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must
 *      not claim that you wrote the original software. If you use this
 *      software in a product, an acknowledgment in the product
 *      documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must
 *      not be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any source
 *      distribution.
 */

unsigned long
lz4_depack(const void *src, void *dst, unsigned long packed_size)
{
	const unsigned char *in = (unsigned char *)src;
	unsigned char *out = (unsigned char *)dst;
	unsigned long dst_size = 0;
	unsigned long cur = 0;
	unsigned long prev_match_start = 0;

	if (in[0] == 0)
	{
		return 0;
	}

	/* Main decompression loop */
	while (cur < packed_size)
	{
		unsigned long token = in[cur++];
		unsigned long lit_len = token >> 4;
		unsigned long len = (token & 0x0F) + 4;
		unsigned long offs;
		unsigned long i;

		/* Read extra literal length bytes */
		if (lit_len == 15)
		{
			while (in[cur] == 255)
			{
				lit_len += 255;
				++cur;
			}
			lit_len += in[cur++];
		}

		/* Copy literals */
		for (i = 0; i < lit_len; ++i)
		{
			out[dst_size++] = in[cur++];
		}

		/* Check for last incomplete sequence */
		if (cur == packed_size)
		{
			/* Check parsing restrictions */
			if (dst_size >= 5 && lit_len < 5)
			{
				return 0;
			}

			if (dst_size > 12 && dst_size - prev_match_start < 12)
			{
				return 0;
			}

			break;
		}

		/* Read offset */
		offs = (unsigned long)in[cur] | ((unsigned long)in[cur + 1] << 8);
		cur += 2;

		/* Read extra length bytes */
		if (len == 19)
		{
			while (in[cur] == 255)
			{
				len += 255;
				++cur;
			}
			len += in[cur++];
		}

		prev_match_start = dst_size;

		/* Copy match */
		for (i = 0; i < len; ++i)
		{
			out[dst_size] = out[dst_size - offs];
			++dst_size;
		}
	}

	/* Return decompressed size */
	return dst_size;
}


/*********************************/

/*
lz4_uncompress() and lz4_get_original_size() are from bzhxx.
free of use, as-is without any warranty. don't use it!
*/

/*
 LZ4 uncompress function
*src 		: pointer on source buffer (LZ4 file format)
*dst 		: pointer on destination buffer
return the size of uncompressed data
return 0 if there is an error
 */
unsigned int
lz4_uncompress(const void *src, void *dst)
{
	const unsigned char *in = (unsigned char *)src;
	unsigned char *out = (unsigned char *)dst;

	unsigned int uncompressed_size = 0;
	unsigned int compressed_size = 0;
	unsigned int original_size = 0;
	unsigned int content_offset = 0;
	unsigned int compressed_size_offset = 0;
	unsigned char flags;

	/* check if it's LZ4  format */
	if (memcmp(&in[0], LZ4_MAGIC, LZ4_MAGIC_SIZE) == 0)
	{

		/* Parse the header to determine :
		- the compressed size
		- the content offset 
		- the original size (if present) */

		/* get the header flags */
		memcpy(&flags, &in[LZ4_FLG_OFFSET], sizeof(flags));

		/* Content size field in header ? */
		if ((flags & LZ4_FLG_MASK_C_SIZE) != 0)
		{
			memcpy(&original_size, &in[LZ4_CONTENT_SIZE_OFFSET], sizeof(original_size));
			compressed_size_offset += LZ4_CONTENT_SIZE;
		}

		/* optional Dict. field in header ? */
		if ((flags & LZ4_FLG_MASK_DICTID) != 0)
		{
			compressed_size_offset += LZ4_DICTID_SIZE;
		}

		/* Add the minimum header size  */
		compressed_size_offset += LZ4_MAGIC_SIZE + LZ4_FLG_SIZE + LZ4_BD_SIZE + LZ4_HC_SIZE;
		content_offset += compressed_size_offset + LZ4_FRAME_SIZE;

		/* get the compressed size */
		memcpy(&compressed_size, &in[compressed_size_offset], sizeof(compressed_size));

		/* Uncompress the content to RAM */
		uncompressed_size = lz4_depack(&in[content_offset], out, compressed_size);

		/* Additional control */
		if ((flags & LZ4_FLG_MASK_C_SIZE) != 0)
		{
			if (uncompressed_size != original_size)
				uncompressed_size = 0;
		}
	}

	return uncompressed_size;
}

/*
LZ4  function to get the uncompressed size from LZ4 header
*src 				: pointer on source buffer (LZ4 file format)
return the the uncompressed size of the content
return 0 if there is an error
 */
unsigned int
lz4_get_original_size(const void *src)
{

	const unsigned char *in = (unsigned char *)src;
	unsigned int original_size = 0;
	unsigned char flags;

	/* check if it's LZ4  format */
	/* get the original size of the content to uncompress from the LZ4 header */
	if (memcmp(&in[0], LZ4_MAGIC, LZ4_MAGIC_SIZE) == 0)
	{

		memcpy(&flags, &in[LZ4_FLG_OFFSET], sizeof(flags));

		if ((flags & LZ4_FLG_MASK_C_SIZE) != 0)
			memcpy(&original_size, &in[LZ4_CONTENT_SIZE_OFFSET], sizeof(original_size));
	}

	return original_size;
}

unsigned int
lz4_get_file_size(const void *src)
{

	const unsigned char *in = (unsigned char *)src;

	unsigned int compressed_size = 0;
	unsigned int original_size = 0;
	unsigned int content_offset = 0;
	unsigned int compressed_size_offset = 0;
	unsigned char flags;
	unsigned int file_size = 0;

	/* check if it's LZ4  format */
	if (memcmp(&in[0], LZ4_MAGIC, LZ4_MAGIC_SIZE) == 0)
	{

		/* Parse the header to determine :
		- the compressed size
		- the content offset 
		- the original size (if present) */

		/* get the header flags */
		memcpy(&flags, &in[LZ4_FLG_OFFSET], sizeof(flags));

		/* Content size field in header ? */
		if ((flags & LZ4_FLG_MASK_C_SIZE) != 0)
		{
			memcpy(&original_size, &in[LZ4_CONTENT_SIZE_OFFSET], sizeof(original_size));
			compressed_size_offset += LZ4_CONTENT_SIZE;
		}

		/* optional Dict. field in header ? */
		if ((flags & LZ4_FLG_MASK_DICTID) != 0)
		{
			compressed_size_offset += LZ4_DICTID_SIZE;
		}

		/* Add the minimum header size  */
		compressed_size_offset += LZ4_MAGIC_SIZE + LZ4_FLG_SIZE + LZ4_BD_SIZE + LZ4_HC_SIZE;
		content_offset += compressed_size_offset + LZ4_FRAME_SIZE;

		/* get the compressed size */
		memcpy(&compressed_size, &in[compressed_size_offset], sizeof(compressed_size));

		file_size = compressed_size + content_offset + LZ4_ENDMARK_SIZE;

		if ((flags & LZ4_FLG_MASK_C_CHECKSUM) != 0) file_size = file_size +  LZ4_CHECKSUM_SIZE;
	}
	
	return file_size;
}