//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2024 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// This file has been modified by Dave Bernazzani (wavemotion-dave)
// for optimized execution on the DS/DSi platform. Please seek the
// official Stella source distribution which is far cleaner, newer,
// and better maintained.
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#ifndef M6502_HXX
#define M6502_HXX

class M6502;

#include "bspf.hxx"
#include "System.hxx"

extern uInt8    A; 
extern uInt8    X; 
extern uInt8    Y; 
extern uInt8    SP;
extern uInt16   gPC;
extern uInt8    N; 
extern uInt8    V; 
extern uInt8    B; 
extern uInt8    D; 
extern uInt8    I; 
extern uInt8    C;
extern uInt8    notZ;
extern uInt8    myDataBusState;
extern uInt32   NumberOfDistinctAccesses;
extern uInt8    cartDriver;
extern uInt16   f8_bankbit;

extern volatile int *stack_executionStatus;

/**
  This is an abstract base class for classes that emulate the
  6502 microprocessor.  The 6502 is an 8-bit microprocessor that
  has a 64K addressing space.

  @author  Bradford W. Mott
  @version $Id: M6502.hxx,v 1.2 2002/05/13 19:10:25 stephena Exp $ 
*/
class M6502
{
  public:
    /**
      Create a new 6502 microprocessor with the specified cycle 
      multiplier.  The cycle multiplier is the number of system cycles 
      per processor cycle.

      @param systemCyclesPerProcessorCycle The cycle multiplier
    */
    M6502(uInt32 systemCyclesPerProcessorCycle);

    /**
      Destructor
    */
    virtual ~M6502();

  public:
    /**
      Install the processor in the specified system.  Invoked by the
      system when the processor is attached to it.

      @param system The system the processor should install itself in
    */
    virtual void install(System& system);

  public:
    /**
      Reset the processor to its power-on state.  This method should not 
      be invoked until the entire 6502 system is constructed and installed
      since it involves reading the reset vector from memory.
    */
    virtual void reset();

    /**
      Get a null terminated string which is the processor's name (i.e. "M6532")

      @return The name of the device
    */
    virtual const char* name() const = 0;

  public:
    /**
      Execute instructions until the specified number of instructions
      is executed, someone stops execution, or an error occurs.  Answers
      true iff execution stops normally.

      @param number Indicates the number of instructions to execute
      @return true iff execution stops normally
    */
    virtual void execute(void)              = 0;
    virtual void execute_4K(void)           = 0;
    virtual void execute_F8(void)           = 0;
    virtual void execute_F6(void)           = 0;
    virtual void execute_F4(void)           = 0;
    virtual void execute_AR(void)           = 0;
    virtual void execute_F8SC(void)         = 0;
    virtual void execute_F6SC(void)         = 0;
    virtual void execute_DPCP(void)         = 0;
    virtual void execute_CDFJ(void)         = 0;
    virtual void execute_CDFJPlus(void)     = 0;
    virtual void execute_CDFJPlusPlus(void) = 0;
    virtual void execute_DPC(void)          = 0;
    virtual void execute_CTY(void)          = 0;

    /**
      Tell the processor to stop executing instructions.  Invoking this 
      method while the processor is executing instructions will stop 
      execution as soon as possible.
    */
    void stop();

  protected:
    /**
      Get the 8-bit value of the Processor Status register.

      @return The processor status register
    */
    uInt8 PS() const;

    /**
      Change the Processor Status register to correspond to the given value.

      @param ps The value to set the processor status register to
    */
    void PS(uInt8 ps);

  protected:

    /**
      Constants used for setting bits in myExecutionStatus
    */
    enum 
    {
      StopExecutionBit = 0x01,
      FatalErrorBit = 0x02,
      MaskableInterruptBit = 0x04,
      NonmaskableInterruptBit = 0x08
    };
  
    /// Pointer to the system the processor is installed in or the null pointer
    System* mySystem;

  protected:
    /// Lookup table used for binary-code-decimal math
    static uInt8 ourBCDTable[2][256];
};
#endif

