/**
   @brief libtimecode - convert A/V timecode and framerate
   @file timecode.h
   @author Robin Gareus <robin@gareus.org>

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

#ifndef TIMECODE_H
#define TIMECODE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h> /* size_t */

#ifndef DOXYGEN_IGNORE
/* libtimecode version */
#define LIBTIMECODE_VERSION "0.5.0"
#define LIBTIMECODE_VERSION_MAJOR  0
#define LIBTIMECODE_VERSION_MINOR  0
#define LIBTIMECODE_VERSION_MICRO  0

/* interface revision number
 * http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
 */
#define LIBTIMECODE_CUR  0
#define LIBTIMECODE_REV  0
#define LIBTIMECODE_AGE  0
#endif

#ifndef int32_t
typedef int int32_t;
#endif

#ifndef int64_t
typedef long long int int64_t;
#endif

typedef struct TimecodeTime {
	int32_t hour;
	int32_t minute;
	int32_t second;
	int32_t frame;
	int32_t subframe;
} TimecodeTime;


typedef struct TimecodeDate {
	int32_t year;
	int32_t month;
	int32_t day;
	int32_t timezone; // minutes west of UTC
} TimecodeDate;


typedef struct TimecodeRate {
	int32_t num;
	int32_t den;
	int drop; //< bool
	int32_t subframes;
} TimecodeRate;

typedef struct Timecode {
	TimecodeTime t;
	TimecodeDate d;
} Timecode;


const TimecodeRate tcfps23976  = {24000, 1001, 0, 80};
const TimecodeRate tcfps24     = {24, 1, 0, 100};
const TimecodeRate tcfps24976  = {25000, 1001, 0, 80};
const TimecodeRate tcfps25     = {25, 1, 0, 80};
const TimecodeRate tcfps2997df = {30000, 1001, 1, 100};
const TimecodeRate tcfps30     = {30, 1, 0, 80};
const TimecodeRate tcfpsMS     = {1000, 1, 0, 1000};

#define TCFPS23976 (&tcfps23976)
#define TCFPS24 (&tcfps24)
#define TCFPS24976 (&tcfps24976)
#define TCFPS25 (&tcfps25)
#define TCFPS2997DF (&tcfps2997df)
#define TCFPS30 (&tcfps30)
#define TCFPSMS (&tcfpsMS)


/**
 * convert Timecode to audio sample number
 *
 * NB. this function can also be used to convert integer milli-seconds or
 * micro-seconds by specifying a samplerate of 1000 or 10^6 respectively.
 *
 * @param t the timecode to convert
 * @param r framerate to use for conversion
 * @param samplerate the sample-rate the sample was taken at
 * @return audio sample number
 */
int64_t timecode_to_sample (TimecodeTime const * const t, TimecodeRate const * const r, const double samplerate);

/**
 * convert audio sample number to Timecode
 *
 * NB. this function can also be used to convert integer milli-seconds or
 * micro-seconds by specifying a samplerate of 1000 or 10^6 respectively.
 *
 * @param t [output] the timecode that corresponds to the sample
 * @param r framerate to use for conversion
 * @param samplerate the sample-rate the sample was taken at
 * @param sample the audio sample number to convert
 */
void timecode_sample_to_time (TimecodeTime * const t, TimecodeRate const * const r, const double samplerate, const int64_t sample);


/* ADD/SUBTRACT timecodes at same framerate */

/**
 */
void timecode_time_add (TimecodeTime * const res, TimecodeRate const * const r, TimecodeTime const * const t1, TimecodeTime const * const t2);
/**
 */
void timecode_time_subtract (TimecodeTime * const res, TimecodeRate const * const r, TimecodeTime const * const t1, TimecodeTime const * const t2);


/* COMPARISON OPERATORS at same framerate */

/**
 * @return +1 if a is later than b, -1 if a is earlier than b, 0 if timecodes are equal
 */
int timecode_time_compare (TimecodeRate const * const r, TimecodeTime const * const a, TimecodeTime const * const b);

/**
 * @return +1 if a is later than b, -1 if a is earlier than b, 0 if timecodes are equal
 */
int timecode_date_compare (TimecodeDate const * const a, TimecodeDate const * const b);

/**
 * @return +1 if a is later than b, -1 if a is earlier than b, 0 if timecodes are equal
 */
int timecode_datetime_compare (TimecodeRate const * const r, Timecode const * const a, Timecode const * const b);


/* increment, decrement */

/**
 */
void timecode_date_increment(TimecodeDate * const d);
/**
 */
int timecode_time_increment(TimecodeTime * const t, TimecodeRate const * const r);
/**
 */
void timecode_date_decrement (TimecodeDate * const d);
/**
 */
int timecode_time_decrement(TimecodeTime * const t, TimecodeRate const * const r);
/**
 */
int timecode_datetime_increment (Timecode * const dt, TimecodeRate const * const r);
/**
 */
int timecode_datetime_decrement (Timecode * const dt, TimecodeRate const * const r);


/* parse from string, export to string */

/**
 * length of smptestring: 33 bytes
 */
void timecode_datetime_to_string (Timecode const * const tc, char *smptestring);
/**
 * length of smptestring: 16 bytes
 */
void timecode_time_to_string (TimecodeTime const * const t, char *smptestring);
/**
 */
void timecode_parse_time (TimecodeTime * const t, TimecodeRate const * const r, const char *val);

/**
 */
double timecode_fr_to_double(TimecodeRate const * const r);

/* TODO, ideas */
// add/subtract int frames or float sec
// parse from float sec, export to float sec
// convert to video frame number and back
// -> use timecode_rate_to_double() as samplerate
//
// convert between framerates including milli-seconds HH:MM:SS.DDD
// Bar, Beat, Tick Time (Tempo-Based Time)

#ifdef __cplusplus
}
#endif



#endif
