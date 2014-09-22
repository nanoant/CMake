/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Adam Strzelecki <ono@java.pl>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmThread_h
#define cmThread_h

#if !defined(CMAKE_USE_STDOUT_FILTER)
# error "This file may be included only if CMAKE_USE_STDOUT_FILTER is enabled."
#endif

#include <pthread.h>

/** \class cmThread
 * \brief Provides generic platform independent thread creation facility.
 *
 * Derieved classes should override () operator with thread body.
 */
class cmThread
{
public:
  cmThread() : running(false) {};
  virtual ~cmThread();
  bool Run();
  bool IsRunning() { return running; };

protected:
  virtual void operator ()() = 0;

private:
  pthread_t tid;
  bool running;
  static void *pthreadEntryPoint(void *ptr);
};

#endif
