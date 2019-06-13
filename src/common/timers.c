/*****************************************************************************\
 *  timers.c - Timer functions
 *****************************************************************************
 *  Copyright (C) 2002-2006 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Morris Jette <jette@llnl.gov>
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

#include <stdio.h>
#include <sys/time.h>
#include <linux/version.h>
#include "src/common/log.h"
#include "src/common/slurm_time.h"

/*
 * slurm_timer_gettime - fill-in a timeval struct with the current
 *               monotonically increasing time (unaffected by clock
 *               changes/adjustments)
 * OUT tv - place t put current time value
 *
 * Returns zero on success, non-zero on failure.
 */
extern int slurm_timer_gettime(struct timeval *tv)
{
    struct timespec     cur_time;
    int                 rc = 1;
    
#if defined(__FreeBSD__)
    rc = clock_gettime(CLOCK_UPTIME, &cur_time);
#elif defined(__NetBSD__)
    rc = clock_gettime(CLOCK_MONOTONIC, &cur_time);
#elif defined(CLOCK_BOOTTIME)
    rc = clock_gettime(CLOCK_BOOTTIME, &cur_time);
#elif defined(CLOCK_MONOTONIC_RAW)
    rc = clock_gettime(CLOCK_MONOTONIC_RAW, &cur_time);
#elif defined(CLOCK_MONOTONIC)
    rc = clock_gettime(CLOCK_MONOTONIC, &cur_time);
#else
#error no monotonically increasing time source available
#endif
    if ( rc == 0 ) {
        tv->tv_sec = cur_time.tv_sec;
        tv->tv_usec = cur_time.tv_nsec / 1000;
    }
    return rc;
}


/*
 * slurm_timer_delta_tv - return the number of microseconds
 *           elapsed since the timer value in tv using the
 *           monotonically-increasing time source
 * INOUT tv - if tv.tv_sec is zero, set to the current time
 *           value on exit; otherwise, used as the starting
 *           time value
 */
extern int slurm_timer_delta_tv(struct timeval *tv)
{
	struct timeval now = {0, 0};
	int delta_t;

	if (slurm_timer_gettime(&now))
		return 1;		/* Some error */

	if (tv->tv_sec == 0) {
		tv->tv_sec  = now.tv_sec;
		tv->tv_usec = now.tv_usec;
		return 0;
	}

	delta_t  = (now.tv_sec - tv->tv_sec) * 1000000;
	delta_t += (now.tv_usec - tv->tv_usec);

	return delta_t;
}

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
                  long *delta_t)
{
	char p[64] = "";
	struct tm tm;
    time_t realtime;
	int debug_limit = limit;

	(*delta_t)  = (tv2->tv_sec - tv1->tv_sec) * 1000000;
	(*delta_t) += tv2->tv_usec;
	(*delta_t) -= tv1->tv_usec;
	snprintf(tv_str, len_tv_str, "usec=%ld", *delta_t);
	if (from) {
		if (!limit) {
			/* NOTE: The slurmctld scheduler's default run time
			 * limit is 4 seconds, but that would not typically
			 * be reached. See "max_sched_time=" logic in
			 * src/slurmctld/job_scheduler.c */
			limit = 3000000;
			debug_limit = 1000000;
		}
		if ((*delta_t > debug_limit) || (*delta_t > limit)) {
            /* NOTE: A monotonically-increasing time source has no
             * direct correlation with the real time, so let's get
             * the current real time, subtract off delta_t and display
             * that as the time of day. */
            realtime = time(NULL) - *delta_t;
			if (!slurm_localtime_r(&realtime, &tm))
				error("localtime_r(): %m");
			if (strftime(p, sizeof(p), "%T", &tm) == 0)
				error("strftime(): %m");
			if (*delta_t > limit) {
				verbose("Warning: Note very large processing "
					"time from %s: %s began=%s",
					from, tv_str, p);
			} else {	/* Log anything over 1 second here */
				debug("Note large processing time from %s: "
				      "%s began=%s",
				      from, tv_str, p);
			}
		}
	}
}


/* Return the number of micro-seconds between now and argument "tv",
 * Initialize tv to NOW if zero on entry */
extern int slurm_delta_tv(struct timeval *tv)
{
	struct timeval now = {0, 0};
	int delta_t;

	if (gettimeofday(&now, NULL))
		return 1;		/* Some error */

	if (tv->tv_sec == 0) {
		tv->tv_sec  = now.tv_sec;
		tv->tv_usec = now.tv_usec;
		return 0;
	}

	delta_t  = (now.tv_sec - tv->tv_sec) * 1000000;
	delta_t += (now.tv_usec - tv->tv_usec);

	return delta_t;
}

/*
 * slurm_diff_tv_str - build a string showing the time difference between two
 *		       times
 * IN tv1 - start of event
 * IN tv2 - end of event
 * OUT tv_str - place to put delta time in format "usec=%ld"
 * IN len_tv_str - size of tv_str in bytes
 * IN from - where the function was called form
 */
extern void slurm_diff_tv_str(struct timeval *tv1, struct timeval *tv2,
			      char *tv_str, int len_tv_str, const char *from,
			      long limit, long *delta_t)
{
	char p[64] = "";
	struct tm tm;
	int debug_limit = limit;

	(*delta_t)  = (tv2->tv_sec - tv1->tv_sec) * 1000000;
	(*delta_t) += tv2->tv_usec;
	(*delta_t) -= tv1->tv_usec;
	snprintf(tv_str, len_tv_str, "usec=%ld", *delta_t);
	if (from) {
		if (!limit) {
			/* NOTE: The slurmctld scheduler's default run time
			 * limit is 4 seconds, but that would not typically
			 * be reached. See "max_sched_time=" logic in
			 * src/slurmctld/job_scheduler.c */
			limit = 3000000;
			debug_limit = 1000000;
		}
		if ((*delta_t > debug_limit) || (*delta_t > limit)) {
			if (!slurm_localtime_r(&tv1->tv_sec, &tm))
				error("localtime_r(): %m");
			if (strftime(p, sizeof(p), "%T", &tm) == 0)
				error("strftime(): %m");
			if (*delta_t > limit) {
				verbose("Warning: Note very large processing "
					"time from %s: %s began=%s.%3.3d",
					from, tv_str, p,
					(int)(tv1->tv_usec / 1000));
			} else {	/* Log anything over 1 second here */
				debug("Note large processing time from %s: "
				      "%s began=%s.%3.3d",
				      from, tv_str, p,
				      (int)(tv1->tv_usec / 1000));
			}
		}
	}
}
