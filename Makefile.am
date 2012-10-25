AUTOMAKE_OPTIONS = foreign

SUBDIRS = @subdirs@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = timecode.pc

EXTRA_DIST= \
	timecode.pc.in Doxyfile.in \
	README.md ChangeLog \
	COPYING INSTALL AUTHORS

CLEANFILES = stamp-doxygen stamp-doc

dox: stamp-doxygen

stamp-doxygen: src/timecode/timecode.h doc/mainpage.dox Doxyfile
	$(DOXYGEN) Doxyfile
	touch stamp-doxygen
