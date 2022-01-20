/*
 * Implementation for critical section locks
 * 
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
 */

#include "critical_section.h"

#ifdef ARDUINO_ARCH_APOLLO3

  // The constructor establishes the lock by disabling interrupts
  __CsLock::__CsLock()
  : m_int_master(am_hal_interrupt_master_disable())
  {  
  }

  // The destructor removes the lock by enabling interrupts again
  __CsLock::~__CsLock()
  {
    am_hal_interrupt_master_set(m_int_master);
  }

# else 
// Assume normal ATmega processor boards

  // The constructor establishes the lock by disabling interrupts
  __CsLock::__CsLock()
  : m_sreg(SREG)
  {
    // disable the global interrupt flag
    SREG &= ~(1 << SREG_I);
  }

  // The destructor removes the lock by enabling interrupts again
  __CsLock::~__CsLock()
  {
    // restore the interrupt state
    SREG = m_sreg;
  }

#endif
