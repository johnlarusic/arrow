#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([Arrow], [2.0], [johnlr@gmail.com])
AC_CONFIG_AUX_DIR(config)

AM_INIT_AUTOMAKE(arrow, 2.0)

AC_CONFIG_SRCDIR([lib/arrow.h])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_CC_C_O

# Checks for libraries.

# Ask user for path to concorde stuff:
AC_ARG_WITH(concorde,
    [ --with-concorde=<path> directory containing concorde.a/concorde.h],
    [CONCORDE_PREFIX=$with_concorde],
    AC_MSG_ERROR([You must call configure with the --with-concorde option.
    This tells configure where to find the Concorde library and header.
    e.g. --with-concorde=/usr/local])
)
AC_SUBST(CONCORDE_PREFIX)
CONCORDE_LIB="${CONCORDE_PREFIX}/concorde.a ${CONCORDE_PREFIX}/qsopt.a "
CONCORDE_CFLAGS="-I${CONCORDE_PREFIX}"
AC_SUBST(CONCORDE_LIB)
AC_SUBST(CONCORDE_CFLAGS)

# Checks for check test framework stuff:
# PKG_CHECK_MODULES([CHECK], [check >= 0.9.5])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h limits.h malloc.h netdb.h netinet/in.h stddef.h stdlib.h string.h strings.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([dup2 gethostname memset socket])

AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile])
# AC_CONFIG_SUBDIRS([config-old])
AC_OUTPUT
