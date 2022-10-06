

#ifndef DEF_LZ4DEPACK
#define DEF_LZ4DEPACK

// https://github.com/lz4/lz4/blob/dev/doc/lz4_Frame_format.md

/* MAGIC WORD found as a magic word in LZ4 payload */
#define LZ4_MAGIC "\x04\x22\x4D\x18" // LE 0x184D2204U

/* LZ4 file format */

/* LZ4 file structure
MagicNb	F. Descriptor	Block	(...)	EndMark	C.Checksum
4 bytes	3-15 bytes			            4 bytes	0-4 bytes

LZ4 F.Descriptor format
FLG	    BD	   (Content Size)	(Dictionary ID)	HC
1 byte	1 byte	0 - 8 bytes	    0 - 4 bytes	    1 byte

FLG byte
BitNb	    7-6	    5	    4	        3	    2	        1	        0
FieldName	Version	B.Indep	B.Checksum	C.Size	C.Checksum	Reserved	DictID

*/

/* header fields size  */
#define LZ4_MAGIC_SIZE (size_t)(4)
#define LZ4_FLG_SIZE 1
#define LZ4_BD_SIZE 1
#define LZ4_CONTENT_SIZE 8
#define LZ4_DICTID_SIZE 4
#define LZ4_HC_SIZE 1

/* footer fields size */
#define LZ4_ENDMARK_SIZE 4
#define LZ4_CHECKSUM_SIZE 4

/* frame size */
#define LZ4_FRAME_SIZE 4

/* FLG bit masks */
#define LZ4_FLG_MASK_DICTID 0x1
#define LZ4_FLG_MASK_C_CHECKSUM 0x4
#define LZ4_FLG_MASK_C_SIZE 0x8
#define LZ4_FLG_MASK_B_CHECKSUM 0x10

/* FLG field offset */
#define LZ4_FLG_OFFSET (LZ4_MAGIC_SIZE)

/* Content size offset if FLG C.Size */
#define LZ4_CONTENT_SIZE_OFFSET (LZ4_MAGIC_SIZE + LZ4_FLG_SIZE + LZ4_BD_SIZE)

/* LZ4 uncompress function
*src 		: pointer on source buffer (LZ4 file format)
*dst 		: pointer on destination buffer
src_size 	: size of source buffer
 */
unsigned int lz4_uncompress(const void *src, void *dst);

/* LZ4  function to get the uncompressed size from LZ4 header
*src 				: pointer on source buffer (LZ4 file format)
packect_size 	: size of the compressed packet to unpack
return true if success
 */
unsigned int lz4_get_original_size(const void *src);

/* LZ4 depack function
*src 				: pointer on source buffer (LZ4 file format)
return the size of the original (uncompressed) content
return 0 if it's not a LZ4 file format by checking LZ4_MAGIC NUMBER. 
 */
unsigned long lz4_depack(const void *src, void *dst, unsigned long packed_size);

unsigned int lz4_get_file_size(const void *src);

#endif /* DEF_LZ4DEPACK */