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

/*****************************************************************************
 * misc & helper functions
 */

#define TCtoDbl(r) ( (double)((r)->num) / (double)((r)->den) )

double timecode_rate_to_double(TimecodeRate const * const r) {
	return TCtoDbl(r);
}

double timecode_frames_per_timecode_frame(TimecodeRate const * const r, const double samplerate) {
	return (samplerate / TCtoDbl(r));
}

/*****************************************************************************
 * timecode <> sample,frame-number
 */

int64_t timecode_to_sample (TimecodeTime const * const t, TimecodeRate const * const r, const double samplerate) {
	const double  fps_d = TCtoDbl(r);
	const int64_t fps_i = ceil(fps_d);
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
	const double  fps_d = TCtoDbl(r);
	const int64_t fps_i = ceil(fps_d);

	if (r->drop) {
		int64_t frameNumber = floor( (double)sample * fps_d / samplerate );

		/* there are 17982 frames in 10 min @ 29.97df */
		const int64_t D = frameNumber / 17982;
		const int64_t M = frameNumber % 17982;

		t->subframe =  rint(r->subframes * ((double)sample * fps_d / samplerate - (double)frameNumber));

		if (t->subframe == r->subframes) {
			t->subframe = 0;
			frameNumber++;
		}

		frameNumber +=  18*D + 2*((M - 2) / 1798);

		t->frame  =    frameNumber % 30;
		t->second =   (frameNumber / 30) % 60;
		t->minute =  ((frameNumber / 30) / 60) % 60;
		t->hour   = (((frameNumber / 30) / 60) / 60);

	} else {
		double timecode_frames_left_exact;
		double timecode_frames_fraction;
		int64_t timecode_frames_left;
		const double frames_per_timecode_frame = samplerate / fps_d;
		const int64_t frames_per_hour = (int64_t)(3600 * fps_i * frames_per_timecode_frame);

		t->hour = sample / frames_per_hour;
		double sample_d = sample % frames_per_hour;

		timecode_frames_left_exact = sample_d / frames_per_timecode_frame;
		timecode_frames_fraction = timecode_frames_left_exact - floor( timecode_frames_left_exact );

		t->subframe = (int32_t) rint(timecode_frames_fraction * r->subframes);

		timecode_frames_left = (int64_t) floor (timecode_frames_left_exact);

		if (t->subframe == r->subframes) {
			t->subframe = 0;
			timecode_frames_left++;
		}

		t->minute = timecode_frames_left / (fps_i * 60);
		timecode_frames_left = timecode_frames_left % (fps_i * 60);
		t->second = timecode_frames_left / fps_i;
		t->frame  = timecode_frames_left % fps_i;
	}
}

int64_t timecode_to_framenumber (TimecodeTime const * const t, TimecodeRate const * const r) {
	return timecode_to_sample(t, r, TCtoDbl(r));
}

void timecode_framenumber_to_time (TimecodeTime * const t, TimecodeRate const * const r, const int64_t frameno) {
	timecode_sample_to_time(t, r, TCtoDbl(r), frameno);
}

void timecode_convert_rate (TimecodeTime * const t_out, TimecodeRate const * const r_out, TimecodeTime * const t_in, TimecodeRate const * const r_in) {
	//const double rate = 84672000; // LCM(192k, 88.2k, 24, 25, 30)
	//const double rate = TCtoDbl(r_out) < TCtoDbl(r_in) ? (TCtoDbl(r_in) * r_in->subframes) : (TCtoDbl(r_out) * r_out->subframes);
	const double rate = TCtoDbl(r_out) < TCtoDbl(r_in) ?  TCtoDbl(r_in) :  TCtoDbl(r_out);
	int64_t s = timecode_to_sample(t_in, r_in, rate);
	timecode_sample_to_time(t_out, r_out, rate, s);
}

/*****************************************************************************
 * float seconds
 */

double timecode_sample_to_seconds (const int64_t sample, double samplerate) {
	return (double)sample / samplerate;
}

int64_t timecode_seconds_to_sample (const double sec, double samplerate) {
	return rint(sec * samplerate);
}

double timecode_framenumber_to_seconds (const int64_t frameno, TimecodeRate const * const r) {
	return (double)frameno / TCtoDbl(r);
}

int64_t timecode_seconds_to_framenumber (const double sec, TimecodeRate const * const r) {
	return floor(sec * TCtoDbl(r));
}

void timecode_seconds_to_time (TimecodeTime * const t, TimecodeRate const * const r, const double sec) {
	const double rate = TCtoDbl(r) * r->subframes;
	timecode_sample_to_time(t, r, rate,
			timecode_seconds_to_sample(sec, rate));
}

double timecode_to_sec (TimecodeTime const * const t, TimecodeRate const * const r) {
	const double rate = TCtoDbl(r) * r->subframes;
	return timecode_sample_to_seconds(timecode_to_sample(t, r, rate), rate);
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

	if (t->frame < fps) goto done;
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

int timecode_datetime_increment (Timecode * const dt) {
	if (timecode_time_increment(&dt->t, &dt->r)) {
		timecode_date_increment(&dt->d);
		return 1;
	}
	return 0;
}

int timecode_datetime_decrement (Timecode * const dt) {
	if (timecode_time_decrement(&dt->t, &dt->r)) {
		timecode_date_decrement(&dt->d);
		return 1;
	}
	return 0;
}

/*****************************************************************************
 * Format & Parse
 */

void timecode_time_to_string (char *smptestring, TimecodeTime const * const t) {
	snprintf(smptestring, 12, "%02d:%02d:%02d:%02d",
			t->hour, t->minute, t->second, t->frame);
}

/* custom version of strncpy - pointer limit, return end of string */
static char *_strlcpy(char *dest, const char *limit, const char *src) {
	while (dest < limit && (*dest = *src++) != '\0') ++dest;
	return dest;
}

#define _fmtval(dest, limit, format, ...)  \
{                                          \
	char tmp[32];                            \
	snprintf(tmp, 32, format, __VA_ARGS__);  \
	tmp[31]= '\0';                           \
	dest = _strlcpy(dest, limit, tmp);       \
}

/* follows strftime() where appropriate */
static char *_fmttc(char *p, const char *limit, const char *format, const Timecode const * const tc) {
	for ( ; *format; ++format) {
		if (*format == '%') {
			switch (*++format) {
				/* misc */
				case '\0':
					--format;
					break;
				case 't':
						p= _strlcpy(p, limit, "\t");
					continue;

				/* date, timezone */
				case 'm':
					_fmtval(p, limit, "%02d", tc->d.month);
					continue;
				case 'd':
					_fmtval(p, limit, "%02d", tc->d.day);
					continue;
				case 'y':
					_fmtval(p, limit, "%02d", tc->d.year%100);
					continue;
				case 'Y':
					_fmtval(p, limit, "%04d", tc->d.year);
					continue;
				case 'z':
					_fmtval(p, limit, "%+03d%02d", tc->d.timezone/60, abs(tc->d.timezone)%60);
					continue;

				/* frame rate */
				case ':':
					if (tc->r.drop) {
						p= _strlcpy(p, limit, ";");
					} else {
						p= _strlcpy(p, limit, ":");
					}
					continue;

				case 'f':
					if (tc->r.den == 1) {
						_fmtval(p, limit, "%d", tc->r.num);
					} else {
						_fmtval(p, limit, "%.2f", TCtoDbl(&tc->r));
					}
					if (tc->r.drop) {
						p= _strlcpy(p, limit, "df");
					}
					continue;

				/* time, frames */
				case 'H':
					_fmtval(p, limit, "%02d", tc->t.hour);
					continue;
				case 'M':
					_fmtval(p, limit, "%02d", tc->t.minute);
					continue;
				case 'S':
					_fmtval(p, limit, "%02d", tc->t.second);
					continue;
				case 'F':
					{
						int lz = (tc->r.den < 1 || TCtoDbl(&tc->r) <= 1) ? 1 : ceil(log10(TCtoDbl(&tc->r)));
						char fmt[8]; snprintf(fmt, 8, "%%0%dd", lz%10);
						_fmtval(p, limit, fmt, tc->t.frame);
					}
					continue;
				case 's':
					{
						int lz = tc->r.subframes < 1 ? 1 : ceil(log10(tc->r.subframes));
						char fmt[8]; snprintf(fmt, 8, "%%0%dd", lz%10);
						_fmtval(p, limit, fmt, tc->t.subframe);
					}
					continue;

				/* presets */
				case 'T':
					p = _fmttc(p, limit, "%H:%M:%S%;%F", tc);
					continue;
				case 'Z':
					p = _fmttc(p, limit, "%Y-%m-%d %H:%M:%S%:%F.%s %z @%f fps", tc);
					continue;
				default:
					break; // out of select
			}
		}
		if (p == limit)
			break; // out of for-loop
		*p++ = *format;
	}
	return p;
}

size_t timecode_strftimecode (char *str, const size_t maxsize, const char *format, const Timecode const * const t) {
	char *p;
	p = _fmttc(str, str + maxsize, ((format == NULL) ? "%c" : format), t);
	if (p == str + maxsize) return 0;
	*p = '\0';
	return p - str;
}

size_t timecode_strftime (char *str, const size_t maxsize, const char *format, const TimecodeTime const * const t, const TimecodeRate const * const r) {
	Timecode tc;
	memset(&tc, 0, sizeof(Timecode));
	memcpy(&tc.t, t, sizeof(TimecodeTime));
	if (r) {
		memcpy(&tc.r, r, sizeof(TimecodeRate));
	} else {
		tc.r.num=tc.r.den=tc.r.subframes=1;
	}
	return timecode_strftimecode(str, maxsize, format, &tc);
}

/* C99 only defines strpbrk() - this is the reverse version */
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

void timecode_parse_packed_time (TimecodeTime * const t, const char *val) {
	const int bcd = atoi(val);
	t->hour     = (bcd/1000000)%24;
	t->minute   = (bcd/10000)%60;
	t->second   = (bcd/100)%60;
	t->frame    = (bcd%100);
	t->subframe = 0;
}

void timecode_parse_timezone (TimecodeDate * const d, const char *val) {
	const int tz = atoi(val);
	d->timezone = (tz/100)*60  + (abs(tz)%100);
}

/* corresponds to https://github.com/x42/libltc SMPTETimecode */
struct LTCTimecode {
	char timezone[6];     ///< "+HHMM" format
	unsigned char years;  ///< LTC-date uses 2-digit year 00.99
	unsigned char months; ///< valid months are 1..12
	unsigned char days;   ///< day of month 1..31

	unsigned char hours;  ///< hour 0..23
	unsigned char mins;   ///< minute 0..60
	unsigned char secs;   ///< second 0..60
	unsigned char frame;  ///< sub-second frame 0..{FPS-1}
};

void timecode_parse_libltc_timecode (Timecode * const tc, const void *ltctc) {
	struct LTCTimecode *ltc = (struct LTCTimecode*) ltctc;
	const int tz = atoi(ltc->timezone);

	tc->d.timezone = (tz/100)*60  + (abs(tz)%100);
	tc->d.year     = ltc->years;
	tc->d.month    = ltc->months;
	tc->d.day      = ltc->days;

	tc->t.hour     = ltc->hours;
	tc->t.minute   = ltc->mins;
	tc->t.second   = ltc->secs;
	tc->t.frame    = ltc->frame;
	tc->t.subframe = 0;
}

void timecode_copy_rate (Timecode * const tc, TimecodeRate const * const r) {
	memcpy(&tc->r, r, sizeof(TimecodeRate));
}

void timecode_set_rate (Timecode * const tc, const int num, const int den, const int df, const int sf) {
	tc->r.num       = num;
	tc->r.den       = den;
	tc->r.drop      = df;
	tc->r.subframes = sf;
}

void timecode_set_time (Timecode * const tc, const int H, const int M, const int S, const int F, const int s) {
	tc->t.hour     = H;
	tc->t.minute   = M;
	tc->t.second   = S;
	tc->t.frame    = F;
	tc->t.subframe = s;
}

void timecode_set_date (Timecode * const tc, const int y, const int m, const int d, const int tz) {
	tc->d.year     = y;
	tc->d.month    = m;
	tc->d.day      = d;
	tc->d.timezone = tz;
}
