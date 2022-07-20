#
# $Id$
#

SUBDIRS = .

EXTRA_DIST = htime.3
# gps-dw9.dat

#AM_CPPFLAGS = @HOPS_INC@

# IBEX_ANC=
TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS             = lifetest check_hops_time.sh

noinst_HEADERS = time_float_type.h hops_time.h \
		 htime.h hetetimecode.h isoclog.h julian.h

noinst_LIBRARIES = libhete.a

bin_PROGRAMS = hops_time
check_PROGRAMS = leap_sec_chk lifetest cnvrt_cdf_maia
dist_check_SCRIPTS = check_hops_time.sh

libhete_a_SOURCES = htime.c lifetime.c leapsec.c sc_time.c \
		    julian.c eph_time.c rtype.c

hops_time_SOURCES = hops_time.c ht_parser.c ht_timeis.c
hops_time_DEPENDENCIES = libhete.a
hops_time_LDADD   = -L. -lhete

leap_sec_chk_SOURCES   = leapsec.c htime.c lifetime.c sc_time.c
leap_sec_chk_CPPFLAGS  = -DCALCULATE_DEFAULT_TABLE=1

cnvrt_cdf_maia_SOURCES = cnvrt_cdf_maia.c

lifetest_SOURCES = lifetest.c
lifetest_LDADD = -L. -lhete

#hops_time.1:    hops_time.c
#	$(MAKE) $(AM_MAKEFLAGS) hops_time$(EXEEXT)
#	$(HELP2MAN) -N -o hops_time.1 ./hops_time$(EXEEXT)

# wget https://cdf.gsfc.nasa.gov/html/CDFLeapSeconds.txt
# mv CDFLeapSeconds.txt tai-utc.dat
#
# this target only works if the NAVY maia server is available
tai-utc: leap_sec_chk
	[ -f tai-utc.dat ] || \
	    wget ftp://maia.usno.navy.mil/ser7/tai-utc.dat
	cmp $(srcdir)/tai-utc.dat tai-utc.dat || \
		cp -p tai-utc.dat $(srcdir)
	./leap_sec_chk
#IBEX_OPS="./" IBEX_DATACALSC="./" ./leap_sec_chk

# this is an alternate version of the table from NASA
CDFLeapSeconds: cnvrt_cdf_maia leap_sec_chk
	[ -f CDFLeapSeconds.txt ] || \
	    wget https://cdf.gsfc.nasa.gov/html/CDFLeapSeconds.txt
	cmp $(srcdir)/CDFLeapSeconds.txt CDFLeapSeconds.txt || \
		cp -p CDFLeapSeconds.txt $(srcdir)
	./cnvrt_cdf_maia CDFLeapSeconds.txt tai-utc.dat
	cmp $(srcdir)/tai-utc.dat tai-utc.dat
	LEAP_SEC_DEBUG=1 \
	IBEX_OPS="./" IBEX_DATACALSC="./" ./leap_sec_chk |\
	tee leap-table-d.txt
	sed -e '1,/^.*GPS.before.*dateH.*/d' \
	    -e '/^.*GPS.before.*dateT.*/,$$d' $(srcdir)/leapsec.c \
	    > leap-table-c.txt
	ls -l leap-table-?.txt
	@diff leap-table-c.txt leap-table-d.txt && \
		echo 'download and compiled code agree' || \
		echo 'download disagrees with compiled code'
# leap_sec_chk is clearly not doing what it should....

CLEANFILES = queries.* tai-utc.dat CDFLeap*txt Debug.txt leap-*.txt

#
# eof
#
