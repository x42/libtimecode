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
#define LIBTIMECODE_VERSION "0.1.0"
#define LIBTIMECODE_VERSION_MAJOR  0
#define LIBTIMECODE_VERSION_MINOR  1
#define LIBTIMECODE_VERSION_MICRO  0

/* interface revision number
 * http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
 */
#define LIBTIMECODE_CUR  1
#define LIBTIMECODE_REV  0
#define LIBTIMECODE_AGE  0
#endif

/* ISO C99 has this in <inttypes.h> */
#if (!defined int32_t && !defined __int8_t_defined)
typedef int int32_t;
#endif

#if (!defined int64_t && !defined __int8_t_defined)
#  if __WORDSIZE == 64
typedef long int int64_t;
#else
typedef long long int int64_t;
#endif
#endif

/**
 * classical timecode
 */
typedef struct TimecodeTime {
	int32_t hour; ///< timecode hours 0..24
	int32_t minute; ///< timecode minutes 0..59
	int32_t second; ///< timecode seconds 0..59
	int32_t frame; ///< timecode frames 0..fps
	int32_t subframe; ///< timecode subframes 0..
} TimecodeTime;

/**
 * date and timezone
 */
typedef struct TimecodeDate {
	int32_t year; ///< year A.D. 4 digits
	int32_t month; ///< month 1..12
	int32_t day;  ///< day of month 0..31
	int32_t timezone; ///< minutes west of UTC +1245 (Chatham Island) .. -1200 (Kwaialein); LA: -0800, NYC: -0500, Paris: +0100, Bombay: +0530, Tokyo: +0900
} TimecodeDate;


/**
 * define a frame rate
 */
typedef struct TimecodeRate {
	int32_t num; ///< fps numerator
	int32_t den; ///< fps denominator
	int drop; ///< 1: use drop-frame timecode (only valid for 2997/100)
	int32_t subframes; ///< number of subframes per frame - may be zero
} TimecodeRate;


extern const TimecodeRate* timecode_FPS23976;  ///< { 24000, 1001, 0, 80};
extern const TimecodeRate* timecode_FPS24;     ///< {    24,    1, 0, 80};
extern const TimecodeRate* timecode_FPS24976;  ///< { 25000, 1001, 0, 80};
extern const TimecodeRate* timecode_FPS25;     ///< {    25,    1, 0, 80};
extern const TimecodeRate* timecode_FPS2997DF; ///< { 30000, 1001, 1, 80};
extern const TimecodeRate* timecode_FPS30;     ///< {    30,    1, 0, 80};
extern const TimecodeRate* timecode_FPSMS;     ///< {  1000,    1, 0, 1000};

/**
 * complete date time description incl frame rate
 */
typedef struct Timecode {
	TimecodeTime t; ///< timecode HH:MM:SS:FF.SSS
	TimecodeDate d; ///< date MM/DD/YYYY + Timezone
	TimecodeRate r; ///< the frame rate used for TimecodeTime
} Timecode;


/*  --- misc functions  --- */

/**
 * convert rational frame rate to double (r->num / r->den).
 *
 * @param r frame rate to convert
 * @return double representation of frame rate
 */
double timecode_rate_to_double(TimecodeRate const * const r);

/**
 * calculate samples per timecode-frame for a given sample rate:
 * (samplerate * r->num / r->den).
 *
 * @param r frame rate to convert
 * @param samplerate the sampling rate
 * @return number of samples per timecode-frame.
 */
double timecode_frames_per_timecode_frame(TimecodeRate const * const r, const double samplerate);


/*  --- timecode <> sample,frame-number  --- */

/**
 * convert timecode to audio sample number
 *
 * NB. this function can also be used to convert integer milli-seconds or
 * micro-seconds by specifying a sample rate of 1000 or 10^6 respectively.
 *
 * When used with samplerate == \ref timecode_rate_to_double this function can also convert
 * timecode to video-frame number.
 *
 * @param t the timecode to convert
 * @param r frame rate to use for conversion
 * @param samplerate the sample rate the sample was taken at
 * @return audio sample number
 */
int64_t timecode_to_sample (TimecodeTime const * const t, TimecodeRate const * const r, const double samplerate);

/**
 * convert audio sample number to timecode
 *
 * NB. this function can also be used to convert integer milli-seconds or
 * micro-seconds by specifying a sample rate of 1000 or 10^6 respectively.
 *
 * When used with samplerate == \ref timecode_rate_to_double this function can also convert
 * video-frame number to timecode.
 *
 * @param t [output] the timecode that corresponds to the sample
 * @param r frame rate to use for conversion
 * @param samplerate the sample rate the sample was taken at
 * @param sample the audio sample number to convert
 */
void timecode_sample_to_time (TimecodeTime * const t, TimecodeRate const * const r, const double samplerate, const int64_t sample);


/**
 * convert timecode to frame number
 *
 * this function simply calls \ref timecode_to_sample with the
 * samplerate set to the fps.
 *
 * @param t the timecode to convert
 * @param r frame rate to use for conversion
 * @return frame-number
 */
int64_t timecode_to_framenumber (TimecodeTime const * const t, TimecodeRate const * const r);

/**
 * convert video frame-number to timecode
 *
 * this function simply calls \ref timecode_framenumber_to_time with the
 * sample rate set to the fps.
 *
 * @param t [output] the timecode that corresponds to the frame
 * @param r frame rate to use for conversion
 * @param frameno the frame-number to convert
 */
void timecode_framenumber_to_time (TimecodeTime * const t, TimecodeRate const * const r, const int64_t frameno);

/**
 * convert timecode from one rate to another.
 *
 * Note: if t_out points to the same timecode as t_in, the timecode will be modified.
 *
 * @param t_out [output] timecode t_in converted to frame rate r_out
 * @param r_out frame rate to convert to
 * @param t_in the timecode to convert (may be identical to t_out)
 * @param r_in the frame rate of the timecode to convert from
 *
 */
void timecode_convert_rate (TimecodeTime * const t_out, TimecodeRate const * const r_out, TimecodeTime * const t_in, TimecodeRate const * const r_in);


/* --- float seconds --- */

/**
 * convert sample number to floating point seconds
 *
 * @param sample sample number (starting at zero)
 * @param samplerate sample rate
 * @return seconds
 */
double timecode_sample_to_seconds (const int64_t sample, double samplerate);

/**
 * convert floating-point seconds to nearest sample number at given sample rate.
 *
 * @param sec seconds
 * @param samplerate sample rate
 * @return sample number (starting at zero)
 */
int64_t timecode_seconds_to_sample (const double sec, double samplerate);

/**
 * convert frame number to floating point seconds
 *
 * @param frameno frame number (starting at zero)
 * @param r frame rate
 * @return seconds
 */
double timecode_framenumber_to_seconds (const int64_t frameno, TimecodeRate const * const r);

/**
 * convert floating-point seconds to timecode frame number at given frame rate.
 * Opposed to \ref timecode_seconds_to_sample which rounds the sample number to
 * the nearest sample, this function always rounds down to the current frame.
 *
 * @param sec seconds
 * @param r frame rate
 * @return sample number (starting at zero)
 */
int64_t timecode_seconds_to_framenumber (const double sec, TimecodeRate const * const r);

/**
 * convert floating point seconds to timecode.
 *
 * uses \ref timecode_sample_to_time and \ref timecode_seconds_to_sample.
 *
 * @param t [output] the timecode that corresponds to the sample
 * @param r frame rate to use for conversion
 * @param sec seconds to convert
 */
void timecode_seconds_to_time (TimecodeTime * const t, TimecodeRate const * const r, const double sec);

/**
 * convert timecode to floating point seconds.
 *
 * uses \ref timecode_sample_to_seconds and \ref timecode_to_sample.
 *
 * @param t the timecode to convert
 * @param r frame rate
 * @return seconds
 */
double timecode_to_sec (TimecodeTime const * const t, TimecodeRate const * const r);


/*  --- add/subtract timecodes at same frame rate  --- */

/**
 * add two timecodes at same frame rate.
 *
 * Note: res, t1 and t2 may all point to the same structure.
 *
 * @param res [output] result of addition
 * @param r frame rate
 * @param t1 first summand
 * @param t2 second summand
 */
void timecode_time_add (TimecodeTime * const res, TimecodeRate const * const r, TimecodeTime const * const t1, TimecodeTime const * const t2);

/**
 * subtract timecode at same frame rate.
 *
 * Note: res, t1 and t2 may all point to the same structure.
 *
 * @param res [output] difference between t1 and t2: (t1-t2)
 * @param r frame rate
 * @param t1 minuend
 * @param t2 subtrahend
 */
void timecode_time_subtract (TimecodeTime * const res, TimecodeRate const * const r, TimecodeTime const * const t1, TimecodeTime const * const t2);


/*  --- comparison operators at same frame rate  --- */

/**
 * The timecode_time_compare() function compares the two timecodes a and b.
 * It returns an integer less than, equal to, or greater than zero if a is
 * found, respectively, to be later than, to match, or be earlier than b.
 *
 * @param r frame rate to use for both a and b
 * @param a timecode to compare (using frame rate r)
 * @param b timecode to compare (using frame rate r)
 * @return +1 if a is later than b, -1 if a is earlier than b, 0 if timecodes are equal
 */
int timecode_time_compare (TimecodeRate const * const r, TimecodeTime const * const a, TimecodeTime const * const b);

/**
 * The timecode_date_compare() function compares the two dates a and b.
 * It returns an integer less than, equal to, or greater than zero if a is
 * found, respectively, to be later than, to match, or be earlier than b.
 *
 * @param a date to compare
 * @param b date to compare
 * @return +1 if a is later than b, -1 if a is earlier than b, 0 if timecodes are equal
 */
int timecode_date_compare (TimecodeDate const * const a, TimecodeDate const * const b);

/**
 * The timecode_datetime_compare() function compares the two datetimes a and b.
 * It returns an integer less than, equal to, or greater than zero if a is
 * found, respectively, to be later than, to match, or be earlier than b.
 *
 * This function is a wrapper around \ref timecode_time_compare and \ref timecode_date_compare
 * it includes additional functionality to handle timezones correctly.
 *
 * @param r frame rate to use for both a and b
 * @param a timecode to compare (using frame rate r)
 * @param b timecode to compare (using frame rate r)
 * @return +1 if a is later than b, -1 if a is earlier than b, 0 if timecodes are equal
 */
int timecode_datetime_compare (TimecodeRate const * const r, Timecode const * const a, Timecode const * const b);


/*  --- increment, decrement  --- */

/**
 * increment date by one day.
 * Note: This function honors leap-years.
 * @param d the date to adjust
 */
void timecode_date_increment(TimecodeDate * const d);

/**
 * decrement date by one day.
 * Note: this function honors leap-years.
 * @param d the date to adjust
 */
void timecode_date_decrement (TimecodeDate * const d);

/**
 * increment timecode by one frame.
 * @param t the timecode to modify
 * @param r frame rate to use
 * @return 1 if timecode wrapped 24 hours, 0 otherwise
 */
int timecode_time_increment(TimecodeTime * const t, TimecodeRate const * const r);

/**
 * decrement timecode by one frame.
 * @param t the timecode to modify
 * @param r frame rate to use
 * @return 1 if timecode wrapped 24 hours, 0 otherwise
 */
int timecode_time_decrement(TimecodeTime * const t, TimecodeRate const * const r);

/**
 * increment datetime by one frame
 * this is a wrapper function around \ref timecode_date_increment and
 * \ref timecode_time_increment
 * @param dt the datetime to modify
 * @return 1 if timecode wrapped 24 hours, 0 otherwise
 */
int timecode_datetime_increment (Timecode * const dt);

/**
 * increment datetime by one frame
 * this is a wrapper function around \ref timecode_date_increment and
 * \ref timecode_time_increment
 * @param dt the datetime to modify
 * @return 1 if timecode wrapped 24 hours, 0 otherwise
 */
int timecode_datetime_decrement (Timecode * const dt);


/*  --- parse from string, export to string  --- */

/**
 * format timecode as string "HH:MM:SS:FF".
 *
 * @param smptestring [output] formatted string, must be at least 12 bytes long.
 * @param t the timecode to print
 */
void timecode_time_to_string (char *smptestring, TimecodeTime const * const t);

/**
 * print formatted timecode to text string.
 *
 * The formatting is very similar to that of strftime or printf:
 * The format string is a a null-terminated string composed of zero or more directives:
 * ordinary characters (not %), which are copied unchanged to the output stream; and
 * conversion specifications, each conversion specification is introduced by the
 * character %, and ends with a conversion specifier.
 *
 * The characters of conversion specifications are replaced as follows:
 *
 * Time conversion:
 *
 * - \%H   timecode hour as a 2-digit integer.
 *
 * - \%M   timecode minute as a 2-digit integer.
 *
 * - \%S   timecode second as a 2-digit integer.
 *
 * - \%F   timecode frame number with leading zeros - the number of leading zeroes depends on the frame rate.
 *
 * - \%s   timecode subframes with leading zeros - the number of leading zeroes depends on the frame rate.
 *
 * Date conversion:
 *
 * - \%y   The year as a decimal number without a century (range 00 to 99).
 *
 * - \%Y   The year as a decimal number including the century.
 *
 * - \%m   The month as a decimal number (range 01 to 12).
 *
 * - \%d   The day of the month as a decimal number (range 01 to 31).
 *
 * Frame rate conversion:
 *
 * - \%f   frame rate incl. drop-frame postfix e.g. "25" or "29.97df"
 *
 * - \%:   frame separator  ':' for non-drop-frame, ';' for drop-frame timecode
 *
 * Miscellaneous and presets:
 *
 * - \%t   A tab character.
 *
 * - \%%   A literal '%' character.
 *
 * - \%T   preset alias for "%H:%M:%S%;%F"
 *
 * - \%Z   preset alias for "%Y-%m-%d %H:%M:%S%:%F.%s %z @%f fps"
 *
 * @param str [output] formatted string str (must be large enough).
 * @param maxsize write at most maxsize bytes (including the trailing null byte ('\0')) to str
 * @param format the format directive
 * @param t the timecode to format
 * @return number of bytes written to str
 */
size_t timecode_strftimecode (char *str, const size_t maxsize, const char *format, Timecode const * const t);

/**
 * wrapper around \ref timecode_strftimecode for formatting timecode time.
 *
 * @param str [output] formatted string str (must be large enough).
 * @param maxsize write at most maxsize bytes (including the trailing null byte ('\0')) to str
 * @param format the format directive
 * @param t the timecode time to format
 * @param r optional framerate (may be NULL)
 * @return number of bytes written to str
 */
size_t timecode_strftime (char *str, const size_t maxsize, const char *format, TimecodeTime const * const t, TimecodeRate const * const r);

/**
 * parse string to timecode time - separators may include ":;"
 * a dot separator indicates subframe division.
 *
 * The accepted format is "[[[HH:]MM:]SS:]FF[.SF]".
 * if the format is "[[[HH:]MM:]SS:]FF", subframes are set to 0.
 *
 * oveflow in each unit moved up to the next unit.
 *
 * @param t [output] the parsed timecode
 * @param r frame rate to use
 * @param val the value to parse
 * @return 24hour overflow in days
 */
int32_t timecode_parse_time (TimecodeTime * const t, TimecodeRate const * const r, const char *val);


/**
 * TODO documentation
 */
void timecode_parse_packed_time (TimecodeTime * const t, const char *val);

/**
 * TODO documentation
 */
void timecode_parse_timezone (TimecodeDate * const d, const char *val);

/**
 * TODO documentation
 */
void timecode_parse_framerate (TimecodeRate * const r, const char *val, int flags);


/*  --- misc assignment functions --- */

/**
 * TODO documentation
 */
void timecode_parse_libltc_timecode (Timecode * const tc, const void *ltctc);

/**
 * TODO documentation
 */
void timecode_copy_rate (Timecode * const tc, TimecodeRate const * const r);

/**
 * TODO documentation
 */
void timecode_set_rate (Timecode * const tc, const int num, const int den, const int df, const int sf);

/**
 * TODO documentation
 */
void timecode_set_time (Timecode * const tc, const int H, const int M, const int S, const int F, const int s);

/**
 * TODO documentation
 */
void timecode_set_date (Timecode * const tc, const int y, const int m, const int d, const int tz);

/**
 * TODO documentation
 */
void timecode_reset_unixtime (Timecode * const tc);

/**
 * TODO documentation - move to compare functions
 */
int timecode_date_is_valid(TimecodeDate * const d);

/**
 * TODO documentation - move to compare/add functions
 */
void timecode_move_date_overflow(TimecodeDate * const d);

/* TODO, ideas */
// add compare function that ignores subframes (or any field) via flag
// Bar, Beat, Tick Time (Tempo-Based Time)

#ifdef __cplusplus
}
#endif


#endif
