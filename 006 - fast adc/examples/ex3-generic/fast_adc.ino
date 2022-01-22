/** \file fast_adc.cpp
 *
 * Fast ADC for Arduino Uno or Mega
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

#include "fast_adc.h"

 FastAdc::FastAdc(const uint8_t* p_fast_list, uint8_t num_fast,
         const uint8_t* p_slow_list, uint8_t num_slow)
 : m_p_fast_list(p_fast_list)
 , m_num_fast(num_fast)
 , m_p_slow_list(p_slow_list)
 , m_num_slow(num_slow)
 , m_adc_pin(0)
 , m_adc_hiprio(true)
 , m_adc_hipri_index(0)
 , m_adc_lopri_index(0)
 , m_adc_start_time(0)
 , m_adc_conv_time(0)
 , m_isr_time(0)
 {

 }

 void FastAdc::begin()
 {
  // Oh so cheesy but this is how we roll so we can hook
  // into the Arduino ISR system
  FastAdc::s_pInst = this;

  // Configure the ADC with a prescaler of 16 (so it's faster)
  ADCSRA =  bit(ADEN);   // turn ADC on
  ADCSRA |= bit(ADPS2);  // Prescaler of 16

  // Set the mux for the first port in the hi prio list
  _setAdcMux(m_p_fast_list[m_adc_hipri_index]);

  // start the first conversion with interrupt enabled
  m_adc_start_time = micros();
  ADCSRA |= bit(ADSC) | bit(ADIE);
}

 // Get the complete set of samples. The pointer must be to
 // an array NUM_ANALOG_PORTS in size
 const void FastAdc::getSamples(uint16_t* buf, uint8_t num_samples)
 {
	 ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		 memcpy(buf, (const void*)m_adc_samples, num_samples * sizeof(uint16_t));
	 }
 }


 // get a sampled value. Port can be like A3 or the actual index like 3
 uint16_t FastAdc::sample(uint8_t port)
 {
	 uint16_t s;
	 ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		 s = m_adc_samples[_ATOPN(port)];
	 }
	 return s;
 }


void FastAdc::_setAdcMux(uint8_t analogPin)
{
  // set the ADC mux for the specified pin
  // like: A0..A15, or 0..15
  // on Uno A0..A7 is 14..21
  // on Mega A0..A15 is 54..69

  uint8_t index = _ATOPN(analogPin); // 0..15
  ADMUX = bit(REFS0) | (index & 0x07);

#if defined (__AVR_ATmega2560__)

  // Mega mux has another selector for 8..15
  ADCSRB = (index > 7) ? bit(MUX5) : 0;
#endif

}

// Horrible global pointer to the instance to use
FastAdc* FastAdc::s_pInst = 0;

// Hook into the ADC interrupt vecotor
ISR (ADC_vect)
{
  if (FastAdc::s_pInst) {
    FastAdc::s_pInst->_isr();
  }
}

void FastAdc::_isr()
{
  // compute the conversion time
  unsigned long isr_start_time = micros();
  m_adc_conv_time = isr_start_time - m_adc_start_time;

  // read the 16-bit result of the previous conversion
  m_adc_samples[m_adc_pin] = ADC;

  // figure out which analog pin to sample next
  if (m_adc_hiprio) {
    // we are doing the hi prio list
    m_adc_hipri_index++;
    if (m_adc_hipri_index >= m_num_fast) {
      // end of the hi prio list
      m_adc_hipri_index = 0;
      if (m_p_slow_list) {
        // take the next one from the slow list
        m_adc_hiprio = false;
        m_adc_pin = _ATOPN(m_p_slow_list[m_adc_lopri_index]);
      } else {
        // we have no slow list so just go back to the start of the fast list
        m_adc_hiprio = true;
        m_adc_pin = _ATOPN(m_p_fast_list[0]);
      }
      // notify derived class
      onFastUpdate();
    } else {
      // set up for next one off the hi prio list
      m_adc_pin = _ATOPN(m_p_fast_list[m_adc_hipri_index]);
    }
  } else {
    // we just did one of the lo prio pins
    // so we go back to the hi prio pins
    m_adc_hiprio = true;
    m_adc_hipri_index = 0;
    m_adc_pin = _ATOPN(m_p_fast_list[0]);
    // set up for next lo prio one
    m_adc_lopri_index++;
    if (m_adc_lopri_index >= m_num_slow) {
      // end of the low prio list
      m_adc_lopri_index = 0;
      // notify derived class
      onSlowUpdate();
    }
  }

  // set the mux for the next conversion
  _setAdcMux(m_adc_pin);

  // start the next ADC conversion
  m_adc_start_time = micros();
  ADCSRA |= bit (ADSC) | bit (ADIE);

  m_isr_time = m_adc_start_time - isr_start_time;
}

uint32_t FastAdc::getIsrTime()
{
  uint32_t t;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    t = m_isr_time;
  }
  return t;
}

uint32_t FastAdc::getAdcTime()
{
  uint32_t t;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    t = m_adc_conv_time;
  }
  return t;
}
