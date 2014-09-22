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
#include "cmStdoutFilter.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>

//----------------------------------------------------------------------------
static void cmStdoutFilterPreparePipe(int* fds, int dup_fd)
{
  pipe(fds);
  int out_fd = dup(dup_fd);
  dup2(fds[1], dup_fd);
  close(fds[1]);
  fds[1] = out_fd;
  fcntl(fds[0], F_SETFL, fcntl(fds[0], F_GETFL, 0) | O_NONBLOCK);
}

//----------------------------------------------------------------------------
cmStdoutFilter::cmStdoutFilter(const char* prefix)
: Prefix(prefix)
, PrefixLength(strlen(prefix))
{
  cmStdoutFilterPreparePipe(&fds[0], STDERR_FILENO);
  cmStdoutFilterPreparePipe(&fds[2], STDOUT_FILENO);

  // Restore stdout flush on new line and stderr non-blocking.
  setvbuf(stdout, NULL, _IOLBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  Run();
}

//----------------------------------------------------------------------------
cmStdoutFilter::~cmStdoutFilter()
{
  // Ensure output gets flushed.
  fflush(stdout);
  fflush(stderr);

  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}

//----------------------------------------------------------------------------
void cmStdoutFilter::operator ()()
{
  int matchlen = 0;
  char match[1024];
  for (;;)
    {
    int max_fd = -1;
    fd_set readset;
    FD_ZERO(&readset);
    for (int i = 0; i < 4; i += 2)
      {
      if (fds[i] >= 0)
        {
        FD_SET(fds[i], &readset);
        if (max_fd < fds[i]) max_fd = fds[i];
        }
      }
    // Exit if there are no more fds to watch on:
    if (max_fd == -1) break;
    // Wait for data and quit on iterrupt:
    if (select(max_fd + 1, &readset, NULL, NULL, NULL) == -1 && errno == EINTR)
      {
      break;
      }
    // Process changes:
    for (int i = 0; i < 4; i += 2)
      {
      if (fds[i] >= 0 && FD_ISSET(fds[i], &readset))
        {
        char buf[1024];
        ssize_t len;
        if ((len = read(fds[i], buf, sizeof(buf))) > 0)
          {
          char* line = buf, *lf;
          // Check for matching subsequent lines:
          if (i == 2 /* only on stdout */)
            {
            // Process stdout line by line:
            while (line < buf + len && (lf = strchr(line, '\n')))
              {
              ssize_t linelen = lf - line;
              // Do matching only on lines beginning with prefix
              if (linelen > PrefixLength && !memcmp(line, Prefix, PrefixLength))
                {
                // Is there line to match?
                if (matchlen > 0)
                  {
                  if (matchlen < linelen && !memcmp(line, match, matchlen))
                    {
                    // Just write remaining suffix instead whole line.
                    write(fds[3], line + matchlen,
                          linelen - matchlen + 1);
                    matchlen = 0;
                    line = lf + 1;
                    continue;
                    }
                  // Terminate previous line.
                  write(fds[3], "\n", 1);
                  }
                // Store this line for next match.
                memcpy(match, line, linelen);
                matchlen = linelen;
                // Write incomplete line without trailing line break.
                write(fds[3], line, linelen);
                }
              else
                {
                if (matchlen > 0)
                  {
                  // Terminate previous line.
                  write(fds[3], "\n", 1);
                  matchlen = 0;
                  }
                // Write complete line with terminator.
                write(fds[3], line, linelen + 1);
                }
              line = lf + 1;
              }
            }
          if (line < buf + len)
            {
            // Terminate previous stdout match:
            if (matchlen > 0)
              {
              write(fds[3], "\n", 1);
              matchlen = 0;
              }
            write(fds[i + 1], line, buf + len - line);
            }
          }
        else
          {
          // Terminate previous stdout match:
          if (i == 2 /* stdout */ && matchlen > 0)
            {
            write(fds[3], "\n", 1);
            }
          close(fds[i]);
          fds[i] = -1;
          }
        }
      }
    }
}
