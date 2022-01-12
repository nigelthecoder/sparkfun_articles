/** \file serial_utils.h
 *  \brief Macros and functions to support printf-like serial output.
 *
 *  This header contains a series of macros and functions to make
 *  it easier to send printf-like output to the Arduino Serial Monitor app
 *  when debugging your code.
 *
 *  Because the serial_printf function cannot print floats,
 *  there are some functions included to convert floats to const char* strings
 *  using a common buffer so that they can be included in printf strings using %s
 *  as the format.
 *
 *
 *
 */

#ifndef _SERIAL_UTILS_H_
#define _SERIAL_UTILS_H_

#include "Arduino.h"


/// \brief print to the serial port.
///
/// This allows the use of printf-like formatting but
/// does not support floats or doubles. It includes a line feed
/// character at the end of the string.
///
/// You must either call Serial.begin(baud_rate) before calling this function
/// or call nt::core_begin().
///
/// Beware that the buffer used to format the output is only 128 chars
/// so do not exceed this size. 
///
/// \param fmt The printf-like formatting string.
/// \param ... The argument list to format.
void serial_printf(const char* fmt, ...);

/// \brief A macro to shorten nt::serial_printf
#define sout serial_printf

/// \brief Convert a float value to a const char* string.
/// The function uses a common char buffer so do not store the returned
/// char* pointer.
///
/// \param value The float value to format.
/// \param places The number of decimal places to format the value with.
/// \return A pointer to the formatted string.
const char* f2s(float& value, uint8_t places);

///////////////////////////////////////////////////////////////////////////////////
//
// Debug support
//
// Ref: http://dbp-consulting.com/tutorials/SuppressingGCCWarnings.html
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402
#define GCC_DIAG_STR(s) #s
#define GCC_DIAG_JOINSTR(x,y) GCC_DIAG_STR(x ## y)
# define GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)
# define GCC_DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
# if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
#  define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(push) \
  GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W,x))
#  define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(pop)
# else
#  define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W,x))
#  define GCC_DIAG_ON(x)  GCC_DIAG_PRAGMA(warning GCC_DIAG_JOINSTR(-W,x))
# endif
#else
# define GCC_DIAG_OFF(x)
# define GCC_DIAG_ON(x)
#endif

#ifdef DEBUG

#define dbg serial_printf

#else // not DEBUG

GCC_DIAG_OFF(unused-value)

/// \brief Write a debug message to the serial port.
///
/// Use this like serial_printf to format and print a message written
/// to the serial port. The output is only generated when DEBUG
/// is defined. You must #define NT_DEBUG before including this header
/// or any other header that includes this one.
#define dbg (void) // Note that this generates 'unused-value' warnings without prev macro

#endif // not DEBUG


#endif // _SERIAL_UTILS_H_
