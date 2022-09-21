// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
// This file has been modified from Ken Silverman's original release

/* DDOI - This file is an attempt to reimplement a_nasm.asm in C */
/* FCS: However did that work: This is far from perfect but you have my eternal respect !!! */

#include "platform.h"
#include "build.h"
#include "draw.h"

int32_t pixelsAllowed = 10000000000;

uint8_t  *transluc = NULL;

static int transrev = 0;


#define shrd(a,b,c) (((b)<<(32-(c))) | ((a)>>(c)))
#define shld(a,b,c) (((b)>>(32-(c))) | ((a)<<(c)))















/* ---------------  WALLS RENDERING METHOD (USED TO BE HIGHLY OPTIMIZED ASSEMBLY) ----------------------------*/
extern int32_t asm1;
extern intptr_t asm2;
extern uint8_t *asm3;
extern int32_t asm4;

static uint8_t machxbits_al;
static uint8_t bitsSetup;
static uint8_t * textureSetup;
void sethlinesizes(int32_t i1, int32_t _bits, uint8_t * textureAddress)
{
    machxbits_al = i1;
    bitsSetup = _bits;
    textureSetup = textureAddress;
} 



//FCS:   Draw ceiling/floors
//Draw a line from destination in the framebuffer to framebuffer-numPixels
IRAM_ATTR void hlineasm4(int32_t numPixels, int32_t shade, uint32_t i4, uint32_t i5, uint8_t *dest){

    int32_t shifter = ((256-machxbits_al) & 0x1f);
    uint32_t source;
    
    uint8_t * texture = textureSetup;
    uint8_t bits = bitsSetup;
    
    shade = shade & 0xffffff00;
    numPixels++;
    
	if (!RENDER_DRAW_CEILING_AND_FLOOR)
		return;

    while (numPixels) {

	    source = i5 >> shifter;
	    source = shld(source,i4,bits);
	    source = texture[source];
        
		if (pixelsAllowed-- > 0)
			*dest = globalpalwritten[shade|source];
        
	    dest--;
        
	    i5 -= asm1;
	    i4 -= asm2;
        
	    numPixels--;
		
    }
}

static int32_t rmach_eax;
static int32_t rmach_ebx;
static int32_t rmach_ecx;
static int32_t rmach_edx;
static int32_t rmach_esi;
void setuprhlineasm4(int32_t i1, int32_t i2, int32_t i3, int32_t i4, int32_t i5, int32_t i6)
{
    rmach_eax = i1;
    rmach_ebx = i2;
    rmach_ecx = i3;
    rmach_edx = i4;
    rmach_esi = i5;
} 


IRAM_ATTR void rhlineasm4(int32_t i1, uint8_t* texture, int32_t i3, uint32_t i4, uint32_t i5, int32_t dest)
{
    uint32_t ebp = dest - i1;
    uint32_t rmach6b = ebp-1;
    int32_t numPixels;
    
    if (i1 <= 0) return;

    numPixels = i1;
    do {
		
		

	    i3 = ((i3&0xffffff00)|(*texture));
	    i4 -= rmach_eax;
	    ebp = (((i4+rmach_eax) < i4) ? -1 : 0);
	    i5 -= rmach_ebx;
        
	    if ((i5 + rmach_ebx) < i5)
            texture -= (rmach_ecx+1);
	    else
            texture -= rmach_ecx;
        
	    ebp &= rmach_esi;
	    i1 = ((i1&0xffffff00)|(((uint8_t *)i3)[rmach_edx]));

		if (pixelsAllowed-- > 0)
			 ((uint8_t *)rmach6b)[numPixels] = (i1&0xff);

	    texture -= ebp;
	    numPixels--;
    } while (numPixels);
}

static int32_t rmmach_eax;
static int32_t rmmach_ebx;
static int32_t rmmach_ecx;
static int32_t rmmach_edx;
static int32_t setupTileHeight;
void setuprmhlineasm4(int32_t i1, int32_t i2, int32_t i3, int32_t i4, int32_t tileHeight, int32_t i6)
{
    rmmach_eax = i1;
    rmmach_ebx = i2;
    rmmach_ecx = i3;
    rmmach_edx = i4;
    setupTileHeight = tileHeight;
} 


//FCS: ????
IRAM_ATTR void rmhlineasm4(int32_t i1, intptr_t shade, int32_t colorIndex, int32_t i4, int32_t i5, int32_t dest)
{
    uint32_t ebp = dest - i1;
    uint32_t rmach6b = ebp-1;
    int32_t numPixels;
    
    if (i1 <= 0)
        return;

    numPixels = i1;
    do {

	

	    colorIndex = ((colorIndex&0xffffff00)|(*((uint8_t *)shade)));
	    i4 -= rmmach_eax;
	    ebp = (((i4+rmmach_eax) < i4) ? -1 : 0);
	    i5 -= rmmach_ebx;
        
	    if ((i5 + rmmach_ebx) < i5)
            shade -= (rmmach_ecx+1);
	    else
            shade -= rmmach_ecx;
        
	    ebp &= setupTileHeight;
        
        //Check if this colorIndex is the transparent color (255).
	    if ((colorIndex&0xff) != 255) {
			if (pixelsAllowed-- > 0)
			{
				i1 = ((i1&0xffffff00)|(((uint8_t  *)colorIndex)[rmmach_edx]));
				((uint8_t  *)rmach6b)[numPixels] = (i1&0xff);
			}
	    }
        
	    shade -= ebp;
	    numPixels--;
        
    } while (numPixels);
} 


//Variable used to draw column.
//This is how much you have to skip in the framebuffer in order to be one pixel below.
static int32_t bytesperline;
void setBytesPerLine(int32_t _bytesperline)
{
    bytesperline = _bytesperline;
} 



static uint8_t  mach3_al;

//FCS:  RENDER TOP AND BOTTOM COLUMN
IRAM_ATTR int32_t prevlineasm1(int32_t i1, uint8_t* palette, int32_t i3, int32_t i4, uint8_t  *source, uint8_t  *dest)
{


    if (i3 == 0)
    {
		if (!RENDER_DRAW_TOP_AND_BOTTOM_COLUMN)
            return 0;

	    i1 += i4;
        i4 = ((uint32_t)i4) >> mach3_al;
	    i4 = (i4&0xffffff00) | source[i4];

		if (pixelsAllowed-- > 0)
			*dest = palette[i4];

		

	    return i1;
    } else {
	    return vlineasm1(i1,palette,i3,i4,source,dest);
    }
}


//FCS: This is used to draw wall border vertical lines
IRAM_ATTR int32_t vlineasm1(int32_t vince, uint8_t* palookupoffse, int32_t numPixels, int32_t vplce, uint8_t* texture, uint8_t* dest)
{
    uint32_t temp;

    if (!RENDER_DRAW_WALL_BORDERS)
		return vplce;

    numPixels++;
    while (numPixels)
    {
	    temp = ((uint32_t)vplce) >> mach3_al;
        
	    temp = texture[temp];
      
		if (pixelsAllowed-- > 0)
			*dest = palookupoffse[temp];
	    
		vplce += vince;
	    dest += bytesperline;
	    numPixels--;
    }
    return vplce;
} 


IRAM_ATTR int32_t tvlineasm1(int32_t i1, uint8_t  * texture, int32_t numPixels, int32_t i4, uint8_t  *source, uint8_t  *dest)
{
    uint8_t shiftValue = (globalshiftval & 0x1f);
    
	numPixels++;
	while (numPixels)
	{
		uint32_t temp = i4;
		temp >>= shiftValue;
		temp = source[temp];

	    //255 is the index for transparent color index. Skip drawing this pixel. 
		if (temp != 255)
		{
			uint16_t colorIndex;
            
			colorIndex = texture[temp];
			colorIndex |= ((*dest)<<8);
            
			if (transrev) 
				colorIndex = ((colorIndex>>8)|(colorIndex<<8));
            
			if (pixelsAllowed-- > 0)
				*dest = transluc[colorIndex];
		}
        
		i4 += i1;
        
        //We are drawing a column ?!
		dest += bytesperline;
		numPixels--;
	}
	return i4;
} /* tvlineasm1 */


static uint8_t  tran2shr;
static uint32_t tran2pal_ebx;
static uint32_t tran2pal_ecx;
void setuptvlineasm2(int32_t i1, int32_t i2, int32_t i3)
{
	tran2shr = (i1&0x1f);
	tran2pal_ebx = i2;
	tran2pal_ecx = i3;
} /* */


IRAM_ATTR void tvlineasm2(uint32_t i1, uint32_t i2, uintptr_t i3, uintptr_t i4, uint32_t i5, uintptr_t i6)
{
	uint32_t ebp = i1;
	uint32_t tran2inca = i2;
	uint32_t tran2incb = asm1;
	uintptr_t tran2bufa = i3;
	uintptr_t tran2bufb = i4;
	uintptr_t tran2edi = asm2;
	uintptr_t tran2edi1 = asm2 + 1;

	i6 -= asm2;

	do {
		
		i1 = i5 >> tran2shr;
		i2 = ebp >> tran2shr;
		i5 += tran2inca;
		ebp += tran2incb;
		i3 = ((uint8_t  *)tran2bufa)[i1];
		i4 = ((uint8_t  *)tran2bufb)[i2];
		if (i3 == 255) { // skipdraw1
			if (i4 != 255) { // skipdraw3
				uint16_t val;
				val = ((uint8_t  *)tran2pal_ecx)[i4];
				val |= (((uint8_t  *)i6)[tran2edi1]<<8);

				if (transrev) 
					val = ((val>>8)|(val<<8));

				if (pixelsAllowed-- > 0)
					((uint8_t  *)i6)[tran2edi1] = transluc[val];
			}
		} else if (i4 == 255) { // skipdraw2
			uint16_t val;
			val = ((uint8_t  *)tran2pal_ebx)[i3];
			val |= (((uint8_t  *)i6)[tran2edi]<<8);

			if (transrev) 
                val = ((val>>8)|(val<<8));

			if (pixelsAllowed-- > 0)
				((uint8_t  *)i6)[tran2edi] = transluc[val];
		} else {
			uint16_t l = ((uint8_t  *)i6)[tran2edi]<<8;
			uint16_t r = ((uint8_t  *)i6)[tran2edi1]<<8;
			l |= ((uint8_t  *)tran2pal_ebx)[i3];
			r |= ((uint8_t  *)tran2pal_ecx)[i4];
			if (transrev) {
				l = ((l>>8)|(l<<8));
				r = ((r>>8)|(r<<8));
			}
			if (pixelsAllowed-- > 0)
			{
				((uint8_t  *)i6)[tran2edi] = transluc[l];
				((uint8_t  *)i6)[tran2edi1] =transluc[r];
				pixelsAllowed--;
			}
		}
		i6 += bytesperline;
	} while (i6 > i6 - bytesperline);
	asm1 = i5;
	asm2 = ebp;
} 



static uint8_t  machmv;
IRAM_ATTR int32_t mvlineasm1(int32_t vince, uint8_t* palookupoffse, int32_t i3, int32_t vplce, uint8_t* texture, uint8_t  *dest)
{
    uint32_t temp;

    for(;i3>=0;i3--)
    {
		temp = ((uint32_t)vplce) >> machmv;
	    temp = texture[temp];

	    if (temp != 255) 
		{
			if (pixelsAllowed-- > 0)
			*dest = palookupoffse[temp];
		}

	    vplce += vince;
	    dest += bytesperline;
    }
    return vplce;
}


void setupvlineasm(int32_t i1)
{
    mach3_al = (i1&0x1f);
}

//FCS This is used to fill the inside of a wall (so it draws VERTICAL column, always).
IRAM_ATTR void vlineasm4(int32_t columnIndex, intptr_t framebuffer)
{

	if (!RENDER_DRAW_WALL_INSIDE)
		return ;

    {
        int i;
        uint32_t temp;
        
        uintptr_t index = (framebuffer + ylookup[columnIndex]);
        uint8_t  *dest= (uint8_t *)(-ylookup[columnIndex]);
        //uint8_t  *dest= (uint8_t *)framebuffer;
        
        //uint32_t index = 0;
        do {
            for (i = 0; i < 4; i++)
            {
				
        	    temp = ((uint32_t)vplce[i]) >> mach3_al;
        	    temp = (((uint8_t *)(bufplce[i]))[temp]);
                
				if (pixelsAllowed-- > 0)
        			dest[index+i] = palookupoffse [i] [temp];
                
	            vplce[i] += vince[i];
            }
            dest += bytesperline;
        } while (((uint32_t)dest - bytesperline) < ((uint32_t)dest));
    }
} 


void setupmvlineasm(int32_t i1)
{
    //Only keep 5 first bits
    machmv = (i1&0x1f);
} 


IRAM_ATTR void mvlineasm4(int32_t column, intptr_t framebufferOffset)
{
    int i;
    uint32_t temp;
    uintptr_t index = (framebufferOffset + ylookup[column]);
    uint8_t  *dest = (uint8_t *)(-ylookup[column]);

    do {

		if (pixelsAllowed <= 0)
			return;

        for (i = 0; i < 4; i++)
        {
			
	      temp = ((uint32_t)vplce[i]) >> machmv;
	      temp = (((uint8_t *)(bufplce[i]))[temp]);
	      if (temp != 255)
		  {
			  if (pixelsAllowed-- > 0)
				dest[index+i] = palookupoffse[i][temp];
		  }
	      vplce[i] += vince[i];
        }
        dest += bytesperline;

    } while (((uint32_t)dest - bytesperline) < ((uint32_t)dest));
} 
/* END ---------------  WALLS RENDERING METHOD (USED TO BE HIGHLY OPTIMIZED ASSEMBLY) ----------------------------*/





























/* ---------------  SPRITE RENDERING METHOD (USED TO BE HIGHLY OPTIMIZED ASSEMBLY) ----------------------------*/
static int32_t spal_eax;
static int32_t smach_eax;
static int32_t smach2_eax;
static int32_t smach5_eax;
static int32_t smach_ecx;
IRAM_ATTR void setupspritevline(int32_t i1, int32_t i2, int32_t i3, int32_t i4, int32_t i5, int32_t i6)
{
    spal_eax = i1;
    smach_eax = (i5<<16);
    smach2_eax = (i5>>16)+i2;
    smach5_eax = smach2_eax + i4;
    smach_ecx = i3;
} 


IRAM_ATTR void spritevline(int32_t i1, uint32_t i2, int32_t i3, uint32_t i4, uint8_t* source, uint8_t* dest)
{
    

setup:

    i2 += smach_eax;
    i1 = (i1&0xffffff00) | (*source&0xff);
    if ((i2 - smach_eax) > i2) 
		source += smach2_eax + 1;
    else 
		source += smach2_eax;

    while(1) {
        
        i1 = (i1&0xffffff00) | (((uint8_t  *)spal_eax)[i1]&0xff);
        
        if (pixelsAllowed-- > 0)
            *dest = i1;
        
        dest += bytesperline;

        i4 += smach_ecx;
        i4--;
        if (!((i4 - smach_ecx) > i4) && i4 != 0)
            goto setup;
        
        if (i4 == 0) 
            return;
        
        i2 += smach_eax;
        
        i1 = (i1&0xffffff00) | (*source&0xff);
        
        if ((i2 - smach_eax) > i2) 
            source += smach5_eax + 1;
        else 
            source += smach5_eax;
    }
}


static int32_t mspal_eax;
static int32_t msmach_eax;
static int32_t msmach2_eax;
static int32_t msmach5_eax;
static int32_t msmach_ecx;
IRAM_ATTR void msetupspritevline(int32_t i1, int32_t i2, int32_t i3, int32_t i4, int32_t i5, int32_t i6)
{
    mspal_eax = i1;
    msmach_eax = (i5<<16);
    msmach2_eax = (i5>>16)+i2;
    msmach5_eax = smach2_eax + i4;
    msmach_ecx = i3;
} 


IRAM_ATTR void mspritevline(int32_t colorIndex, int32_t i2, int32_t i3, int32_t i4, uint8_t  * source, uint8_t  * dest)
{
 
setup:
    i2 += smach_eax;
    
    colorIndex = (colorIndex&0xffffff00) | (*source&0xff);
    
    if ((i2 - smach_eax) > i2) 
        source += smach2_eax + 1;
    else 
        source += smach2_eax;

	while(1){
    
        //Skip transparent pixels (index=255)
        if ((colorIndex&0xff) != 255)
        {
            colorIndex = (colorIndex&0xffffff00) | (((uint8_t  *)spal_eax)[colorIndex]&0xff);
            
            if (pixelsAllowed-- > 0)
                *dest = colorIndex;
        }
   
        dest += bytesperline;
        i4 += smach_ecx;
        i4--;
    
        if (!((i4 - smach_ecx) > i4) && i4 != 0)
            goto setup;
   
        if (i4 == 0) 
            return;
    
        i2 += smach_eax;
    
        colorIndex = (colorIndex&0xffffff00) | (*source&0xff);
    
        if ((i2 - smach_eax) > i2) 
            source += smach5_eax + 1;
        else 
            source += smach5_eax;
    }
}


uint8_t * tspal;
uint32_t tsmach_eax1;
uint32_t adder;
uint32_t tsmach_eax3;
uint32_t tsmach_ecx;
IRAM_ATTR void tsetupspritevline(uint8_t * palette, int32_t i2, int32_t i3, int32_t i4, int32_t i5)
{
	tspal = palette;
	tsmach_eax1 = i5 << 16;
	adder = (i5 >> 16) + i2;
	tsmach_eax3 = adder + i4;
	tsmach_ecx = i3;
} 


/*
 FCS: Draw a sprite vertical line of pixels.
 */
IRAM_ATTR void DrawSpriteVerticalLine(int32_t i2, int32_t numPixels, uint32_t i4, uint8_t  * texture, uint8_t  * dest)
{
    uint8_t colorIndex;
    
	while (numPixels)
	{
		numPixels--;
        
		if (numPixels != 0)
		{
			
			i4 += tsmach_ecx;
            
			if (i4 < (i4 - tsmach_ecx)) 
                adder = tsmach_eax3;
            
			colorIndex = *texture;
            
			i2 += tsmach_eax1;
			if (i2 < (i2 - tsmach_eax1)) 
                texture++;
            
			texture += adder;
			
            //255 is the index of the transparent color: Do not draw it.
			if (colorIndex != 255)
			{
				uint16_t val;
				val = tspal[colorIndex];
				val |= (*dest)<<8;

				if (transrev) 
					val = ((val>>8)|(val<<8));

				colorIndex = transluc[val];

				if (pixelsAllowed-- > 0)
					*dest = colorIndex;
			}
            
            //Move down one pixel on the framebuffer
			dest += bytesperline;
		}

		
	}
} 
/* END---------------  SPRITE RENDERING METHOD (USED TO BE HIGHLY OPTIMIZED ASSEMBLY) ----------------------------*/
























/* ---------------  FLOOR/CEILING RENDERING METHOD (USED TO BE HIGHLY OPTIMIZED ASSEMBLY) ----------------------------*/

void settrans(int32_t type){
	transrev = type;
}

static uint8_t  * textureData;
static uint8_t  * mmach_asm3;
static int32_t mmach_asm1;
static int32_t mmach_asm2;

void mhline(uint8_t  * texture, int32_t i2, int32_t numPixels, int32_t i4, int32_t i5, uint8_t* dest)
{
    textureData = texture;
    mmach_asm3 = asm3;
    mmach_asm1 = asm1;
    mmach_asm2 = asm2;
    mhlineskipmodify(i2,numPixels>>16,i5,dest);
}


static uint8_t  mshift_al = 26;
static uint8_t  mshift_bl = 6;
void mhlineskipmodify( uint32_t i2, int32_t numPixels, int32_t i5, uint8_t* dest)
{
    uint32_t ebx;
    int32_t colorIndex;
    
    while (numPixels >= 0)
    {
	    ebx = i2 >> mshift_al;
	    ebx = shld (ebx, (uint32_t)i5, mshift_bl);
	    colorIndex = textureData[ebx];

        //Skip transparent color.
		if ((colorIndex&0xff) != 0xff){
            if (pixelsAllowed-- > 0)
				*dest = mmach_asm3[colorIndex];
        }
	    i2 += mmach_asm1;
	    i5 += mmach_asm2;
	    dest++;
	    numPixels--;

		
    }
}


void msethlineshift(int32_t i1, int32_t i2)
{
    i1 = 256-i1;
    mshift_al = (i1&0x1f);
    mshift_bl = (i2&0x1f);
} /* msethlineshift */


static uint8_t * tmach_eax;
static uint8_t * tmach_asm3;
static int32_t tmach_asm1;
static int32_t tmach_asm2;

void thline(uint8_t  * i1, int32_t i2, int32_t i3, int32_t i4, int32_t i5, uint8_t * i6)
{
    tmach_eax = i1;
    tmach_asm3 = asm3;
    tmach_asm1 = asm1;
    tmach_asm2 = asm2;
    thlineskipmodify(asm2,i2,i3,i4,i5,i6);
}

static uint8_t  tshift_al = 26;
static uint8_t  tshift_bl = 6;
void thlineskipmodify(int32_t i1, uint32_t i2, uint32_t i3, int32_t i4, int32_t i5, uint8_t * i6)
{
    uint32_t ebx;
    int counter = (i3>>16);
    while (counter >= 0)
    {
	    ebx = i2 >> tshift_al;
	    ebx = shld (ebx, (uint32_t)i5, tshift_bl);
	    i1 = tmach_eax[ebx];
	    if ((i1&0xff) != 0xff)
	    {
		    uint16_t val = tmach_asm3[i1];
		    val |= (*i6)<<8;

		    if (transrev) 
				val = ((val>>8)|(val<<8));

			if (pixelsAllowed-- > 0)
			 *i6 = transluc[val];
	    }

	    i2 += tmach_asm1;
	    i5 += tmach_asm2;
	    i6++;
	    counter--;

		
    }
} 


void tsethlineshift(int32_t i1, int32_t i2)
{
    i1 = 256-i1;
    tshift_al = (i1&0x1f);
    tshift_bl = (i2&0x1f);
}




static intptr_t slopemach_ebx;
static int32_t slopemach_ecx;
static int32_t slopemach_edx;
static uint8_t  slopemach_ah1;
static uint8_t  slopemach_ah2;
static float asm2_f;
typedef union { unsigned int i; float f; } bitwisef2i;
void setupslopevlin(int32_t i1, intptr_t i2, int32_t i3)
{
    bitwisef2i c;
    slopemach_ebx = i2;
    slopemach_ecx = i3;
    slopemach_edx = (1<<(i1&0x1f)) - 1;
    slopemach_edx <<= ((i1&0x1f00)>>8);
    slopemach_ah1 = 32-((i1&0x1f00)>>8);
    slopemach_ah2 = (slopemach_ah1 - (i1&0x1f)) & 0x1f;
    c.f = asm2_f = (float)asm1;
    asm2 = c.i;
}

extern int32_t reciptable[2048];
extern int32_t globalx3, globaly3;
extern int32_t fpuasm;
#define low32(a) ((a&0xffffffff))
#define high32(a) ((int)(((__int64)a&(__int64)0xffffffff00000000)>>32))

//FCS: Render RENDER_SLOPPED_CEILING_AND_FLOOR
void slopevlin(intptr_t i1, uint32_t i2, int32_t i3, int32_t i4, int32_t i5, int32_t i6)
{
    bitwisef2i c;
    uint32_t ecx,eax,ebx,edx,esi,edi;
//This is so bad to cast asm3 to int then float :( !!!
    float a = (float)(int32_t) asm3 + asm2_f;
    i1 -= slopemach_ecx;
    esi = i5 + low32((__int64)globalx3 * (__int64)(i2<<3));
    edi = i6 + low32((__int64)globaly3 * (__int64)(i2<<3));
    ebx = i4;

	if (!RENDER_SLOPPED_CEILING_AND_FLOOR)
		return;

    do {
	    // -------------
	    // All this is calculating a fixed point approx. of 1/a
	    c.f = a;
	    fpuasm = eax = c.i;
	    edx = (((int32_t)eax) < 0) ? 0xffffffff : 0;
	    eax = eax << 1;
	    ecx = (eax>>24);	//  exponent
	    eax = ((eax&0xffe000)>>11);
	    ecx = ((ecx&0xffffff00)|((ecx-2)&0xff));
	    eax = reciptable[eax/4];
	    eax >>= (ecx&0x1f);
	    eax ^= edx;
	    // -------------
	    edx = i2;
	    i2 = eax;
	    eax -= edx;
	    ecx = low32((__int64)globalx3 * (__int64)eax);
	    eax = low32((__int64)globaly3 * (__int64)eax);
	    a += asm2_f;

	    asm4 = ebx;
	    ecx = ((ecx&0xffffff00)|(ebx&0xff));
	    if (ebx >= 8) ecx = ((ecx&0xffffff00)|8);

	    ebx = esi;
	    edx = edi;
	    while ((ecx&0xff))
	    {
		    ebx >>= slopemach_ah2;
		    esi += ecx;
		    edx >>= slopemach_ah1;
		    ebx &= slopemach_edx;
		    edi += eax;
		    i1 += slopemach_ecx;
		    edx = ((edx&0xffffff00)|((((uint8_t  *)(ebx+edx))[slopemach_ebx])));
		    ebx = *((uint32_t*)i3); // register trickery
		    i3 -= 4;
		    eax = ((eax&0xffffff00)|(*((uint8_t  *)(ebx+edx))));
		    ebx = esi;

			if (pixelsAllowed-- > 0)
				*((uint8_t  *)i1) = (eax&0xff);

		    edx = edi;
		    ecx = ((ecx&0xffffff00)|((ecx-1)&0xff));

			
	    }
	    ebx = asm4;
	    ebx -= 8;	// BITSOFPRECISIONPOW

		

    } while ((int32_t)ebx > 0);
}


/* END ---------------  FLOOR/CEILING RENDERING METHOD (USED TO BE HIGHLY OPTIMIZED ASSEMBLY) ----------------------------*/
