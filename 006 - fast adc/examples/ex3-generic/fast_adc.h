/** \file fast_adc.h
 *
 * Fast ADC class declration.
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
 */

#ifndef _FAST_ADC_H_
#define _FAST_ADC_H_

#include "Arduino.h"
// We use a locking mechanism from the AVR sources
#include "util/atomic.h"

#if defined (__AVR_ATmega2560__)

// The Arduino Mega has 16 analog ports. We allow for the max here as it simplifies storing and
// retrieving the ADC samples
#define NUM_ANALOG_PORTS 16

#else
// Assume we are on Uno with 8 ADC inputs,
// although it really only has 6 accesible

#define NUM_ANALOG_PORTS 8

#endif // (__AVR_ATmega2560__)

/// \brief Fast analog to digital converter (ADC)
/// This class can be used directly to sample a set of fast and/or slow
/// ADC inputs.
class FastAdc
{
public:
  /// \brief Construct with a list of fast ports and a list of slow ports.
  /// The constructor takes pointers to two lists of analog port numbers to sample
  /// a null pointer can be used to say that a specific list isn't present.
  /// Values in either list should be like A0, A2, A5, etc.
  /// \param p_fast_list A pointer to the list of analog ports to be sampled
  /// as fast as possible. This list cannot be empty.
  /// \param num_fast The number of ports in the fast list. This must be at least one.
  /// \param p_slow_list A pointer to the list of analog ports to be sampled
  /// slowly.
  /// \param num_slow The number of ports in the slow list.
  FastAdc(const uint8_t* p_fast_list, uint8_t num_fast,
          const uint8_t* p_slow_list, uint8_t num_slow);

  /// \brief Call this to set up the ADc conversions.
  /// Call this from your application's \c setup() function to
  /// start the conversion process.
  void begin();

  /// \brief Get a set of samples.
  /// Copies the already sampled values to a buffer. The buffer must be
  /// big enough for the samples to be copied. Max samples is 8 for Uno
  /// and 16 for Mega. Note that the first sample in the buffer is A0, the
  /// seconds is A1 and so on.
  const void getSamples(uint16_t* buf, uint8_t num_samples);

  /// \brief Get a sampled value.
  /// Returns a single sample for a specific port.
  /// \param port can be like A3 or just the index index like 3.
  /// \return The sample value.
  uint16_t sample(uint8_t port);

  /// \brief Get the most recent ISR run time
  /// \return Returns the ISR time in microseconds
  uint32_t getIsrTime();

  /// \brief Get the most recent ADC conversion time.
  /// \brief Returns the ADC conversion time in microseconds;
  uint32_t getAdcTime();

  // BUGBUG make these private
  // static (global) pointer to the instance of this class
  static FastAdc* s_pInst;

  // public function that implements our ISR
  void _isr();

protected:
  /// \brief Callback when fast samples are available.
  /// Overload this function in your derived class to be called when all the
  /// fast list has been updated. Note that your code is inside the ISR so
  /// it needs to be fast and not interfere with foreground variables etc.
  virtual void onFastUpdate()
  {
  }

  /// \brief Callback when the slow sampled inputs are available.
  /// Overload this function in your derived class to be called when all the
  /// slow list has been updated. Note that your code is inside the ISR so
  /// it needs to be fast and not interfere with foreground variables etc.
  virtual void onSlowUpdate()
  {
  }

  // This array is where the ISR stores the analog values it reads
  // Not all entries may be used
  volatile uint16_t m_adc_samples[NUM_ANALOG_PORTS];

private:
  // the lists of fast and slow update ports to sample
  const uint8_t* m_p_fast_list;
  const uint8_t m_num_fast;
  const uint8_t* m_p_slow_list;
  const uint8_t m_num_slow;

  // A bunch of variables we use inside the ISR to keep track of which port
  // we are doing next
  volatile uint8_t m_adc_pin; // the analog pin we are currently sampling
  volatile bool m_adc_hiprio; // true if we are on the hi priority list now
  volatile uint8_t m_adc_hipri_index; // The high prio port we do next
  volatile uint8_t m_adc_lopri_index; // The low prio port we do next

  // data so we can see how long the ISR takes
  // TBD: use lo pass filter to average these?
  volatile unsigned long m_adc_start_time;
  volatile unsigned long m_adc_conv_time;
  volatile unsigned long m_isr_time;


  // fn to convert the analog pin identifiers like A0 to the analog port
  // index numer (0..N-1)
  // On the UNO A0 is 14
  inline uint8_t _ATOPN(uint8_t p)
  {
    return ((p < NUM_ANALOG_PORTS) ? p : (p - A0));
  }

  void _setAdcMux(uint8_t analogPin);

};



 #endif // _FAST_ADC_H_
