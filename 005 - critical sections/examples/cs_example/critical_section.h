/*
 * Critical section support macros
 * 
 * MIT License:
 *
 * Copyright 2022 Nigel Thompson
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
 * THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * 
 * 
 * There are two sets of macros/functions here to support disabling and re-enabling
 * interrupts for critcal sections in your code.
 * These macros will work on conventional ATmega boards like the Uno, Mega, 
 * or the SparkFun RedBoards that use the ATmega processors, and also
 * on the SparkFun Artemis boards that use the Apollo 3 MCU.
 * The idea is expanadbale to other MCU families of course but the implementation
 * here will try to detect the Apollo 3 MCU or default to the ATmega MCU.
 * 
 */

#ifndef _CRITICAL_SECTION_H_
#define _CRITICAL_SECTION_H_

#include "Arduino.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Critical section macros that are used in pairs
//
//
// Usage:
   
/*  
  // Start a crtical section, disabling interrupts
  CS_BEGIN

    Your code that runs with interupts off
    goes here.

  // End the critical secion, restoring interrupts
  CS_END
  
*/


#ifdef ARDUINO_ARCH_APOLLO3

// Artemis boards (using macros from am_reg_macros.h)
#define CS_BEGIN AM_CRITICAL_BEGIN
#define CS_END AM_CRITICAL_END

#else

// Assume normal ATmega processor boards (using AVR macros)
#include "util/atomic.h"
#define CS_BEGIN ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
#define CS_END }

#endif


/////////////////////////////////////////////////////////////////////////////////////////
// Critical section implementation for code blocks that uses one macro
//
// A small class to provide a critical section inside a code scope block
// like the body of a function, or inside a pair of curly braces { }
// The class saves the interrupt state, then disable interrupts. In the
// dtor it restores the state of the interrupt register.
// The macro simply creates an instance of the class.

class __CsLock
{
public:
  __CsLock();
  ~__CsLock();

private:
#ifdef ARDUINO_ARCH_APOLLO3 // Atermis with Apollo 3 MCU
  volatile uint32_t m_int_master;
#else // Assume normal ATmega processor boards
  volatile uint8_t m_sreg;
#endif
};

// A macro to use in the code like this:
//
// { // start lock scope
//    CS_LOCK
//    your protected code
// } // end of lock scope
//
// The code block can be the body of an entire function or simply
// a block between { and } anywhere in the code.
#define CS_LOCK __CsLock __thisCsLock;

#endif // _CRITICAL_SECTION_H_
