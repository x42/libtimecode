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

double timecode_rate_to_double(TimecodeRate const * const r) {
	return TCtoDbl(r);
}

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

		t->subframe =  rint(r->subframes * ((double)sample * fps_d / samplerate - (double)frameNumber ));

		if (t->subframe == r->subframes) {
			t->subframe = 0;
			frameNumber++;
		}

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

/*****************************************************************************
 * Add Subtract
 */

static int32_t move_overflow(TimecodeTime * const t, TimecodeRate const * const r) {
	int i;
	int32_t rv = 0;
	int32_t * const bcd[6] = {&t->subframe, &t->frame, &t->second, &t->minute, &t->hour, &rv };

	int smpte_table[6] =  {1, 1, 60, 60, 24, 0 };
	smpte_table[0] = r->subframes;
	smpte_table[1] = ceil(TCtoDbl(r));

	for (i=0; i<5; i++) {
		if ((*bcd[i] >= smpte_table[i]) || (*bcd[i] < 0) ) {
			int ov= (int) floor((double) (*bcd[i]) / smpte_table[i]);
#if 0
			// TODO drop-frames ? - basically only needed when parsing invalid TC
			// via timecode_parse_time()
			if (r->drop && i==2) minutes_crossed += ov;
			if (r->drop && i==3) minutes_crossed += ov * 60;
			//if (r->drop && i==4) minutes_crossed += ov * 60 * 24;
#endif
			*bcd[i] -= ov*(smpte_table[i]);
			if (bcd[i+1]) { *bcd[i+1]+=ov; }
		}
	}

#if 0
	if (r->drop) {
		// 108 drop-frames per hour, but..
	}
#endif
	return rv;
}

static int32_t dropped_frames(TimecodeTime const * const t) {
		int64_t totalMinutes = 60 * t->hour + t->minute;
		return 2 * (totalMinutes - totalMinutes / 10);
}

void timecode_time_add (TimecodeTime * const res, TimecodeRate const * const r, TimecodeTime const * const t1, TimecodeTime const * const t2) {
	int df = 0;
	if (r->drop) {
		df = dropped_frames(t1) + dropped_frames(t2);
	}

	res->subframe = t1->subframe + t2->subframe;
	res->frame    = t1->frame    + t2->frame   ;
	res->second   = t1->second   + t2->second  ;
	res->minute   = t1->minute   + t2->minute  ;
	res->hour     = t1->hour     + t2->hour    ;

	if (r->drop) {
		move_overflow(res, r);
		res->frame += dropped_frames(res) - df;
	}

	move_overflow(res, r);
}

void timecode_time_subtract (TimecodeTime * const res, TimecodeRate const * const r, TimecodeTime const * const t1, TimecodeTime const * const t2) {
	int df = 0;
	if (r->drop) {
		df = dropped_frames(t1) - dropped_frames(t2);
	}

	res->subframe = t1->subframe - t2->subframe;
	res->frame    = t1->frame    - t2->frame   ;
	res->second   = t1->second   - t2->second  ;
	res->minute   = t1->minute   - t2->minute  ;
	res->hour     = t1->hour     - t2->hour    ;

	if (r->drop) {
		move_overflow(res, r);
		res->frame += dropped_frames(res) - df;
	}

	move_overflow(res, r);
}

#define CMP(a,b) ( (a) > (b) ? 1 : -1)
int timecode_time_compare (TimecodeRate const * const r, TimecodeTime const * const a, TimecodeTime const * const b) {
	if (a->hour     != b->hour    ) return CMP(a->hour, b->hour);
	if (a->minute   != b->minute  ) return CMP(a->minute, b->minute);
	if (a->second   != b->second  ) return CMP(a->second, b->second);
	if (a->frame    != b->frame   ) return CMP(a->frame, b->frame);
	if (a->subframe != b->subframe) return CMP(a->subframe, b->subframe);
	return (0);
}

int timecode_date_compare (TimecodeDate const * const a, TimecodeDate const * const b) {
	if (a->year  != b->year ) return CMP(a->year, b->year);
	if (a->month != b->month) return CMP(a->month, b->month);
	if (a->day   != b->day  ) return CMP(a->day, b->day);
	// TODO timezone  -- check intl. date-line crossing.
	if (a->timezone != b->timezone) return CMP(a->timezone, b->timezone);
	return (0);
}

int timecode_datetime_compare (TimecodeRate const * const r, Timecode const * const a, Timecode const * const b) {
	Timecode ax, bx;
	memcpy(&ax, a, sizeof(Timecode));
	memcpy(&bx, b, sizeof(Timecode));

	/* convert to UTC */
	ax.t.minute -= a->d.timezone;
	bx.t.minute -= b->d.timezone;
	ax.d.timezone = bx.d.timezone = 0;

	/* adjust day, month */
	int ao = move_overflow(&ax.t, r);
	int bo = move_overflow(&bx.t, r);

	if (ao < 0) {
		int i;
		for (i=0; i > ao; --i) {
			timecode_date_decrement(&ax.d);
		}
	} else if (ao > 0) {
		int i;
		for (i=0; i < ao; ++i) {
			timecode_date_increment(&ax.d);
		}
	}

	if (bo < 0) {
		int i;
		for (i=0; i > bo; --i) {
			timecode_date_decrement(&bx.d);
		}
	} else if (bo > 0) {
		int i;
		for (i=0; i < bo; ++i) {
			timecode_date_increment(&bx.d);
		}
	}

	int rv;
	if ((rv=timecode_date_compare(&ax.d, &bx.d)) != 0)
		return rv;

	return (timecode_time_compare(r, &ax.t, &bx.t));
}

/*****************************************************************************
 * Increment/Decrement
 */

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

	if (fps < t->frame) goto done;
	t->frame = 0;
	t->second++;
	if (t->second < 60) goto done;
	t->second = 0;
	t->minute++;
	if (t->minute < 60) goto done;
	t->minute = 0;
	t->hour++;
	if (t->hour < 24) goto done;
	t->hour = 0;
	rv=1;

done:
	if (r->drop && (t->minute%10 != 0) && (t->second == 0) && (t->frame == 0)) {
		t->frame += 2;
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

	if (r->drop && (t->minute%10 != 0) && (t->second == 0) && (t->frame == 2)) {
		; // assume t->frame==0;
	} else

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

/*****************************************************************************
 * Format & Parse
 */

void timecode_datetime_to_string (Timecode const * const tc, char *smptestring) {
	snprintf(smptestring, 33, "%02d/%02d/%04d %02d:%02d:%02d:%02d.%03d %+03d%02d",
			tc->d.month, tc->d.day, tc->d.year,
			tc->t.hour, tc->t.minute, tc->t.second, tc->t.frame, tc->t.subframe,
			tc->d.timezone/60, abs(tc->d.timezone)%60
			);
}

void timecode_time_to_string (TimecodeTime const * const t, char *smptestring) {
	snprintf(smptestring, 16, "%02d:%02d:%02d:%02d.%03d",
			t->hour, t->minute, t->second, t->frame, t->subframe);
}

static const char *strrpbrk(const char * const haystack, const char * const needle) {
	const char *ph, *pn, *rv;
	if (!haystack || !needle) return NULL;
	for (ph = haystack, rv = NULL ; *ph ; ++ph) {
		for (pn = needle; *pn != '\0' ; ++pn) {
			if (*pn == *ph) {
				rv = ph;
				break;
			}
		}
	}
	return rv;
}

void timecode_parse_time (TimecodeTime * const t, TimecodeRate const * const r, const char *val) {
	int i = 0;
	char *buf = strdup(val);
	char *pe;
	int32_t * const bcd[5] = {&t->frame, &t->second, &t->minute, &t->hour, NULL };

	t->subframe = 0;
	for (i=0; i<4; i++) {
		*bcd[i] = 0;
	}

	i=0;
	while (i < 4 && (pe= (char*) strrpbrk(buf,":.;"))) {
		*bcd[i] = (int) atoi(pe+1);
		*pe = '\0';
		i++;
	}

	if (i < 4 && *buf) {
		*bcd[i]= (int) atoi(buf);
	}

	free(buf);

	move_overflow(t, r);

	if (r->drop && (t->minute%10 != 0) && (t->second == 0) && (t->frame == 0)) {
		t->frame=2;
	}
}
