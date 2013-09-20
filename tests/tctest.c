// TODO
//
// this needs to become a proper self-test
// and example-code.
//
// currently it's a mess of things :)

#include <stdio.h>
#include <string.h>
#include <timecode/timecode.h>

int checkfps(int64_t magic, TimecodeRate const * const fps, double samplerate) {
	TimecodeTime t;
	char tcs[128];
	int64_t test;

	timecode_sample_to_time(&t, fps, samplerate, magic);
	test = timecode_to_sample(&t, fps, samplerate);
	timecode_strftime(tcs, 128, "%Z", &t, fps); fprintf(stdout, "%s\n", tcs);
	printf("%lld %lld  diff: %lld\n", test, magic, magic-test);

	return 0;
}

int checkadd(TimecodeRate const * const fps, double samplerate) {
	TimecodeTime t;
	char tcs[20];
	int64_t test;

	t.hour   = 2;
	t.minute = 33;
	t.second = 59;
	t.frame  = 22;
	t.subframe = 20;

	test = timecode_to_sample(&t, fps, samplerate);
	test*=2;
	timecode_sample_to_time(&t, fps, samplerate, test);
	timecode_time_to_string(tcs, &t); fprintf(stdout, "%s\n", tcs);
	test/=2;
	timecode_sample_to_time(&t, fps, samplerate, test);

	t.hour   = 2;
	t.minute = 33;
	t.second = 59;
	t.frame  = 22;
	t.subframe = 20;
	timecode_time_add(&t, fps, &t, &t);
	timecode_time_to_string(tcs, &t); fprintf(stdout, "%s\n", tcs);
	return 0;
}

int checksub(TimecodeRate const * const fps, double samplerate) {
	TimecodeTime t1,t2,res;
	char tcs[20];
	int64_t test1,test2;

	t1.hour   = 2;
	t1.minute = 33;
	t1.second = 59;
	t1.frame  = 22;
	t1.subframe = 0;

	t2.hour   = 0;
	t2.minute = 20;
	t2.second = 9;
	t2.frame  = 12;
	t2.subframe = 50;

	test1 = timecode_to_sample(&t1, fps, samplerate);
	test2 = timecode_to_sample(&t2, fps, samplerate);

	timecode_sample_to_time(&res, fps, samplerate, test1-test2);
	timecode_time_to_string(tcs, &res); fprintf(stdout, "%s\n", tcs);

	timecode_time_subtract(&res, fps, &t1, &t2);
	timecode_time_to_string(tcs, &res); fprintf(stdout, "%s\n", tcs);
	return 0;
}

int checkcmp() {
	Timecode a,b;

	a.d.year     = 2008;
	a.d.month    = 12;
	a.d.day      = 31;
	a.d.timezone = 0;
	a.t.hour     = 23;
	a.t.minute   = 59;
	a.t.second   = 59;
	a.t.frame    = 29;
	a.t.subframe = 29;

	b.d.year     = 2008;
	b.d.month    = 12;
	b.d.day      = 31;
	b.d.timezone = -60;
	b.t.hour     = 23;
	b.t.minute   = 59;
	b.t.second   = 59;
	b.t.frame    = 29;
	b.t.subframe = 29;

	printf("compare a<>b = %d\n", 
			timecode_datetime_compare(TCFPS2997DF, &a, &b));
	return 0;
}

int main (int argc, char **argv) {
  int64_t magic = 964965602; // 05:34:43:11 @29.97ndf, 48kSPS
  //int64_t magic = 1601568888; //
	Timecode tc;
	memset(&tc, 0, sizeof(Timecode));

	checkfps(259199740, TCFPS2997DF, 48000);
	checkfps(48048, TCFPS2997DF, 48000);
	checkfps(48047, TCFPS2997DF, 48000);
	/* test converter */
	printf("test convert\n");
	checkfps(magic, &tcfps23976, 48000);
	checkfps(magic, TCFPS24, 48000);
	checkfps(magic, TCFPS25, 48000);
	checkfps(magic, &tcfps2997ndf, 48000);
	checkfps(magic, TCFPS2997DF, 48000);
	checkfps(magic, TCFPS30, 48000);
	checkfps(magic, &tcfps30df, 48000);

	/* test parser */
	printf("test parser\n");
	TimecodeTime t;
	char tcs[64];
	timecode_parse_time(&t, TCFPS25, "1:::-1");
	timecode_time_to_string(tcs, &t); fprintf(stdout, "%s\n", tcs);

	timecode_parse_time(&tc.t, TCFPS25, ":::1.100");
	timecode_copy_rate(&tc, TCFPSMS);
	timecode_strftimecode(tcs, 64, "%Z", &tc); fprintf(stdout, "%s\n", tcs);

	/* test add/sub */
	printf("test addition/subtraction\n");
	checkadd(TCFPS2997DF, 48000);
	checksub(TCFPS2997DF, 48000);

	checkadd(TCFPS30, 48000);
	checksub(TCFPS30, 48000);

	/* test add/sub */
	printf("test compare\n");
	checkcmp();

	/* test inc/dec */
	printf("test inc/dec\n");
	tc.d.year     = 2008;
	tc.d.month    = 12;
	tc.d.day      = 31;
	tc.d.timezone = 0;
	tc.t.hour     = 23;
	tc.t.minute   = 59;
	tc.t.second   = 59;
	tc.t.frame    = 29;
	tc.t.subframe = 29;
	memcpy(&tc.r, TCFPS30, sizeof(TimecodeRate));

	timecode_datetime_increment(&tc);
	timecode_datetime_decrement(&tc);

	fprintf(stdout, "%02d/%02d/%02d  %02d:%02d:%02d%c%02d\n",
			tc.d.month,
			tc.d.day,
			tc.d.year,

			tc.t.hour,
			tc.t.minute,
			tc.t.second,
			(TCFPS30->drop) ? '.' : ':',
			tc.t.frame
			);

	tc.t.subframe = 0;

	timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s @30fps\n", tcs);
	timecode_convert_rate(&tc.t, TCFPS24, &tc.t, TCFPS30);
	timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s @24fps\n", tcs);
	timecode_convert_rate(&tc.t, TCFPS25, &tc.t, TCFPS24);
	timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s @25fps\n", tcs);
	timecode_convert_rate(&tc.t, TCFPS30, &tc.t, TCFPS25);
	timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s @30fps\n", tcs);
	timecode_convert_rate(&tc.t, TCFPS25, &tc.t, TCFPS30);
	timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s @25fps\n", tcs);
	timecode_convert_rate(&tc.t, TCFPS24, &tc.t, TCFPS25);
	timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s @24fps\n", tcs);
	//timecode_convert_rate(&tc.t, TCFPS30, &tc.t, TCFPS24);
	//timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s @30fps\n", tcs);
	//timecode_convert_rate(&tc.t, &tcfpsUS, &tc.t, TCFPS30);
	//timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s\n", tcs);

	printf(" to/from sec\n");
	double sec = timecode_to_sec(&tc.t, &tcfps24);
	printf("%f\n", sec);
	timecode_seconds_to_time(&tc.t, TCFPSMS, sec);

	memcpy(&tc.r, TCFPSMS, sizeof(TimecodeRate));
	tc.t.frame = 5;
	tc.d.timezone = -90;
	timecode_strftimecode(tcs, 64,"%Z", &tc); fprintf(stdout, "%s\n", tcs);

	return 0;
}
