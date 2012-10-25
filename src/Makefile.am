lib_LTLIBRARIES = libtimecode.la
include_HEADERS = timecode/timecode.h

libtimecode_la_SOURCES=timecode.c config.h timecode/timecode.h
libtimecode_la_LDFLAGS=@LIBTIMECODE_LDFLAGS@ -version-info @VERSION_INFO@
libtimecode_la_LIBADD=-lm
libtimecode_la_CFLAGS=@LIBTIMECODE_CFLAGS@
