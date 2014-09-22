/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Adam Strzelecki <ono@java.pl>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmStandardIncludes.h" // to get CMAKE_USE_STDOUT_FILTER first
#include "cmThread.h"

//----------------------------------------------------------------------------
cmThread::~cmThread()
{
  if (running)
    {
    pthread_join(tid, NULL);
    }
}

//----------------------------------------------------------------------------
bool cmThread::Run()
{
  running = pthread_create(&tid, NULL, pthreadEntryPoint, (void *)this) != 0;
  return running;
}

//----------------------------------------------------------------------------
void *cmThread::pthreadEntryPoint(void *ptr)
{
  (*((cmThread *)ptr))();
  return ptr;
}
