#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([nmtkit], [0.1.0], [yus.takara@gmail.com])
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR([src/bin/train.cc])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_LANG([C++])
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_CC

#AX_CXX_COMPILE_STDCXX_11
#AX_CHECK_COMPILE_FLAG([-std=c++11], [CXXFLAGS="$CXXFLAGS -std=c++11"])
CXXFLAGS="$CXXFLAGS -std=c++11"

CXXFLAGS="$CXXFLAGS -Wall"
CXXFLAGS="$CXXFLAGS -Werror"
CXXFLAGS="$CXXFLAGS -O2"
#CXXFLAGS="$CXXFLAGS -pg"

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/bin/Makefile
                 src/include/Makefile
                 src/lib/Makefile
                 src/test/Makefile
                 submodules/Makefile
])

AC_CONFIG_MACRO_DIR([m4])

AX_BOOST_BASE([1.50], , AC_MSG_ERROR([Boost 1.50 or later is required]))
AX_BOOST_FILESYSTEM
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_SERIALIZATION
AX_BOOST_SYSTEM

AX_CUDA
AX_EIGEN
AX_DYNET

AC_CONFIG_SUBDIRS([submodules/mteval])

AC_OUTPUT
