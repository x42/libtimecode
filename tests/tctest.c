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
			timecode_datetime_compare(timecode_FPS2997DF, &a, &b));
	return 0;
}

int main (int argc, char **argv) {
	const TimecodeRate tcfpsUS      = {   1000000,   1, 0, 1};
	const TimecodeRate tcfps2997ndf = { 30000, 1001, 0, 80};
	const TimecodeRate tcfps30df    = {    30,    1, 1, 80};

  int64_t magic = 964965602; // 05:34:43:11 @29.97ndf, 48kSPS
  //int64_t magic = 1601568888; //
	Timecode tc;
	memset(&tc, 0, sizeof(Timecode));

	checkfps(259199740, timecode_FPS2997DF, 48000);
	checkfps(48048, timecode_FPS2997DF, 48000);
	checkfps(48047, timecode_FPS2997DF, 48000);
	/* test converter */
	printf("test convert\n");
	checkfps(magic, timecode_FPS23976, 48000);
	checkfps(magic, timecode_FPS24, 48000);
	checkfps(magic, timecode_FPS25, 48000);
	checkfps(magic, &tcfps2997ndf, 48000);
	checkfps(magic, timecode_FPS2997DF, 48000);
	checkfps(magic, timecode_FPS30, 48000);
	checkfps(magic, &tcfps30df, 48000);

	/* test parser */
	printf("test parser\n");
	TimecodeTime t;
	char tcs[64];
	timecode_parse_time(&t, timecode_FPS25, "1:::-1");
	timecode_time_to_string(tcs, &t); fprintf(stdout, "%s\n", tcs);

	timecode_parse_time(&tc.t, timecode_FPS25, ":::1.100");
	timecode_copy_rate(&tc, timecode_FPSMS);
	timecode_strftimecode(tcs, 64, "%Z", &tc); fprintf(stdout, "%s\n", tcs);

	/* test add/sub */
	printf("test addition/subtraction\n");
	checkadd(timecode_FPS2997DF, 48000);
	checksub(timecode_FPS2997DF, 48000);

	checkadd(timecode_FPS30, 48000);
	checksub(timecode_FPS30, 48000);

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
	memcpy(&tc.r, timecode_FPS30, sizeof(TimecodeRate));

	timecode_datetime_increment(&tc);
	timecode_datetime_decrement(&tc);

	fprintf(stdout, "%02d/%02d/%02d  %02d:%02d:%02d%c%02d\n",
			tc.d.month,
			tc.d.day,
			tc.d.year,

			tc.t.hour,
			tc.t.minute,
			tc.t.second,
			(timecode_FPS30->drop) ? '.' : ':',
			tc.t.frame
			);

	tc.t.subframe = 0;

	timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s @30fps\n", tcs);
	timecode_convert_rate(&tc.t, timecode_FPS24, &tc.t, timecode_FPS30);
	timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s @24fps\n", tcs);
	timecode_convert_rate(&tc.t, timecode_FPS25, &tc.t, timecode_FPS24);
	timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s @25fps\n", tcs);
	timecode_convert_rate(&tc.t, timecode_FPS30, &tc.t, timecode_FPS25);
	timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s @30fps\n", tcs);
	timecode_convert_rate(&tc.t, timecode_FPS25, &tc.t, timecode_FPS30);
	timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s @25fps\n", tcs);
	timecode_convert_rate(&tc.t, timecode_FPS24, &tc.t, timecode_FPS25);
	timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s @24fps\n", tcs);
	timecode_convert_rate(&tc.t, timecode_FPS30, &tc.t, timecode_FPS24);
	timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s @30fps\n", tcs);

#if 1
	timecode_convert_rate(&tc.t, &tcfpsUS, &tc.t, timecode_FPS30);
	timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s\n", tcs);
#endif

	printf(" to/from sec\n");
	double sec = timecode_to_sec(&tc.t, &tcfpsUS);
	printf("%f\n", sec);
	timecode_seconds_to_time(&tc.t, timecode_FPSMS, sec);
	memcpy(&tc.r, timecode_FPSMS, sizeof(TimecodeRate));
#if 0
	tc.t.frame = 5;
	tc.d.timezone = -90;
#endif
	timecode_strftimecode(tcs, 64,"%Z", &tc); fprintf(stdout, "%s\n", tcs);

	printf(" 29.97df test 1\n");
	timecode_copy_rate(&tc, timecode_FPS2997DF);
	timecode_parse_time(&tc.t, &tc.r, "05:35:03:15");
	printf("%lld  <> 964965602\n", timecode_to_sample(&tc.t, &tc.r, 48000)); // ??

	// https://github.com/x42/libtimecode/issues/5
	printf(" 29.97df test 2\n");
	timecode_copy_rate(&tc, timecode_FPS2997DF);
	timecode_parse_time(&tc.t, &tc.r, "08:33:25:02");
	timecode_sample_to_time (&tc.t, &tc.r, 48000, timecode_to_sample(&tc.t, &tc.r, 48000));
	timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s <> 08:33:25:02\n", tcs);

	printf(" 29.97df test 3\n");
	timecode_copy_rate(&tc, timecode_FPS2997DF);
	timecode_parse_time(&tc.t, &tc.r, "08:33:25:02");
	printf("%lld  <> 923228\n", timecode_to_framenumber(&tc.t, &tc.r));
	timecode_framenumber_to_time (&tc.t, &tc.r, timecode_to_framenumber(&tc.t, &tc.r));
	timecode_time_to_string(tcs, &tc.t); fprintf(stdout, "%s <> 08:33:25:02\n", tcs);

	printf(" 29.97ndf test\n");
	timecode_copy_rate(&tc, &tcfps2997ndf);
	timecode_parse_time(&tc.t, &tc.r, "05:34:43:11");
	printf("%lld  <> 964965602\n", timecode_to_sample(&tc.t, &tc.r, 48000));

	return 0;
}
