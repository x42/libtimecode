check_PROGRAMS = tctest

LIBTIMECODEDIR =../src/
INCLUDES = -I$(srcdir)/$(LIBTIMECODEDIR)

tctest_SOURCES = tctest.c
tctest_LDADD = $(LIBTIMECODEDIR)/libtimecode.la -lm
tctest_CFLAGS=-g -Wall

check: $(check_PROGRAMS)
	 date
	 uname -a
	 @echo "-----------------------------------------------------------------"
	 ./tctest
	 @echo "-----------------------------------------------------------------"
	 @echo "  ${PACKAGE}-${VERSION} passed all tests."
	 @echo "-----------------------------------------------------------------"
