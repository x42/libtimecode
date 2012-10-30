/*
   libtimecode - convert A/V timecode and framerate

   Copyright (C) 2006, 2007, 2008, 2012 Robin Gareus <robin@gareus.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "timecode/timecode.h"

#define TCtoDbl(r) ( (double)((r)->num) / (double)((r)->den) )

int64_t timecode_to_sample (TimecodeTime const * const t, TimecodeRate const * const r, const double samplerate) {
	const double fps_d = TCtoDbl(r);
	const int    fps_i = ceil(fps_d);
	const double frames_per_timecode_frame = samplerate / fps_d;
	int64_t sample;

	if (r->drop) {
		int64_t totalMinutes = 60 * t->hour + t->minute;
		int64_t frameNumber  = fps_i * 3600 * t->hour + fps_i * 60 * t->minute
			                 + fps_i * t->second + t->frame
											 - 2 * (totalMinutes - totalMinutes / 10);

		sample = frameNumber * samplerate / fps_d;
	} else {
		sample = (int64_t) rint(
				(
				   ((t->hour * 60 * 60) + (t->minute * 60) + t->second)
				 * (fps_i * frames_per_timecode_frame)
				)
				 + (t->frame * frames_per_timecode_frame));
	}
	if (r->subframes != 0) {
		sample += t->subframe * frames_per_timecode_frame / r->subframes;
	}
	return sample;
}

void timecode_sample_to_time (TimecodeTime * const t, TimecodeRate const * const r, const double samplerate, const int64_t sample) {
	const double fps_d = TCtoDbl(r);
	const int    fps_i = ceil(fps_d);
	if (r->drop) {
		int64_t frameNumber = floor( (double)sample * fps_d / samplerate );

		const int64_t D = frameNumber / 17982;
		const int64_t M = frameNumber % 17982;

		t->subframe =  r->subframes * ( (double)sample / samplerate - (double)frameNumber / fps_d );

		frameNumber +=  18*D + 2*((M - 2) / 1798);

		t->frame  =    frameNumber % 30;
		t->second =   (frameNumber / 30) % 60;
		t->minute =  ((frameNumber / 30) / 60) % 60;
		t->hour   = (((frameNumber / 30) / 60) / 60) % 24;

	} else {
		double timecode_frames_left_exact;
		double timecode_frames_fraction;
		int64_t timecode_frames_left;

		const double frames_per_timecode_frame = samplerate / fps_d;
		int32_t frames_per_hour = (int32_t)(3600 * fps_i * frames_per_timecode_frame);

		t->hour = sample / frames_per_hour;
		double sample_d = sample % frames_per_hour;

		timecode_frames_left_exact = sample_d / frames_per_timecode_frame;
		timecode_frames_fraction = timecode_frames_left_exact - floor( timecode_frames_left_exact );

		t->subframe = (int32_t) floor(timecode_frames_fraction * r->subframes);
		//assert (t->subframe < r->subframes);

		timecode_frames_left = (int64_t) floor (timecode_frames_left_exact);

		t->minute = timecode_frames_left / (fps_i * 60);
		timecode_frames_left = timecode_frames_left % (fps_i * 60);
		t->second = timecode_frames_left / fps_i;
		t->frame  = timecode_frames_left % fps_i;
	}
}


/* ------------- */

/** Drop-frame support function
 * skip the first two frame numbers (0 and 1) at the beginning of each minute,
 * except for minutes 0, 10, 20, 30, 40, and 50
 */
static void skip_drop_frames(TimecodeTime * const t) {
	if ( (t->minute %10 != 0) && (t->second == 0) && (t->frame == 0)) {
		t->frame += 2;
	}
}

void timecode_date_increment(TimecodeDate * const d) {
	unsigned char dpm[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	if ( (d->year%4)==0 && ( (d->year%100) != 0 || (d->year%400) == 0) ) {
		dpm[1]=29;
	}

	d->day++;
	if (d->day > dpm[d->month-1]) {
		d->day=1;
		d->month++;
		if (d->month > 12) {
			d->month = 1;
			d->year++;
		}
	}
}

int timecode_time_increment(TimecodeTime * const t, TimecodeRate const * const r) {
	int rv = 0;
	const int fps = ceil(TCtoDbl(r));
	t->frame++;

	if (fps == t->frame) {
		t->frame = 0;
		t->second++;
		if (t->second == 60) {
			t->second = 0;
			t->minute++;
			if (t->minute == 60) {
				t->minute = 0;
				t->hour++;
				if (t->hour==24) {
					/* 24h wrap around */
					rv=1;
					t->hour = 0;
				}
			}
		}
	}

	if (r->drop) {
		skip_drop_frames(t);
	}
	return rv;
}

void timecode_date_decrement (TimecodeDate * const d) {
	unsigned char dpm[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	if ( (d->year%4)==0 && ( (d->year%100) != 0 || (d->year%400) == 0) ) {
		dpm[1]=29;
	}

	if (d->day > 1) {
		d->day--;
		return;
	}
	d->month = 1 + (d->month + 10)%12;
	d->day = dpm[d->month-1];

	if (d->month == 12) {
			d->year--;
	}
}

int timecode_time_decrement(TimecodeTime * const t, TimecodeRate const * const r) {
	const int fps = ceil(TCtoDbl(r));
	if (t->frame > 0) {
		t->frame--;
		return 0;
	}

	t->frame = fps-1;
	if (t->second > 0) {
		t->second--;
		return 0;
	}

	t->second = 59;
	if (t->minute > 0) {
		t->minute--;
		return 0;
	}

	t->minute = 59;
	if (t->hour > 0) {
		t->hour--;
		return 0;
	}
	t->hour = 23;

	return 1;
}

int timecode_datetime_increment (Timecode * const dt, TimecodeRate const * const r) {
	if (timecode_time_increment(&dt->t, r)) {
		timecode_date_increment(&dt->d);
		return 1;
	}
	return 0;
}

int timecode_datetime_decrement (Timecode * const dt, TimecodeRate const * const r) {
	if (timecode_time_decrement(&dt->t, r)) {
		timecode_date_decrement(&dt->d);
		return 1;
	}
	return 0;
}

/* ------------- */

void timecode_time_to_string (TimecodeTime *t, char *smptestring) {
	snprintf(smptestring, 16, "%02d:%02d:%02d:%02d.%03d",
			t->hour, t->minute, t->second, t->frame, t->subframe);
}
