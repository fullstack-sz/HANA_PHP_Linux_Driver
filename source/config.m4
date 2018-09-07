dnl $Id$
dnl config.m4 for extension hdb

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(hdb, for hdb support,
Make sure that the comment is aligned:
[  --with-hdb             Include hdb support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(hdb, whether to enable hdb support,
Make sure that the comment is aligned:
[  --enable-hdb           Enable hdb support])

if test "$PHP_HDB" != "no"; then
    hdb_src_class="\
           conn.cpp \
           util.cpp \
           init.cpp  \
           stmt.cpp  \
           "
    shared_src_class="\
           shared/core_conn.cpp \
           shared/core_results.cpp \
           shared/core_stream.cpp \
           shared/core_init.cpp \ 
           shared/core_stmt.cpp \
           shared/core_util.cpp \
           shared/FormattedPrint.cpp \
           shared/localizationimpl.cpp \
           shared/StringFunctions.cpp \
           "

  hdb_shared_path="./shared/"
  hdb_common_path="./common/"
  CXXFLAGS="$CXXFLAGS -std=c++11"
  dnl CXXFLAGS="$CXXFLAGS -D_FORTIFY_SOURCE=2 -O2"
  CXXFLAGS="$CXXFLAGS -fstack-protector"

  HOST_OS_ARCH=`uname`
  if test "${HOST_OS_ARCH}" = "Darwin"; then
      HDB_SHARED_LIBADD="$HDB_SHARED_LIBADD -Wl,-bind_at_load"
      MACOSX_DEPLOYMENT_TARGET=`sw_vers -productVersion`
  else
      HDB_SHARED_LIBADD="$HDB_SHARED_LIBADD -Wl,-z,now"
  fi

  PHP_REQUIRE_CXX()
  PHP_ADD_LIBRARY(stdc++, 1, HDB_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(odbcHDB, "common/odbc", HDB_SHARED_LIBADD)
  dnl PHP_ADD_LIBRARY(odbcHDB, 1, HDB_SHARED_LIBADD)
  PHP_SUBST(HDB_SHARED_LIBADD)
  AC_DEFINE(HAVE_HDB, 1, [ ])
  PHP_ADD_INCLUDE($hdb_shared_path)
  PHP_ADD_INCLUDE($hdb_common_path)
  dnl PHP_SUBST(HDB_SHARED_LIBADD)

  PHP_NEW_EXTENSION(hdb, $hdb_src_class $shared_src_class, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1 -std=c++11)
  PHP_ADD_BUILD_DIR([$ext_builddir/shared], 1)
fi
