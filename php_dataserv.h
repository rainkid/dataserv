/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2010 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header 297205 2010-03-30 21:09:07Z johannes $ */

#ifndef PHP_DATASERV_H
#define PHP_DATASERV_H

extern zend_module_entry dataserv_module_entry;
#define phpext_dataserv_ptr &dataserv_module_entry

#ifdef PHP_WIN32
#	define PHP_DATASERV_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_DATASERV_API __attribute__ ((visibility("default")))
#else
#	define PHP_DATASERV_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(dataserv);
PHP_MSHUTDOWN_FUNCTION(dataserv);
PHP_RINIT_FUNCTION(dataserv);
PHP_RSHUTDOWN_FUNCTION(dataserv);
PHP_MINFO_FUNCTION(dataserv);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(dataserv)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(dataserv)
*/

/* In every utility function you add that needs to use variables 
   in php_dataserv_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as DATASERV_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define DATASERV_G(v) TSRMG(dataserv_globals_id, zend_dataserv_globals *, v)
#else
#define DATASERV_G(v) (dataserv_globals.v)
#endif

#endif	/* PHP_DATASERV_H */

#include <string.h>

#define DS_MEMBERS "MEMBERS"
#define DS_THREADS "THREADS"
#define DS_TMSGS "TMSGS"

#define DSV_MEMBERS 0
#define DSV_THREADS 1
#define DSV_TMSGS 2

#define DATATYPE(module_name) DSV_##module_name


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
