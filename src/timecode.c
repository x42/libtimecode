/* 
   libtimecode - convert A/V timecode and framerates

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

#ifdef HAVE_GMP
# include <gmp.h>
#endif

#include "timecode/timecode.h"

void timecode_stub() {
	;
}

#ifdef HAVE_GMP
int fl2ratio(long int *num, unsigned long int *den, double val) {
	mpq_t rational;
	mpz_t mtmp;
	if (!num || !den) return (1);
	mpz_init (mtmp);
	mpq_init (rational);
	//mpq_set_ui(rational, mnum, mden);
	mpq_set_d(rational, val);
	mpq_canonicalize(rational);
	mpq_get_num(mtmp, rational);
	*num = (long int) mpz_get_d(mtmp);
	mpq_get_den(mtmp, rational);
	*den = (long int) mpz_get_d(mtmp);

	mpq_t ntsc;
	mpq_init (ntsc);
	mpq_set_ui(ntsc,30000,1001);
#if 0 // debug
	printf("cmp: %f <=> 30000/1001 -> %i\n",val, mpq_cmp(rational,ntsc));
#endif
	mpq_clear (ntsc);
#if 0
	char *rats = mpq_get_str(NULL,10,rational);
	printf("compare %f:= %f = %li/%li = %s\n",val,mpq_get_d(rational),*num,*den,rats);
	free(rats);
#endif
	mpq_clear (rational);
	mpz_clear (mtmp);
	return(0);
}

#else /* HAVE_GMP */
int fl2ratio(long int *num, long int *den, double val) {
	return (1);
}
#endif /* HAVE_GMP */
