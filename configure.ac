AC_INIT([collectd-agent],[1.0.1])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([.])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_CONFIG_SRCDIR([src/main.c])
AC_PROG_CC
AC_PROG_LEX
AC_PROG_YACC

AC_CHECK_LIB(netsnmp,init_snmp, [],
   AC_MSG_ERROR("Can't find Net-SNMP library"))

CFLAGS="-Wall -g"

SNMP_CONFIG="net-snmp-config"
SNMP_CFLAGS=""
SNMP_LIBS=""
SNMP_FOUND="no"

PKG_CHECK_MODULES(SNMP, net-snmp, [SNMP_FOUND=yes], [SNMP_FOUND=no])

if test "$SNMP_FOUND" = "no" ; then
   AC_MSG_CHECKING(net-snmp $SNMP_CONFIG)
   if ! $SNMP_CONFIG --version > /dev/null 2>&1 ; then
      AC_MSG_ERROR([Could not find $SNMP_CONFIG anywhere (see config.log for details)])
   fi
   SNMP_LIBS="`$SNMP_CONFIG --agent-libs`"
   SNMP_CFLAGS="`$SNMP_CONFIG --base-cflags`"
   AC_MSG_RESULT([yes])
fi

AC_CHECK_HEADERS(net-snmp/net-snmp-config.h)

AC_ARG_WITH(libcollectdclient, [AS_HELP_STRING([--with-libcollectdclient@<:@=PREFIX@:>@], [Path to libcollectdclient.])],
[
	if test "x$withval" != "xno" && test "x$withval" != "xyes"
	then
		CFLAGS="$CFLAGS -I$withval/include"
		CPPFLAGS="$CPPFLAGS -I$withval/include"
		LDFLAGS="$LDFLAGS -L$withval/lib"
	fi
],[])

AC_CHECK_HEADERS(collectd/client.h, [], 
    AC_MSG_ERROR("Can't locate collectd client headers"))

AC_CHECK_LIB(collectdclient, lcc_getval, [], 
    AC_MSG_ERROR("Can't locate collectd client library"))

AC_OUTPUT
