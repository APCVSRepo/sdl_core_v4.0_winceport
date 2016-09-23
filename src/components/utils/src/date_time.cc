/*
 * Copyright (c) 2015, Ford Motor Company
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ford Motor Company nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "utils/date_time.h"

#if defined(OS_WIN32) || defined(OS_WINCE)
#include <time.h>
#include <assert.h>
#include "os/poll_windows.h"
#elif defined(OS_MAC)
#else
#include <sys/time.h>
#endif
#include <stdint.h>

#if defined(OS_WINCE)
#include "time_ext.h"
#endif

#if defined(OS_WIN32)||defined(OS_WINCE)
static uint64_t win32_system_time_ = time(NULL);  //wince start time ,unit :s
#endif

#if defined(OS_WINCE)
static time_t time(time_t* TimeOutPtr)  
{
	SYSTEMTIME SytemTime;
	FILETIME FileTime;
	time_t Time = 0;

	GetSystemTime( &SytemTime );
	if( SystemTimeToFileTime( &SytemTime, &FileTime ) )
	{
		memcpy( &Time, &FileTime, sizeof( FILETIME ) );
		// subtract the FILETIME value for 1970-01-01 00:00 (UTC)
		Time -= 116444736000000000;
		// convert to seconds
		Time /= 10000000;
	}
	if( TimeOutPtr )
	{
		*TimeOutPtr = Time;
	}
	return Time;
}
#endif

#if defined(OS_WIN32) || defined(OS_WINCE)
void clock_gettime(int i, timespec * tm)
{
	if (i == CLOCK_MONOTONIC) {
          unsigned __int64 cur = GetTickCount();
          tm->tv_sec = cur / 1000;
          tm->tv_nsec = (cur % 1000) * 1000000;
	}
	else if (i == CLOCK_REALTIME) {
          uint64_t curr = clock();
          // Finally change microseconds to seconds and place in the seconds value.
          // The modulus picks up the microseconds.
          tm->tv_sec = static_cast<long>(win32_system_time_ + curr / 1000);
          tm->tv_nsec = static_cast<long>((curr % 1000) * 1000000);
	}
    else {
        assert(false);
    }
}
#elif defined(OS_MAC)
void clock_gettime(int i, timespec * tm)
{
	clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), i, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    tm->tv_sec = mts.tv_sec;
    tm->tv_nsec = mts.tv_nsec;
}
#endif

namespace date_time {

#if defined(OS_WIN32)|| defined(OS_WINCE)

TimevalStruct DateTime::getCurrentTime() {
  TimevalStruct tv;
  const uint32_t kMillisecondsInSecond = 1000u;
  uint64_t curr=clock();
  // Finally change microseconds to seconds and place in the seconds value.
  // The modulus picks up the microseconds.
  tv.tv_sec = static_cast<long>(win32_system_time_+curr/kMillisecondsInSecond);
  tv.tv_usec = static_cast<long>((curr % kMillisecondsInSecond)*1000);
  return tv;
}
#else
TimevalStruct DateTime::getCurrentTime() {
  TimevalStruct currentTime;

#ifdef OS_MAC
    struct timezone timeZone;
#else
  timezone timeZone;
#endif
  gettimeofday(&currentTime, &timeZone);

  return currentTime;
}
#endif

int64_t date_time::DateTime::getSecs(const TimevalStruct &time) {
   const TimevalStruct times = ConvertionUsecs(time);
   return static_cast<int64_t>(times.tv_sec);
}

int64_t DateTime::getmSecs(const TimevalStruct &time) {
  const TimevalStruct times = ConvertionUsecs(time);
  return static_cast<int64_t>(times.tv_sec) * MILLISECONDS_IN_SECOND
      + times.tv_usec / MICROSECONDS_IN_MILLISECOND;
}

int64_t DateTime::getuSecs(const TimevalStruct &time) {
  const TimevalStruct times = ConvertionUsecs(time);
  return static_cast<int64_t>(times.tv_sec) * MILLISECONDS_IN_SECOND
      * MICROSECONDS_IN_MILLISECOND + times.tv_usec;
}

int64_t DateTime::calculateTimeSpan(const TimevalStruct& sinceTime) {
  return calculateTimeDiff(getCurrentTime(), sinceTime);
}

int64_t DateTime::calculateTimeDiff(const TimevalStruct &time1,
                                    const TimevalStruct &time2) {
  const TimevalStruct times1 = ConvertionUsecs(time1);
  const TimevalStruct times2 = ConvertionUsecs(time2);
  TimevalStruct ret;
  if (Greater(times1, times2)) {
    ret = Sub(times1, times2);
  } else {
    ret = Sub(times2, times1);
  }
  return getmSecs(ret);
}

void DateTime::AddMilliseconds(TimevalStruct& time,
                             uint32_t milliseconds) {
  const uint32_t sec = milliseconds/MILLISECONDS_IN_SECOND;
  const uint32_t usec = (milliseconds%MILLISECONDS_IN_SECOND)*MICROSECONDS_IN_MILLISECOND;
  time.tv_sec += sec;
  time.tv_usec += usec;
  time = ConvertionUsecs(time);
}

TimevalStruct DateTime::Sub(const TimevalStruct& time1,
                            const TimevalStruct& time2) {
  const TimevalStruct times1 = ConvertionUsecs(time1);
  const TimevalStruct times2 = ConvertionUsecs(time2);
  TimevalStruct ret;
  timersub(&times1, &times2, &ret);
  return ret;
}

bool DateTime::Greater(const TimevalStruct& time1, const TimevalStruct& time2) {
  const TimevalStruct times1 = ConvertionUsecs(time1);
  const TimevalStruct times2 = ConvertionUsecs(time2);
  return timercmp(&times1, &times2, >);
}

bool DateTime::Less(const TimevalStruct& time1, const TimevalStruct& time2) {
  const TimevalStruct times1 = ConvertionUsecs(time1);
  const TimevalStruct times2 = ConvertionUsecs(time2);
  return timercmp(&times1, &times2, <);
}

bool DateTime::Equal(const TimevalStruct& time1, const TimevalStruct& time2) {
  const TimevalStruct times1 = ConvertionUsecs(time1);
  const TimevalStruct times2 = ConvertionUsecs(time2);
  return !timercmp(&times1, &times2, !=);
}

TimeCompare date_time::DateTime::compareTime(const TimevalStruct &time1, const TimevalStruct &time2) {
  if (Greater(time1, time2)) return GREATER;
  if (Less(time1, time2)) return LESS;
  return EQUAL;
}

TimevalStruct date_time::DateTime::ConvertionUsecs(const TimevalStruct &time) {
  if (time.tv_usec >= MICROSECONDS_IN_SECOND) {
    TimevalStruct time1;
    time1.tv_sec = static_cast<int64_t>(time.tv_sec) + (time.tv_usec/MICROSECONDS_IN_SECOND);
    time1.tv_usec = static_cast<int64_t>(time.tv_usec) % MICROSECONDS_IN_SECOND;
    return time1;
  }
  return time;
}

}  // namespace date_time

bool operator<(const TimevalStruct& time1, const TimevalStruct& time2) {
  return date_time::DateTime::Less(time1, time2);
}

bool operator==(const TimevalStruct& time1, const TimevalStruct& time2) {
  return date_time::DateTime::Equal(time1, time2);
}

const TimevalStruct operator-(const TimevalStruct& time1, const TimevalStruct& time2) {
    return date_time::DateTime::Sub(time1, time2);
}
