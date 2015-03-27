AC_PREREQ(2.57)
AC_INIT([libtimecode], [-], [robin@gareus.org], [], [http://x42.github.com/libtimecode/])

AC_PATH_PROG(SED, sed, "", $PATH:/bin:/usr/bin:/usr/local/bin)
if test -z "$SED"; then
  AC_MSG_ERROR([sed was not found -- sed is requied to extract version information])
fi

AC_MSG_CHECKING([libtimecode version])
VERSION=`$SED -ne 's/^#define LIBTIMECODE_VERSION "\(.*\)"/\1/p' ${srcdir}/src/timecode/timecode.h 2>/dev/null`
AC_MSG_RESULT($VERSION)
if test -z "$VERSION"; then
  AC_MSG_ERROR([version number can not be retrieved from src/timecode/timecode.h])
fi

VERSION_CUR=`$SED -ne 's/^#define LIBTIMECODE_CUR *\([0-9]*\)/\1/p' ${srcdir}/src/timecode/timecode.h 2>/dev/null`
VERSION_REV=`$SED -ne 's/^#define LIBTIMECODE_REV *\([0-9]*\)/\1/p' ${srcdir}/src/timecode/timecode.h 2>/dev/null`
VERSION_AGE=`$SED -ne 's/^#define LIBTIMECODE_AGE *\([0-9]*\)/\1/p' ${srcdir}/src/timecode/timecode.h 2>/dev/null`
VERSION_INFO=${VERSION_CUR}:${VERSION_REV}:${VERSION_AGE}

AC_CONFIG_SRCDIR([src/timecode.c])
AC_CONFIG_TESTDIR([tests])
AC_CANONICAL_TARGET([])
AC_COPYRIGHT([Copyright (C) Robin Gareus 2006-2012])

AM_INIT_AUTOMAKE(libtimecode,$VERSION)
AM_CONFIG_HEADER(src/config.h)

AC_CANONICAL_HOST

AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_LN_S
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL
AM_PROG_CC_C_O
AC_LIBTOOL_WIN32_DLL

dnl *** misc complier/linker flags ***
LIBTIMECODE_CFLAGS="-Wall"
LIBTIMECODE_LDFLAGS="-export-symbols-regex '^(timecode_).*'"

case $target_os in
  *mingw32*|*win*)
    LDFLAGS="-no-undefined"
		;;
  *)
    AC_MSG_CHECKING([if -Wl,--as-needed works])
    LDFLAGS_save=$LDFLAGS
    LDFLAGS="$LDFLAGS -Wl,--as-needed"
    AC_TRY_LINK([], [],
                [
                 AC_MSG_RESULT([yes])
                 LIBTIMECODE_LDFLAGS="$LIBTIMECODE_LDFLAGS -Wl,--as-needed"
                ],
                [AC_MSG_RESULT([no])])
    LDFLAGS=$LDFLAGS_save
  ;;
esac

dnl *** check for dependencies ***
AC_CHECK_HEADERS(stdio.h stdlib.h string.h unistd.h sys/types.h stdint.h)

dnl *** check for doxygen ***
AC_ARG_VAR(DOXYGEN, Doxygen)
AC_PATH_PROG(DOXYGEN, doxygen, no)

if test "$DOXYGEN" != "no"; then
  DOXMAKE='run "make dox" to generate API html reference: doc/html/index.html'
fi

subdirs="src doc tests"

AC_SUBST(subdirs)
AC_SUBST(VERSION)
AC_SUBST(VERSION_INFO)
AC_SUBST(LIBTIMECODE_CFLAGS)
AC_SUBST(LIBTIMECODE_LDFLAGS)

AC_OUTPUT(Makefile src/Makefile doc/Makefile tests/Makefile timecode.pc Doxyfile)

AC_MSG_NOTICE([

 libtimecode configured:
 -----------------------

  version:             $VERSION
  interface revision:  $VERSION_INFO

  doxygen:             $DOXYGEN
  installation prefix: $prefix

 type "make" followed my "make install" as root.
 run "make check" to perform selftests.
 $DOXMAKE
])
