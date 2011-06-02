dnl $Id$
dnl config.m4 for extension dataserv

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(dataserv, for dataserv support,
dnl Make sure that the comment is aligned:
dnl [  --with-dataserv             Include dataserv support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(dataserv, whether to enable dataserv support,
[  --enable-dataserv           Enable dataserv support])

if test "$PHP_DATASERV" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-dataserv -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/dataserv.h"  # you most likely want to change this
  dnl if test -r $PHP_DATASERV/$SEARCH_FOR; then # path given as parameter
  dnl   DATASERV_DIR=$PHP_DATASERV
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for dataserv files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       DATASERV_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$DATASERV_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the dataserv distribution])
  dnl fi

  dnl # --with-dataserv -> add include path
  dnl PHP_ADD_INCLUDE($DATASERV_DIR/include)

  dnl # --with-dataserv -> check for lib and symbol presence
  dnl LIBNAME=dataserv # you may want to change this
  dnl LIBSYMBOL=dataserv # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $DATASERV_DIR/lib, DATASERV_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_DATASERVLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong dataserv lib version or lib not found])
  dnl ],[
  dnl   -L$DATASERV_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(DATASERV_SHARED_LIBADD)

  PHP_NEW_EXTENSION(dataserv, dataserv.c, $ext_shared)
fi
