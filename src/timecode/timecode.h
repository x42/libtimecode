/** 
   @brief libtimecode - timecode and framerate conversion 
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

/**
 * getting started sub
 */
void timecode_sub();

#ifdef __cplusplus
}
#endif



#endif
