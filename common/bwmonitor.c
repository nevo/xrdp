/*
   Copyright (c) 2012 Nevo

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.

   Bandwidth Monitor
*/

#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "bwmonitor.h"
#include "os_calls.h"
#include "thread_calls.h"

struct bwmonitor {
  long long t0;
  int len;
};

struct bwmonitor_counters {
  long long counter_bytes;
  long long counter_ms;
};

static struct bwmonitor_counters g_counters = { 0 };
static int monitor_thread = -1;

static void* APP_CC
bwmonitor_monitoring(void* arg);

mon_ptr APP_CC
bwmonitor_begin(int len)
{
  struct timeval tv;
  struct bwmonitor *m = malloc(sizeof(struct bwmonitor));

  if (monitor_thread == -1)
  {
    monitor_thread = tc_thread_create(bwmonitor_monitoring, NULL);
  }

  gettimeofday(&tv, NULL);
  m->len = len;
  m->t0 = tv.tv_sec * 1000 + tv.tv_usec / 1000;
  return m;
}

void APP_CC
bwmonitor_end(mon_ptr handle)
{
  struct timeval tv;
  struct bwmonitor *m = (struct bwmonitor*)handle;
  long long t1;

  gettimeofday(&tv, NULL);
  t1 = tv.tv_sec * 1000 + tv.tv_usec / 1000;

  g_counters.counter_bytes += (long long)m->len;
  g_counters.counter_ms += t1 - m->t0;
  free(m);
}

static void* APP_CC
bwmonitor_monitoring(void* arg)
{
  int old = 0;
  do {
    float bw = (g_counters.counter_bytes - old) * 1.0 / 5;
    g_writeln("send %lld bytes, bandwidth %f KB/s", g_counters.counter_bytes, bw / 1024);
    old = g_counters.counter_bytes;
    sleep(5);
  } while(1);
}
