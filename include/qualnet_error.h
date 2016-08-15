// Copyright (c) 2001-2015, SCALABLE Network Technologies, Inc.  All Rights Reserved.
//                          600 Corporate Pointe
//                          Suite 1200
//                          Culver City, CA 90230
//                          info@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the QualNet
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

/// \defgroup Package_ERROR ERROR

/// \file
/// \ingroup Package_ERROR
/// This file defines data structures and functions used in error-handling.


#ifndef QUALNET_ERROR_H
#define QUALNET_ERROR_H

#include <stdarg.h>
#include <stdio.h>
#include <string>

#include "types.h"  // for BOOL
#include "main.h"   // for MAX_STRING_LENGTH  and _WIN32's snprintf()

/// Signals an assertion error to ERROR_WriteError
#define ERROR_ASSERTION 0

/// Signals a fatal error to ERROR_WriteError
#define ERROR_ERROR     1

/// Signals a recoverable warning to ERROR_WriteError
#define ERROR_WARNING   2

extern char* ERROR_Filename;
extern char* ERROR_Condition;
extern int   ERROR_LineNumber;
extern int   ERROR_Type;

#ifdef NO_ASSERTS
#define ERROR_Assert(expr, str)
#define ERROR_AssertArgs(expr, ...)
#ifdef assert
#undef assert
#endif
#define assert(expr)
#else

/// May be used in place of assert, to include an error message
///
/// \par Example:
/// \code
/// ERROR_Assert(timeoutParameter > 0,
///     "The TIMEOUT parameter must be positive.");
/// \endcode
#define ERROR_Assert(expr, str)  ((expr) || ERROR_WriteError(ERROR_ASSERTION, # expr, str, __FILE__, __LINE__))

/// Like ERROR_Assert but can take a \c printf style format and arguments
/// instead of a static error string.
///
/// \par Example:
/// \code
/// ERROR_AssertArgs(numChannels <= MAX_NUM_CHANNELS,
///     "The number of channels given, %d, is too large -- reduce it "
///     "to %d or less.", numChannels, MAX_NUM_CHANNELS);
/// \endcode
#define ERROR_AssertArgs(expr, ...) \
    ((expr) || ERROR_WriteErrorArgs(ERROR_ASSERTION, # expr, \
                                    __FILE__, __LINE__, __VA_ARGS__))

#ifdef assert
#undef assert
#endif // assert

/// In DEBUG mode assert macro will be replaced by
/// ERROR_WriteError with ERROR_ASSERTION type
#define assert(expr)  ((expr) || ERROR_WriteError(ERROR_ASSERTION, # expr, NULL, __FILE__, __LINE__))
#endif // NO_ASSERTS

/// Function call used to report an error condition in QualNet,
/// and notify GUI of such.
#define ERROR_ReportError(str)  ERROR_WriteError(ERROR_ERROR, NULL, str, __FILE__, __LINE__)

/// Macro used to report an error condition with variable arguments,
/// similar to ERROR_ReportError.
#define ERROR_ReportErrorArgs(...) \
    ERROR_WriteErrorArgs(ERROR_ERROR, NULL, __FILE__, __LINE__, __VA_ARGS__)

/// Function call used to report a recoverable error condition.
/// This macro in turns calls ERROR_WriteError with ERROR_WARNING type. It
/// reports a warning message in QualNet, and notify GUI of such
#define ERROR_ReportWarning(str)  ERROR_WriteError(ERROR_WARNING, NULL, str, __FILE__, __LINE__)

/// Macro used to report a recoverable error condition
/// with variable arguments, similar to ERROR_ReportWarning.
#define ERROR_ReportWarningArgs(...) \
    ERROR_WriteErrorArgs(ERROR_WARNING, NULL, __FILE__, __LINE__, __VA_ARGS__)

#define DEBUG_PRINT(fmt, ...) \
    do { if (DEBUG) fprintf(stdout, fmt, __VA_ARGS__); } while (0)

/// Function call used to report failed assertions, errors,
/// and warnings, and notify the GUI of such.  The user should not call this
/// function directly, but should use one of the previously defined macros.
///
/// \param type  assertion, error, or warning
/// \param condition  a string representing the failed assertion condition
/// \param msg  an error message
/// \param file  the file name in which the error occurred
/// \param lineno  the line on which the error occurred.
extern BOOL ERROR_WriteError(int   type,
                             const char* condition,
                             const char* msg,
                             const char* file,
                             int   lineno);

/// Function call used to report failed assertions, errors,
/// and warnings, and notify the GUI of such.  The user should not call this
/// function directly, but should use one of the previously defined macros.
/// This differs from ERROR_WriteError() only in taking variable arguments.
///
/// \param type  assertion, error or warning
/// \param condition  a string representing the failed boolean condition
/// \param file  the file name in which the error occurred
/// \param lineno  the line on which the error occurred
/// \param fmt  a \c printf style format string
extern BOOL ERROR_WriteErrorArgs(int type,
                                 const char* condition,
                                 const char* file,
                                 int lineno,
                                 const char* fmt,
                                 ...)
#ifdef __GNUC__
__attribute__((__format__(__printf__, 5, 6) ))
#endif
;

typedef void (*QErrorHandler)(int type, const char * errorMessage);


///
/// Function used to register a callback function. The callback function
/// will be invoked by ERROR_ when ERROR_WriteError () is invoked.
/// For example - logging error messages into a log file or send the error
/// messages to another application (e.g. to the Qualnet IDE that started
/// the simulation.)
///
/// \param type  assertion, error, or warning
/// \param condition  a string representing the failed boolean condition
/// \param msg  an error message
/// \param file  the file name in which the assertion failed
/// \param lineno  the line on which the assertion failed.
/// \param functionPointer  pointer to a function with signature
///    that takes the error type and the
///    error message string.
extern void ERROR_InstallHandler(QErrorHandler functionPointer);

/// Reports an error when user attempts to use a model that
/// hasn't been installed, either because the customer hasn't
/// purchased that feature, or they haven't downloaded and
/// compiled it.
///
/// \param model  the name of the model/protocol being used.
/// \param addon  the name of the addon to which the model belongs
inline void ERROR_ReportMissingAddon(const char* model,
                                     const char* addon) {
    ERROR_ReportErrorArgs("The %s model requires the %s addon", model, addon);
}

/// Reports an error when user attempts to use a model that
/// hasn't been installed, either because the customer hasn't
/// purchased that feature, or they haven't downloaded and
/// compiled it.
///
/// \param model  the name of the model/protocol being used.
/// \param iface  the name of the interface to which the model belongs
inline void ERROR_ReportMissingInterface(const char* model,
                                         const char* iface) {
    ERROR_ReportErrorArgs("The %s model requires the %s interface", model, iface);
}

/// Reports an error when user attempts to use a model that
/// hasn't been installed, either because the customer hasn't
/// purchased that feature, or they haven't downloaded and
/// compiled it.
///
/// \param model  the name of the model/protocol being used.
/// \param library  the name of the library to which the model belongs
inline void ERROR_ReportMissingLibrary(const char* model,
                                       const char* library) {
    ERROR_ReportErrorArgs("The %s model requires the %s library", model, library);
}

#endif

#ifdef WINDOWS_OS
typedef unsigned long error_t;
#else
typedef int error_t;
#endif

/// \brief Return error string for a given error number.
///
/// This function returns a string representing the given system error number.
/// \param errorNumber a system error number to be converted to text
/// \return the error string
std::string ERROR_GetErrorStdStr(error_t errorNumber);


/// \brief Return error string for last system error.
///
/// This function returns a string representing the last error that
/// occurred (based on \c errno on Linux, or the result of
/// GetLastError() on Windows).
///
/// \return the error string
std::string ERROR_GetErrorStdStr(void);

/// \brief Return warning string for \c printf style format string.
///
/// This function returns a string representing of the warnings that,
/// occuerred, in the given \c printf style format string.
///
/// \param cStyleFormat  a \c printf style format string
///
/// \return the warning string
std::string ERROR_FormatString(const char* cStyleFormat, ...);
