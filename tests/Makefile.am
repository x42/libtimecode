check_PROGRAMS = tctest

LIBTIMECODEDIR =../src/
INCLUDES = -I$(srcdir)/$(LIBTIMECODEDIR)

tctest_SOURCES = tctest.c
tctest_LDADD = $(LIBTIMECODEDIR)/libtimecode.la -lm
tctest_CFLAGS=-g -Wall

check: $(check_PROGRAMS)
