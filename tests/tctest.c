#include <stdio.h>
#include <timecode/timecode.h>

int checkfps(int64_t magic, TimecodeRate const * const fps) {
	TimecodeTime t;
	char tcs[20];
	int64_t test;

	timecode_sample_to_time(&t, fps, 48000, magic);
	test = timecode_to_sample(&t, fps, 48000);
	timecode_time_to_string(&t, tcs); fprintf(stdout, "%s\n", tcs);
	printf("%lld %lld  diff: %lld\n", test, magic, magic-test);

	return 0;
}

int main (int argc, char **argv) {
  int64_t magic = 964965602; // 05:34:42:11 @29.97df
	Timecode tc;

	checkfps(magic, TCFPS24);
	checkfps(magic, TCFPS25);
	checkfps(magic, TCFPS2997DF);
	checkfps(magic, TCFPS30);

	tc.d.year   =  8;
	tc.d.month  = 12;
	tc.d.day    = 31;
	tc.t.hour   = 23;
	tc.t.minute = 59;
	tc.t.second = 59;
	tc.t.frame  = 29;

	timecode_datetime_increment(&tc, TCFPS30);
	timecode_datetime_decrement(&tc, TCFPS30);

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

	return 0;
}
