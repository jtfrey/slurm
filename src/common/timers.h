/*****************************************************************************\
 *  timers.h - timing functions
 *****************************************************************************
 *  Copyright (C) 2002 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Morris Jette <jette1@llnl.gov> and Kevin Tew <tew1@llnl.gov>
 *  CODE-OCEC-09-009. All rights reserved.
 *
 *  This file is part of Slurm, a resource management program.
 *  For details, see <https://slurm.schedmd.com/>.
 *  Please also read the included file: DISCLAIMER.
 *
 *  Slurm is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  In addition, as a special exception, the copyright holders give permission
 *  to link the code of portions of this program with the OpenSSL library under
 *  certain conditions as described in each individual source file, and
 *  distribute linked combinations including the two. You must obey the GNU
 *  General Public License in all respects for all of the code used other than
 *  OpenSSL. If you modify file(s) with this exception, you may extend this
 *  exception to your version of the file(s), but you are not obligated to do
 *  so. If you do not wish to do so, delete this exception statement from your
 *  version.  If you delete this exception statement from all source files in
 *  the program, then also delete it here.
 *
 *  Slurm is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Slurm; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
\*****************************************************************************/

#ifndef _HAVE_TIMERS_H
#define _HAVE_TIMERS_H

#include <sys/time.h>

/*
 * slurm_timer_gettime - fill-in a timeval struct with the current
 *               monotonically increasing time (unaffected by clock
 *               changes/adjustments)
 * OUT tv - place t put current time value
 *
 * Returns zero on success, non-zero on failure.
 */
extern int slurm_timer_gettime(struct timeval *tv);

/*
 * slurm_timer_delta_tv - return the number of microseconds
 *           elapsed since the timer value in tv using the
 *           monotonically-increasing time source
 * INOUT tv - if tv.tv_sec is zero, set to the current time
 *           value on exit; otherwise, used as the starting
 *           time value
 */
extern int slurm_timer_delta_tv(struct timeval *tv);


/*
 * slurm_timer_diff_tv_str - build a string showing the time
 *             difference between two monotonically-increasing
 *             times
 * IN tv1 - start of event
 * IN tv2 - end of event
 * OUT tv_str - place to put delta time in format "usec=%ld"
 * IN len_tv_str - size of tv_str in bytes
 * IN from - where the function was called form
 */
extern void slurm_timer_diff_tv_str(struct timeval *tv1, struct timeval *tv2,
			      char *tv_str, int len_tv_str, const char *from, long limit,
                  long *delta_t);


#define DEF_TIMERS	struct timeval tv1, tv2; char tv_str[20] = ""; long delta_t;
#define START_TIMER	slurm_timer_gettime(&tv1)
#define ELAPSED_TIMER slurm_timer_delta_tv(&tv1)
#define END_TIMER	slurm_timer_gettime(&tv2); \
	slurm_timer_diff_tv_str(&tv1, &tv2, tv_str, sizeof(tv_str), NULL, 0, &delta_t)
#define END_TIMER2(from) slurm_timer_gettime(&tv2); \
	slurm_timer_diff_tv_str(&tv1, &tv2, tv_str, sizeof(tv_str), from, 0, &delta_t)
#define END_TIMER3(from, limit) slurm_timer_gettime(&tv2); \
	slurm_timer_diff_tv_str(&tv1, &tv2, tv_str, sizeof(tv_str), from, limit, &delta_t)
#define DELTA_TIMER	delta_t
#define TIME_STR 	tv_str

/* Return the number of micro-seconds between now and argument "tv",
 * Initialize tv to NOW if zero on entry */
extern int slurm_delta_tv(struct timeval *tv);

/*
 * slurm_diff_tv_str - build a string showing the time difference between two
 *		       times
 * IN tv1 - start of event
 * IN tv2 - end of event
 * OUT tv_str - place to put delta time in format "usec=%ld"
 * IN len_tv_str - size of tv_str in bytes
 * IN from - Name to be printed on long diffs
 * IN limit - limit to wait
 * OUT delta_t - raw time difference in usec
 */
extern void slurm_diff_tv_str(struct timeval *tv1,struct timeval *tv2,
			      char *tv_str, int len_tv_str, const char *from,
			      long limit, long *delta_t);

#endif
