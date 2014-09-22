/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Adam Strzelecki <ono@java.pl>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmStdoutFilter_h
#define cmStdoutFilter_h

#if !defined(CMAKE_USE_STDOUT_FILTER)
# error "This file may be included only if CMAKE_USE_STDOUT_FILTER is enabled."
#endif

#include "cmThread.h"

/** \class cmStdoutFilter
 * \brief Compacts subsequent stdout lines beginning with given prefix.
 *
 * E.g.:
 *   -- Looking for iconv.h
 *   -- Looking for iconv.h - found
 *
 * Gets replaced by:
 *   -- Looking for iconv.h - found
 *
 * However when anything comes in between then replacement is not done, e.g:
 *   -- Looking for iconv.h
 *   -- Failed to compile iconv_test.c
 *   -- Looking for iconv.h - failed
 */
class cmStdoutFilter : private cmThread
{
public:
  cmStdoutFilter(const char* prefix);
  virtual ~cmStdoutFilter();

protected:
  virtual void operator ()();

private:
  const char* const Prefix;
  const ssize_t PrefixLength;
  int fds[4];
};

#endif
