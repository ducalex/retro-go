#if 0
#ifndef _EMU2413_H_
#define _EMU2413_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef EMU2413_DLL_EXPORTS
  #define EMU2413_API __declspec(dllexport)
#elif  EMU2413_DLL_IMPORTS
  #define EMU2413_API __declspec(dllimport)
#else
  #define EMU2413_API
#endif

#if 0
#define PI 3.14159265358979
#endif

//typedef unsigned int uint32 ;
//typedef int int32 ;
//typedef signed short int16 ;
//typedef unsigned short uint16 ;
//typedef signed char int8 ;
//typedef unsigned char uint8 ;

enum {OPLL_2413_TONE=0, OPLL_VRC7_TONE=1} ;

/* voice data */
typedef struct {
  unsigned int TL,FB,EG,ML,AR,DR,SL,RR,KR,KL,AM,PM,WF ;
} OPLL_PATCH ;

/* slot */
typedef struct {

  OPLL_PATCH *patch;

  int type ;          /* 0 : modulator 1 : carrier */

  /* OUTPUT */
  int32 feedback ;
  int32 output[5] ;      /* Output value of slot */

  /* for Phase Generator (PG) */
  uint32 *sintbl ;    /* Wavetable */
  uint32 phase ;      /* Phase */
  uint32 dphase ;     /* Phase increment amount */
  uint32 pgout ;      /* output */

  /* for Envelope Generator (EG) */
  int fnum ;          /* F-Number */
  int block ;         /* Block */
  int volume ;        /* Current volume */
  int sustine ;       /* Sustine 1 = ON, 0 = OFF */
  uint32 tll ;        /* Total Level + Key scale level*/
  uint32 rks ;        /* Key scale offset (Rks) */
  int eg_mode ;       /* Current state */
  uint32 eg_phase ;   /* Phase */
  uint32 eg_dphase ;  /* Phase increment amount */
  uint32 egout ;      /* output */


  /* refer to opll-> */
  int32 *plfo_pm ;
  int32 *plfo_am ;


} OPLL_SLOT ;

/* Channel */
typedef struct {

  int patch_number ;
  int key_status ;
  OPLL_SLOT *mod, *car ;

} OPLL_CH ;

/* Mask */
#define OPLL_MASK_CH(x) (1<<(x))
#define OPLL_MASK_HH (1<<(9))
#define OPLL_MASK_CYM (1<<(10))
#define OPLL_MASK_TOM (1<<(11))
#define OPLL_MASK_SD (1<<(12))
#define OPLL_MASK_BD (1<<(13))
#define OPLL_MASK_RYTHM ( OPLL_MASK_HH | OPLL_MASK_CYM | OPLL_MASK_TOM | OPLL_MASK_SD | OPLL_MASK_BD )

/* opll */
typedef struct {

  uint32 adr ;

  int32 output[2] ;

  /* Register */
  unsigned char reg[0x40] ;
  int slot_on_flag[18] ;

  /* Rythm Mode : 0 = OFF, 1 = ON */
  int rythm_mode ;

  /* Pitch Modulator */
  uint32 pm_phase ;
  int32 lfo_pm ;

  /* Amp Modulator */
  int32 am_phase ;
  int32 lfo_am ;


  /* Noise Generator */
  uint32 noise_seed ;
  uint32 whitenoise ;
  uint32 noiseA ;
  uint32 noiseB ;
  uint32 noiseA_phase ;
  uint32 noiseB_phase ;
  uint32 noiseA_idx ;
  uint32 noiseB_idx ;
  uint32 noiseA_dphase ;
  uint32 noiseB_dphase ;

  /* Channel & Slot */
  OPLL_CH *ch[9] ;
  OPLL_SLOT *slot[18] ;

  /* Voice Data */
  OPLL_PATCH *patch[19*2] ;
  int patch_update[2] ; /* flag for check patch update */

  uint32 mask ;

  int masterVolume ; /* 0min -- 64 -- 127 max (Liner) */

} OPLL ;

/* Initialize */
EMU2413_API void OPLL_init(uint32 clk, uint32 rate) ;
EMU2413_API void OPLL_close(void) ;

/* Create Object */
EMU2413_API OPLL *OPLL_new(void) ;
EMU2413_API void OPLL_delete(OPLL *) ;

/* Setup */
EMU2413_API void OPLL_reset(OPLL *) ;
EMU2413_API void OPLL_reset_patch(OPLL *, int) ;
EMU2413_API void OPLL_setClock(uint32 c, uint32 r) ;

/* Port/Register access */
EMU2413_API void OPLL_writeIO(OPLL *, uint32 reg, uint32 val) ;
EMU2413_API void OPLL_writeReg(OPLL *, uint32 reg, uint32 val) ;

/* Synthsize */
EMU2413_API int16 OPLL_calc(OPLL *) ;

/* Misc */
EMU2413_API void OPLL_copyPatch(OPLL *, int, OPLL_PATCH *) ;
EMU2413_API void OPLL_forceRefresh(OPLL *) ;
EMU2413_API void dump2patch(unsigned char *, OPLL_PATCH *) ;

/* Channel Mask */
EMU2413_API uint32 OPLL_setMask(OPLL *, uint32 mask) ;
EMU2413_API uint32 OPLL_toggleMask(OPLL *, uint32 mask) ;

EMU2413_API void OPLL_update(OPLL *opll, int16 **buffer, int length) ;
EMU2413_API void OPLL_write(OPLL *opll, int offset, int data) ;

#ifdef __cplusplus
}
#endif

#endif
#endif
