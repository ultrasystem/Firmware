/****************************************************************************
 *
 *   Copyright (C) 2015 Mark Charlebois. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file px4_log.h
 * Platform dependant logging/debug implementation
 */

#pragma once

#if defined(__PX4_ROS)

#include <ros/console.h>
#define PX4_PANIC(...)	ROS_WARN(__VA_ARGS__)
#define PX4_ERR(...)	ROS_WARN(__VA_ARGS__)
#define PX4_WARN(...) 	ROS_WARN(__VA_ARGS__)
#define PX4_INFO(...) 	ROS_WARN(__VA_ARGS__)
#define PX4_DEBUG(...) 	

#else

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <stdio.h>
#include <px4_defines.h>

__BEGIN_DECLS
__EXPORT extern uint64_t hrt_absolute_time(void);

// Used to silence unused variable warning
static inline void do_nothing(int level, ...)
{
	(void)level;
}

#define _PX4_LOG_LEVEL_ALWAYS		0
#define _PX4_LOG_LEVEL_PANIC		1
#define _PX4_LOG_LEVEL_ERROR		2
#define _PX4_LOG_LEVEL_WARN		3
#define _PX4_LOG_LEVEL_DEBUG		4

__EXPORT extern const char *__px4_log_level_str[5];
__EXPORT extern int __px4_log_level_current;

// __px4_log_level_current will be initialized to PX4_LOG_LEVEL_AT_RUN_TIME
#define PX4_LOG_LEVEL_AT_RUN_TIME	_PX4_LOG_LEVEL_WARN

/****************************************************************************
 * Implementation of log section formatting based on printf
 *
 * To write to a specific stream for each message type, open the streams and
 * set __px4__log_startline to something like:
 * 	if (level <= __px4_log_level_current) printf(_px4_fd[level], 
 *
 * Additional behavior can be added using "{\" for __px4__log_startline and
 * "}" for __px4__log_endline and any other required setup or teardown steps
 ****************************************************************************/
#define __px4__log_startline(level)	if (level <= __px4_log_level_current) printf(

#define __px4__log_timestamp_fmt	"%-10" PRIu64 " "
#define __px4__log_timestamp_arg 	,hrt_absolute_time()
#define __px4__log_level_fmt		"%-5s "
#define __px4__log_level_arg(level)	,__px4_log_level_str[level]
#define __px4__log_thread_fmt		"%#X "
#define __px4__log_thread_arg		,(unsigned int)pthread_self()

#define __px4__log_file_and_line_fmt 	" (file %s line %u)"
#define __px4__log_file_and_line_arg 	, __FILE__, __LINE__
#define __px4__log_end_fmt 		"\n"
#define __px4__log_endline 		)

/****************************************************************************
 * Output format macros
 * Use these to implement the code level macros below
 ****************************************************************************/

/****************************************************************************
 * __px4_log_omit:
 * Compile out the message
 ****************************************************************************/
#define __px4_log_omit(level, FMT, ...)   do_nothing(level, ##__VA_ARGS__)

/****************************************************************************
 * __px4_log:
 * Convert a message in the form:
 * 	PX4_WARN("val is %d", val);
 * to
 * 	printf("%-5s val is %d\n", __px4_log_level_str[3], val);
 ****************************************************************************/
#define __px4_log(level, FMT, ...) \
	__px4__log_startline(level)\
	__px4__log_level_fmt \
	FMT\
	__px4__log_end_fmt \
	__px4__log_level_arg(level), ##__VA_ARGS__\
	__px4__log_endline

/****************************************************************************
 * __px4_log_timestamp:
 * Convert a message in the form:
 * 	PX4_WARN("val is %d", val);
 * to
 * 	printf("%-5s %10lu val is %d\n", __px4_log_level_str[3],
 *		hrt_absolute_time(), val);
 ****************************************************************************/
#define __px4_log_timestamp(level, FMT, ...) \
	__px4__log_startline(level)\
	__px4__log_level_fmt\
	__px4__log_timestamp_fmt\
	FMT\
	__px4__log_end_fmt\
	__px4__log_level_arg(level)\
	__px4__log_timestamp_arg\
	, ##__VA_ARGS__\
	__px4__log_endline

/****************************************************************************
 * __px4_log_timestamp_thread:
 * Convert a message in the form:
 * 	PX4_WARN("val is %d", val);
 * to
 * 	printf("%-5s %10lu %#X val is %d\n", __px4_log_level_str[3],
 *		hrt_absolute_time(), pthread_self(), val);
 ****************************************************************************/
#define __px4_log_timestamp_thread(level, FMT, ...) \
	__px4__log_startline(level)\
	__px4__log_level_fmt\
	__px4__log_timestamp_fmt\
	__px4__log_thread_fmt\
	FMT\
	__px4__log_end_fmt\
	__px4__log_level_arg(level)\
	__px4__log_timestamp_arg\
	__px4__log_thread_arg\
	, ##__VA_ARGS__\
	__px4__log_endline

/****************************************************************************
 * __px4_log_file_and_line:
 * Convert a message in the form:
 * 	PX4_WARN("val is %d", val);
 * to
 * 	printf("%-5s val is %d (file %s line %u)\n", 
 *		__px4_log_level_str[3], val, __FILE__, __LINE__);
 ****************************************************************************/
#define __px4_log_file_and_line(level, FMT, ...) \
	__px4__log_startline(level)\
	__px4__log_level_fmt\
	__px4__log_timestamp_fmt\
	FMT\
	__px4__log_file_and_line_fmt\
	__px4__log_end_fmt\
	__px4__log_level_arg(level)\
	__px4__log_timestamp_arg\
	, ##__VA_ARGS__\
	__px4__log_file_and_line_arg\
	__px4__log_endline

/****************************************************************************
 * __px4_log_timestamp_file_and_line:
 * Convert a message in the form:
 * 	PX4_WARN("val is %d", val);
 * to
 * 	printf("%-5s %-10lu val is %d (file %s line %u)\n", 
 *		__px4_log_level_str[3], hrt_absolute_time(),
 *		val, __FILE__, __LINE__);
 ****************************************************************************/
#define __px4_log_timestamp_file_and_line(level, FMT, ...) \
	__px4__log_startline(level)\
	__px4__log_level_fmt\
	__px4__log_timestamp_fmt\
	FMT\
	__px4__log_file_and_line_fmt\
	__px4__log_end_fmt\
	__px4__log_level_arg(level)\
	__px4__log_timestamp_arg\
	, ##__VA_ARGS__\
	__px4__log_file_and_line_arg\
	__px4__log_endline

/****************************************************************************
 * __px4_log_thread_file_and_line:
 * Convert a message in the form:
 * 	PX4_WARN("val is %d", val);
 * to
 * 	printf("%-5s %#X val is %d (file %s line %u)\n", 
 *		__px4_log_level_str[3], pthread_self(), 
 *		val, __FILE__, __LINE__);
 ****************************************************************************/
#define __px4_log_thread_file_and_line(level, FMT, ...) \
	__px4__log_startline(level)\
	__px4__log_level_fmt\
	__px4__log_thread_fmt\
	FMT\
	__px4__log_file_and_line_fmt\
	__px4__log_end_fmt\
	__px4__log_level_arg(level)\
	__px4__log_thread_arg\
	, ##__VA_ARGS__\
	__px4__log_file_and_line_arg\
	__px4__log_endline

/****************************************************************************
 * __px4_log_timestamp_thread_file_and_line:
 * Convert a message in the form:
 * 	PX4_WARN("val is %d", val);
 * to
 * 	printf("%-5s %-10lu %#X val is %d (file %s line %u)\n", 
 *		__px4_log_level_str[3], hrt_absolute_time(), 
 *		pthread_self(), val, __FILE__, __LINE__);
 ****************************************************************************/
#define __px4_log_timestamp_thread_file_and_line(level, FMT, ...) \
	__px4__log_startline(level)\
	__px4__log_level_fmt\
	__px4__log_timestamp_fmt\
	__px4__log_thread_fmt\
	FMT\
	__px4__log_file_and_line_fmt\
	__px4__log_end_fmt\
	__px4__log_level_arg(level)\
	__px4__log_timestamp_arg\
	__px4__log_thread_arg\
	, ##__VA_ARGS__\
	__px4__log_file_and_line_arg\
	__px4__log_endline


/****************************************************************************
 * Code level macros
 * These are the log APIs that should be used by the code
 ****************************************************************************/

/****************************************************************************
 * Messages that should never be filtered or compiled out
 ****************************************************************************/
#define PX4_LOG(FMT, ...) 	__px4_log(_PX4_LOG_LEVEL_ALWAYS, FMT, ##__VA_ARGS__)
#define PX4_INFO(FMT, ...) 	__px4_log(_PX4_LOG_LEVEL_ALWAYS, FMT, ##__VA_ARGS__)

#if defined(TRACE_BUILD)
/****************************************************************************
 * Extremely Verbose settings for a Trace build
 ****************************************************************************/
#define PX4_PANIC(FMT, ...)	__px4_log_timestamp_thread_file_and_line(_PX4_LOG_LEVEL_PANIC, FMT, ##__VA_ARGS__)
#define PX4_ERR(FMT, ...)	__px4_log_timestamp_thread_file_and_line(_PX4_LOG_LEVEL_ERROR, FMT, ##__VA_ARGS__)
#define PX4_WARN(FMT, ...) 	__px4_log_timestamp_thread_file_and_line(_PX4_LOG_LEVEL_WARN,  FMT, ##__VA_ARGS__)
#define PX4_DEBUG(FMT, ...) 	__px4_log_timestamp_thread(_PX4_LOG_LEVEL_DEBUG, FMT, ##__VA_ARGS__)

#elif defined(DEBUG_BUILD)
/****************************************************************************
 * Verbose settings for a Debug build
 ****************************************************************************/
#define PX4_PANIC(FMT, ...)	__px4_log_timestamp_file_and_line(_PX4_LOG_LEVEL_PANIC, FMT, ##__VA_ARGS__)
#define PX4_ERR(FMT, ...)	__px4_log_timestamp_file_and_line(_PX4_LOG_LEVEL_ERROR, FMT, ##__VA_ARGS__)
#define PX4_WARN(FMT, ...) 	__px4_log_timestamp_file_and_line(_PX4_LOG_LEVEL_WARN,  FMT, ##__VA_ARGS__)
#define PX4_DEBUG(FMT, ...) 	__px4_log_timestamp(_PX4_LOG_LEVEL_DEBUG, FMT, ##__VA_ARGS__)

#elif defined(RELEASE_BUILD)
/****************************************************************************
 * Non-verbose settings for a Release build to minimize strings in build
 ****************************************************************************/
#define PX4_PANIC(FMT, ...)	__px4_log_file_and_line(_PX4_LOG_LEVEL_PANIC, FMT, ##__VA_ARGS__)
#define PX4_ERR(FMT, ...)	__px4_log_file_and_line(_PX4_LOG_LEVEL_ERROR, FMT, ##__VA_ARGS__)
#define PX4_WARN(FMT, ...) 	__px4_log_omit(_PX4_LOG_LEVEL_WARN,  FMT, ##__VA_ARGS__)
#define PX4_DEBUG(FMT, ...) 	__px4_log_omit(_PX4_LOG_LEVEL_DEBUG, FMT, ##__VA_ARGS__)

#else
/****************************************************************************
 * Medium verbose settings for a default build
 ****************************************************************************/
#define PX4_PANIC(FMT, ...)	__px4_log_file_and_line(_PX4_LOG_LEVEL_PANIC, FMT, ##__VA_ARGS__)
#define PX4_ERR(FMT, ...)	__px4_log_file_and_line(_PX4_LOG_LEVEL_ERROR, FMT, ##__VA_ARGS__)
#define PX4_WARN(FMT, ...) 	__px4_log_file_and_line(_PX4_LOG_LEVEL_WARN,  FMT, ##__VA_ARGS__)
#define PX4_DEBUG(FMT, ...) 	__px4_log_omit(_PX4_LOG_LEVEL_DEBUG, FMT, ##__VA_ARGS__)

#endif
__END_DECLS
#endif
