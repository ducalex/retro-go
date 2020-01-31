#include "gnuboy.h"

/* Slightly modified from its original form so as not to exit the
 * program on errors. The resulting file remains in the public
 * domain for all to use. */

/* --- GZIP file format uncompression routines --- */

/* The following routines (notably the unzip()) function below
 * uncompress gzipped data.  They are terribly slow at the task, but
 * it is presumed that they work reasonably well.  They don't do any
 * error checking, but they're probably not too vulnerable to buggy
 * data either.  Another important limitation (but it would be pretty
 * easy to get around) is that the data must reside in memory, it is
 * not read as a stream.  They have been very little tested.  Anyway,
 * whatever these functions are good for, I put them in the public
 * domain.  -- David Madore <david.madore@ens.fr> 1999/11/21 */

static unsigned int
peek_bits (const unsigned char *data, long p, int q)
     /* Read q bits starting from bit p from the data pointed to by
      * data.  Data is in little-endian format. */
{
  unsigned int answer;
  int cnt; /* Number of bits already placed in answer */
  char ob, lb; /* Offset and length of bit field within current byte */

  answer = 0;
  for ( cnt=0 ; cnt<q ; /* cnt updated in body */ )
    {
      ob = (p+cnt)%8;
      lb = 8-ob;
      if ( cnt+lb > q )
	lb = q-cnt;
      answer |= ((unsigned int)((data[(p+cnt)/8]>>ob)&((1U<<lb)-1)))<<cnt;
      cnt += lb;
    }
  return answer;
}

static unsigned int
read_bits (const unsigned char *data, long *p, int q)
     /* Read q bits as per peek_bits(), but also increase p by q. */
{
  unsigned int answer;

  answer = peek_bits (data, *p, q);
  *p += q;
  return answer;
}

static void
make_code_table (const char size_table[], int table_length,
		 unsigned int code_table[], int maxbits)
     /* Make a code table from a length table.  See rfc1951, section
      * 3.2.2, for details on what this means.  The size_table
      * contains the length of the Huffman codes for each letter, and
      * the code_table receives the computed codes themselves.
      * table_length is the size of the tables (alphabet length) and
      * maxbits is the maximal allowed code length. */
{
  int i, j;
  unsigned int code;

  code = 0;
  for ( i=1 ; i<=maxbits ; i++ )
    {
      for ( j=0 ; j<table_length ; j++ )
	{
	  if ( size_table[j]==i )
	    code_table[j] = code++;
	}
      code <<= 1;
    }
}

static int
decode_one (const unsigned char *data, long *p,
	    const char size_table[], int table_length,
	    const unsigned int code_table[], int maxbits)
     /* Decode one alphabet letter from the data, starting at bit p
      * (which will be increased by the appropriate amount) using
      * size_table and code_table to decipher the Huffman encoding. */
{
  unsigned int code;
  int i, j;

  code = 0;
  /* Read as many bits as are likely to be necessary - backward, of
   * course. */
  for ( i=0 ; i<maxbits ; i++ )
    code = (code<<1) + peek_bits (data, (*p)+i, 1);
  /* Now examine each symbol of the table to find one that matches the
   * first bits of the code read. */
  for ( j=0 ; j<table_length ; j++ )
    {
      if ( size_table[j]
	   && ( (code>>(maxbits-size_table[j])) == code_table[j] ) )
	{
	  *p += size_table[j];
	  return j;
	}
    }
  return -1;
}

/* I don't know what these should be.  The rfc1951 doesn't seem to say
 * (it only mentions them in the last paragraph of section 3.2.1).  15
 * is almost certainly safe, and it is the largest I can put given the
 * constraints on the size of integers in the C standard. */
#define CLEN_MAXBITS 15
#define HLIT_MAXBITS 15
#define HDIST_MAXBITS 15

/* The magical table sizes... */
#define CLEN_TSIZE 19
#define HLIT_TSIZE 288
#define HDIST_TSIZE 30

static int
get_tables (const unsigned char *data, long *p,
	    char hlit_size_table[HLIT_TSIZE],
	    unsigned int hlit_code_table[HLIT_TSIZE],
	    char hdist_size_table[HDIST_TSIZE],
	    unsigned int hdist_code_table[HDIST_TSIZE])
     /* Fill the Huffman tables (first the code lengths table, and
      * then, using it, the literal/length table and the distance
      * table).  See section 3.2.7 of rfc1951 for details. */
{
  char hlit, hdist, hclen;
  const int clen_weird_tangle[CLEN_TSIZE]
    = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
  char clen_size_table[CLEN_TSIZE];
  unsigned int clen_code_table[CLEN_TSIZE];
  int j;
  unsigned int b;
  int remainder; /* See note at end of section 3.2.7 of rfc1951. */
  char rem_val;

  hlit = read_bits (data, p, 5);
  hdist = read_bits (data, p, 5);
  hclen = read_bits (data, p, 4);
  for ( j=0 ; j<4+hclen ; j++ )
    clen_size_table[clen_weird_tangle[j]]
      = read_bits (data, p, 3);
  for ( ; j<CLEN_TSIZE ; j++ )
    clen_size_table[clen_weird_tangle[j]] = 0;
  make_code_table (clen_size_table, CLEN_TSIZE,
		   clen_code_table, CLEN_MAXBITS);
  remainder = 0;
  rem_val = 0;
  for ( j=0 ; j<257+hlit ; j++ )
    {
      b = decode_one (data, p, clen_size_table, CLEN_TSIZE,
		      clen_code_table, CLEN_MAXBITS);
      if ( b<0 ) return -1;
      if ( b<16 )
	hlit_size_table[j] = b;
      else if ( b == 16 )
	{
	  int k, l;

	  k = read_bits (data, p, 2);
	  for ( l=0 ; l<k+3 && j+l<257+hlit ; l++ )
	    hlit_size_table[j+l] = hlit_size_table[j-1];
	  j += l-1;
	  remainder = k+3-l; /* THIS IS SO UGLY! */
	  rem_val = hlit_size_table[j-1];
	}
      else if ( b == 17 )
	{
	  int k, l;

	  k = read_bits (data, p, 3);
	  for ( l=0 ; l<k+3 && j+l<257+hlit ; l++ )
	    hlit_size_table[j+l] = 0;
	  j += l-1;
	  remainder = k+3-l;
	  rem_val = 0;
	}
      else if ( b == 18 )
	{
	  int k, l;

	  k = read_bits (data, p, 7);
	  for ( l=0 ; l<k+11 && j+l<257+hlit ; l++ )
	    hlit_size_table[j+l] = 0;
	  j += l-1;
	  remainder = k+11-l;
	  rem_val = 0;
	}
    }
  for ( ; j<HLIT_TSIZE ; j++ )
    hlit_size_table[j] = 0;
  make_code_table (hlit_size_table, HLIT_TSIZE,
		   hlit_code_table, HLIT_MAXBITS);
  for ( j=0 ; j<remainder ; j++ )
    hdist_size_table[j] = rem_val;
  for ( ; j<1+hdist ; j++ )
    /* Can you spell: ``copy-paste''? */
    {
      b = decode_one (data, p, clen_size_table, CLEN_TSIZE,
		      clen_code_table, CLEN_MAXBITS);
      if ( b<0 ) return -1;
      if ( b<16 )
	hdist_size_table[j] = b;
      else if ( b == 16 )
	{
	  int k, l;

	  k = read_bits (data, p, 2);
	  for ( l=0 ; l<k+3 && j+l<1+hdist ; l++ )
	    hdist_size_table[j+l] = hdist_size_table[j-1];
	  j += l-1;
	}
      else if ( b == 17 )
	{
	  int k, l;

	  k = read_bits (data, p, 3);
	  for ( l=0 ; l<k+3 && j+l<1+hdist ; l++ )
	    hdist_size_table[j+l] = 0;
	  j += l-1;
	}
      else if ( b == 18 )
	{
	  int k, l;

	  k = read_bits (data, p, 7);
	  for ( l=0 ; l<k+11 && j+l<1+hdist ; l++ )
	    hdist_size_table[j+l] = 0;
	  j += l-1;
	}
    }
  for ( ; j<HDIST_TSIZE ; j++ )
    hdist_size_table[j] = 0;
  make_code_table (hdist_size_table, HDIST_TSIZE,
		   hdist_code_table, HDIST_MAXBITS);
  return 0;
}

/* The (circular) output buffer.  This lets us track
 * backreferences. */

/* Minimal buffer size.  Also the only useful value. */
#define BUFFER_SIZE 32768

/* Pointer to the character to be added to the buffer */
static unsigned int buffer_ptr = 0;

/* The buffer itself */
static unsigned char buffer[BUFFER_SIZE];

static void
pushout (unsigned char ch)
     /* Store one byte in the output buffer so it may be retrieved if
      * it is referenced again. */
{
  buffer[buffer_ptr++] = ch;
  buffer_ptr %= BUFFER_SIZE;
}

static unsigned char
pushin (unsigned int dist)
     /* Retrieve one byte, dist bytes away, from the output buffer. */
{
  return buffer[(buffer_ptr+(BUFFER_SIZE-dist))%BUFFER_SIZE];
}

static int
get_data (const unsigned char *data, long *p,
	  const char hlit_size_table[HLIT_TSIZE],
	  const unsigned int hlit_code_table[HLIT_TSIZE],
	  const char hdist_size_table[HDIST_TSIZE],
	  const unsigned int hdist_code_table[HDIST_TSIZE],
	  void (* callback) (unsigned char d))
     /* Do the actual uncompressing.  Call callback on each character
      * uncompressed. */
{
  unsigned int b;

  while ( 1 ) {
    b = decode_one (data, p, hlit_size_table, HLIT_TSIZE,
		    hlit_code_table, HLIT_MAXBITS);
    if ( b<0 ) return -1;
    if ( b < 256 )
      /* Literal */
      {
	pushout ((unsigned char) b);
	callback ((unsigned char) b);
      }
    else if ( b == 256 )
      /* End of block */
      return 0;
    else if ( b >= 257 )
      /* Back reference */
      {
	unsigned int bb;
	unsigned int length, dist;
	unsigned int l;

	switch ( b )
	  {
	  case 257: length = 3; break;
	  case 258: length = 4; break;
	  case 259: length = 5; break;
	  case 260: length = 6; break;
	  case 261: length = 7; break;
	  case 262: length = 8; break;
	  case 263: length = 9; break;
	  case 264: length = 10; break;
	  case 265: length = 11 + read_bits (data, p, 1); break;
	  case 266: length = 13 + read_bits (data, p, 1); break;
	  case 267: length = 15 + read_bits (data, p, 1); break;
	  case 268: length = 17 + read_bits (data, p, 1); break;
	  case 269: length = 19 + read_bits (data, p, 2); break;
	  case 270: length = 23 + read_bits (data, p, 2); break;
	  case 271: length = 27 + read_bits (data, p, 2); break;
	  case 272: length = 31 + read_bits (data, p, 2); break;
	  case 273: length = 35 + read_bits (data, p, 3); break;
	  case 274: length = 43 + read_bits (data, p, 3); break;
	  case 275: length = 51 + read_bits (data, p, 3); break;
	  case 276: length = 59 + read_bits (data, p, 3); break;
	  case 277: length = 67 + read_bits (data, p, 4); break;
	  case 278: length = 83 + read_bits (data, p, 4); break;
	  case 279: length = 99 + read_bits (data, p, 4); break;
	  case 280: length = 115 + read_bits (data, p, 4); break;
	  case 281: length = 131 + read_bits (data, p, 5); break;
	  case 282: length = 163 + read_bits (data, p, 5); break;
	  case 283: length = 195 + read_bits (data, p, 5); break;
	  case 284: length = 227 + read_bits (data, p, 5); break;
	  case 285: length = 258; break;
	  default:
	    return -1;
	  }
	bb = decode_one (data, p, hdist_size_table, HDIST_TSIZE,
			 hdist_code_table, HDIST_MAXBITS);
	switch ( bb )
	  {
	  case 0: dist = 1; break;
	  case 1: dist = 2; break;
	  case 2: dist = 3; break;
	  case 3: dist = 4; break;
	  case 4: dist = 5 + read_bits (data, p, 1); break;
	  case 5: dist = 7 + read_bits (data, p, 1); break;
	  case 6: dist = 9 + read_bits (data, p, 2); break;
	  case 7: dist = 13 + read_bits (data, p, 2); break;
	  case 8: dist = 17 + read_bits (data, p, 3); break;
	  case 9: dist = 25 + read_bits (data, p, 3); break;
	  case 10: dist = 33 + read_bits (data, p, 4); break;
	  case 11: dist = 49 + read_bits (data, p, 4); break;
	  case 12: dist = 65 + read_bits (data, p, 5); break;
	  case 13: dist = 97 + read_bits (data, p, 5); break;
	  case 14: dist = 129 + read_bits (data, p, 6); break;
	  case 15: dist = 193 + read_bits (data, p, 6); break;
	  case 16: dist = 257 + read_bits (data, p, 7); break;
	  case 17: dist = 385 + read_bits (data, p, 7); break;
	  case 18: dist = 513 + read_bits (data, p, 8); break;
	  case 19: dist = 769 + read_bits (data, p, 8); break;
	  case 20: dist = 1025 + read_bits (data, p, 9); break;
	  case 21: dist = 1537 + read_bits (data, p, 9); break;
	  case 22: dist = 2049 + read_bits (data, p, 10); break;
	  case 23: dist = 3073 + read_bits (data, p, 10); break;
	  case 24: dist = 4097 + read_bits (data, p, 11); break;
	  case 25: dist = 6145 + read_bits (data, p, 11); break;
	  case 26: dist = 8193 + read_bits (data, p, 12); break;
	  case 27: dist = 12289 + read_bits (data, p, 12); break;
	  case 28: dist = 16385 + read_bits (data, p, 13); break;
	  case 29: dist = 24577 + read_bits (data, p, 13); break;
	  default:
            return -1;
	  }
	for ( l=0 ; l<length ; l++ )
	  {
	    unsigned char ch;

	    ch = pushin (dist);
	    pushout (ch);
	    callback (ch);
	  }
      }
  }
  return 0;
}

static int
inflate (const unsigned char *data, long *p,
	 void (* callback) (unsigned char d))
     /* Main uncompression function for the deflate method */
{
  char blast, btype;
  char hlit_size_table[HLIT_TSIZE];
  unsigned int hlit_code_table[HLIT_TSIZE];
  char hdist_size_table[HDIST_TSIZE];
  unsigned int hdist_code_table[HDIST_TSIZE];

 again:
  blast = read_bits (data, p, 1);
  btype = read_bits (data, p, 2);
  if ( btype == 1 || btype == 2 )
    {
      if ( btype == 2 )
        {
	  /* Dynamic Huffman tables */
	  if (get_tables (data, p,
		          hlit_size_table, hlit_code_table,
		          hdist_size_table, hdist_code_table) < 0) return -1;
	}
      else
	/* Fixed Huffman codes */
	{
	  int j;

	  for ( j=0 ; j<144 ; j++ )
	    hlit_size_table[j] = 8;
	  for ( ; j<256 ; j++ )
	    hlit_size_table[j] = 9;
	  for ( ; j<280 ; j++ )
	    hlit_size_table[j] = 7;
	  for ( ; j<HLIT_TSIZE ; j++ )
	    hlit_size_table[j] = 8;
	  make_code_table (hlit_size_table, HLIT_TSIZE,
			   hlit_code_table, HLIT_MAXBITS);
	  for ( j=0 ; j<HDIST_TSIZE ; j++ )
	    hdist_size_table[j] = 5;
	  make_code_table (hdist_size_table, HDIST_TSIZE,
			   hdist_code_table, HDIST_MAXBITS);
	}
      if (get_data (data, p,
		    hlit_size_table, hlit_code_table,
		    hdist_size_table, hdist_code_table,
		    callback) < 0) return -1;;
    }
  else if ( btype == 0 )
    /* Non compressed block */
    {
      unsigned int len, nlen;
      unsigned int l;
      unsigned char b;

      *p = (*p+7)/8; /* Jump to next byte boundary */
      len = read_bits (data, p, 16);
      nlen = read_bits (data, p, 16);
      for ( l=0 ; l<len ; l++ )
	{
	  b = read_bits (data, p, 8);
	  pushout (b);
	  callback (b);
	}
    }
  else
    {
      return -1;
    }
  if ( ! blast )
    goto again;
  return 0;
}

int
unzip (const unsigned char *data, long *p,
       void (* callback) (unsigned char d))
     /* Uncompress gzipped data.  data is a pointer to the data, p is
      * a pointer to a long that is initialized to 0 (unless for some
      * reason you want to start uncompressing further down the data),
      * and callback is a function taking an unsigned char and
      * returning void that will be called successively for every
      * uncompressed byte. */
{
  unsigned char cm, flg;

  if ( read_bits (data, p, 8) != 0x1f
       || read_bits (data, p, 8) != 0x8b )
    {
      return -1;
    }
  cm = read_bits (data, p, 8);
  if ( cm != 0x8 )
    {
      return -1;
    }
  flg = read_bits (data, p, 8);
  if ( flg & 0xe0 )
    /* fprintf (stderr, "Warning: unknown bits are set in flags.\n") */ ;
  read_bits (data, p, 32); /* Ignore modification time */
  read_bits (data, p, 8); /* Ignore extra flags */
  read_bits (data, p, 8); /* Ignore OS type */
  if ( flg & 0x4 )
    {
      /* Skip over extra data */
      unsigned int xlen;

      xlen = read_bits (data, p, 16);
      *p += ((long)xlen)*8;
    }
  if ( flg & 0x8 )
    {
      /* Skip over file name */
      while ( read_bits (data, p, 8) );
    }
  if ( flg & 0x10 )
    {
      /* Skip over comment */
      while ( read_bits (data, p, 8) );
    }
  if ( flg & 0x2 )
    /* Ignore CRC16 */
    read_bits (data, p, 16);
  return inflate (data, p, callback);
  /* CRC32 and ISIZE are at the end.  We don't even bother to look at
   * them. */
}

