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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "main/SAPI.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_alloc.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "zend_objects.h"

#include "php_dataserv.h"

#include <mysql/mysql.h>

#include <time.h>
#include <errno.h>

/* If you declare any globals in php_dataserv.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(dataserv)*/


/* True global resources - no need for thread safety here */
static int le_dataserv;

zend_class_entry * data_serv_ce;

PHP_METHOD(dataserv, __construct) {
	php_printf("%s", "starting creat data.\n");
}

PHP_METHOD(dataserv, __destruct) {
	php_printf("%s", "data creating completed.\n");
}

static void get_rand_str( char* buf, int min, int max)
{
    int i,len = rand()%(max-min+1) + min;
    char* p = buf;
    buf[0] = '\0';
    for( i = 0 ; i < len ; ++ i )
    {
        *(p + i) = 'a' + rand()%26;
    }
    *(p+i) = '\0';
}

static int trim(char* szStr,const char ch, int iMode)
{
    if (szStr == NULL)
    {
        return 1;
    }
    char szTmp[1024*10] = { 0x00 };
    strcpy(szTmp, szStr);
    int iLen = strlen(szTmp);
    char* pStart = szTmp;
    char* pEnd = szTmp+iLen;
    int i;
    for(i = 0;i < iLen;i++){
        if (szTmp[i] == ch && pStart == szTmp+i && iMode != 2)
        {
            ++pStart;
        }
        if (szTmp[iLen-i-1] == ch && pEnd == szTmp+iLen-i && iMode != 1)
        {
            *(--pEnd) = '\0';
        }
    }
    strcpy(szStr, pStart);
    return 0;
}

static int getfields(zval * fields, char * output){
	zval ** data;
	HashTable *fields_hash;
	HashPosition pointer;
	int fields_count;

	fields_hash = Z_ARRVAL_P(fields);
	fields_count = zend_hash_num_elements(fields_hash);

	char *key;
	int key_len;
	long index;
	zval ** type, ** min, ** max;
	char *temp;

	for(zend_hash_internal_pointer_reset_ex(fields_hash, &pointer);
			zend_hash_get_current_data_ex(fields_hash, (void**) &data, &pointer) == SUCCESS;
			zend_hash_move_forward_ex(fields_hash, &pointer))
	 {
		 if (zend_hash_get_current_key_ex(fields_hash, &key, &key_len,&index, 0, &pointer) == HASH_KEY_IS_STRING)
		 {
			 switch(Z_TYPE_P(*data)){
			 case IS_ARRAY:
				 if (zend_hash_index_find(Z_ARRVAL_PP(data), 0, (void**)&type) == FAILURE)
				 {
					 php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong value with:%s",key);
				 }
				 if (zend_hash_index_find(Z_ARRVAL_PP(data), 1, (void**)&min) == FAILURE)
				 {
					 php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong value with:%s",key);
				 }
				 if (zend_hash_index_find(Z_ARRVAL_PP(data), 2, (void**)&max) == FAILURE)
				 {
					 php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong value with:%s",key);
				 }
				 if(strcmp(Z_STRVAL_PP(type), "int") == 0)
				 {
					 temp = (char *)emalloc(Z_LVAL_PP(max) * sizeof(int)+2);
					 sprintf(temp, "%ld,", Z_LVAL_PP(min) + rand() % (Z_LVAL_PP(max) - Z_LVAL_PP(min)));
					 strcat(output, temp);
				 }else if(strcmp(Z_STRVAL_PP(type), "string") == 0)
				 {
					 temp = (char *)emalloc(Z_LVAL_PP(max) * sizeof(char) + 1);
					 get_rand_str(temp, Z_LVAL_PP(min), Z_LVAL_PP(max));
					 strcat(output, temp);
					 strcat(output, ",");
				 }
				 break;
			 case IS_STRING:
				 temp = (char *)emalloc((Z_STRLEN_PP(data)) * sizeof(char) + 2);
				 sprintf(temp, "%s,", Z_STRVAL_PP(data));
				 strcat(output, temp);
				 break;
			 case IS_LONG:
				 temp = (char *)emalloc((Z_STRLEN_PP(data)) * sizeof(long) + 2);
				 sprintf(temp, "%ld,", Z_LVAL_PP(data));
				 strcat(output, temp);
				 break;
			 case IS_DOUBLE:
				 temp = (char *)emalloc((Z_STRLEN_PP(data)) * sizeof(double) + 2);
				 sprintf(temp, "%f,", Z_DVAL_PP(data));
				 strcat(output, temp);
				 break;
			 default:
				 break;
			 }
			 efree(temp);
		 }
	 }
	 trim(output,',',2);
}


PHP_METHOD(dataserv, create) {

	zval * self = getThis();
	zval * z_start, * z_end, * retval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz",  &z_start, &z_end) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	zend_update_property(Z_OBJCE_P(self), self, ZEND_STRL("start"), z_start TSRMLS_CC);
	zend_update_property(Z_OBJCE_P(self), self, ZEND_STRL("end"), z_end TSRMLS_CC);

	zend_call_method_with_0_params(&self, Z_OBJCE_P(self), NULL, "insdata", &retval);
}

PHP_METHOD(dataserv, sethost) {

	zval * self = getThis();
	zval * z_host;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z_host) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	zend_update_property(Z_OBJCE_P(self), self, ZEND_STRL("host"), z_host TSRMLS_CC);
}

PHP_METHOD(dataserv, setuser) {

	zval * self = getThis();
	zval * z_user;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z_user) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	zend_update_property(Z_OBJCE_P(self), self, ZEND_STRL("user"), z_user TSRMLS_CC);
}

PHP_METHOD(dataserv, setpasswd) {

	zval * self = getThis();
	zval * z_passwd;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z_passwd) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	zend_update_property(Z_OBJCE_P(self), self, ZEND_STRL("passwd"), z_passwd TSRMLS_CC);
}

PHP_METHOD(dataserv, setdb) {

	zval * self = getThis();
	zval * z_db;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z_db) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	zend_update_property(Z_OBJCE_P(self), self, ZEND_STRL("db"), z_db TSRMLS_CC);
}

PHP_METHOD(dataserv, settable) {

	zval * self = getThis();
	zval * z_table;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z_table) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	zend_update_property(Z_OBJCE_P(self), self, ZEND_STRL("table"), z_table TSRMLS_CC);
}

PHP_METHOD(dataserv, setfields){
	zval * self = getThis();
	zval * z_fields;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z_fields) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	zend_update_property(Z_OBJCE_P(self), self, ZEND_STRL("fields"), z_fields TSRMLS_CC);
}

PHP_METHOD(dataserv, insdata) {
	int i=0, fstart=1, fend=1;
	char * table;
	zval * self = getThis();
	zval * fields;

	table = Z_STRVAL_P(zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("table"), 0 TSRMLS_CC));
	fstart = Z_LVAL_P(zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("start"), 0 TSRMLS_CC));
	fend = Z_LVAL_P(zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("end"), 0 TSRMLS_CC));
	fields = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("fields"), 0 TSRMLS_CC);

	FILE * fp = NULL;
	char path[1024]={0x00};
	sprintf(path, "%s.sql", table);
	fp = VCWD_FOPEN( path, "ab+" );

	if( fp == NULL )
	{
		php_printf("Open file %s fail:%d.\n", path, errno );
		return;
	}

	fend = fstart+fend;
	for( i = fstart ; i <= fend; ++ i )
	{
		char *output;
		output = (char *)emalloc(1024 * 100 * sizeof(char));
		memset(output, 0x00, 1024 * 100 * sizeof(char));
		getfields(fields, output);
		fprintf(fp,"%s\n",output);
		efree(output);
		if(i%5 == 0) printf("has %d insert into the files.\n", i);
	}
//	printf("LOAD DATA INFILE '$datadir' INTO TABLE $tablename FIELDS TERMINATED BY ',' ENCLOSED BY '\"' LINES STARTING BY '\\n';\n");
	fclose(fp);
}

PHP_METHOD(dataserv, loaddata) {
	zval * self = getThis();
	char * host, * user, * passwd, * db, * table;
	host = Z_STRVAL_P(zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("host"), 0 TSRMLS_CC));
	user = Z_STRVAL_P(zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("user"), 0 TSRMLS_CC));
	passwd = Z_STRVAL_P(zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("passwd"), 0 TSRMLS_CC));
	db = Z_STRVAL_P(zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("db"), 0 TSRMLS_CC));
	table = Z_STRVAL_P(zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("table"), 0 TSRMLS_CC));

	MYSQL mysql_conn; /* Connection handle */
	MYSQL_RES *mysql_result; /* Result handle */
	MYSQL_ROW mysql_row; /* Row data */
	char SQL[1024] = {0x00};

	if (mysql_init(&mysql_conn) == NULL)
	{
		(void) printf("Initialization fails.\n");
		mysql_close(&mysql_conn);
		return;
	}

	if (mysql_real_connect(&mysql_conn, host, user, passwd, db, MYSQL_PORT, NULL, 128) == NULL)
	{
		(void) printf("Connection fails.\n");
		mysql_close(&mysql_conn);
		return;
	}

	sprintf(SQL,"LOAD DATA LOCAL INFILE '%s.sql' INTO TABLE %s.%s FIELDS TERMINATED BY ',' ENCLOSED BY '\"' LINES STARTING BY '\\n'",table,db,table);
//	php_printf("%s\n", SQL);
	if (mysql_query(&mysql_conn, SQL) != 0)
	{
		(void) printf("Query fails.\n");
		mysql_close(&mysql_conn);
		return;
	}
}
/* {{{ dataserv_functions[]
 *
 * Every user visible function must have an entry in dataserv_functions[].
 */
const zend_function_entry dataserv_functions[] = {
	PHP_ME(dataserv, __construct, 	NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(dataserv, __destruct,  	NULL, ZEND_ACC_PUBLIC|ZEND_ACC_DTOR)
	PHP_ME(dataserv, insdata, 	 	 	NULL, ZEND_ACC_PUBLIC)

	PHP_ME(dataserv, sethost, 	 	 	NULL, ZEND_ACC_PUBLIC)
	PHP_ME(dataserv, setuser, 	 	 	NULL, ZEND_ACC_PUBLIC)
	PHP_ME(dataserv, setpasswd, 	 	 	NULL, ZEND_ACC_PUBLIC)
	PHP_ME(dataserv, setdb, 	 	 	NULL, ZEND_ACC_PUBLIC)
	PHP_ME(dataserv, settable, 	 	 	NULL, ZEND_ACC_PUBLIC)
	PHP_ME(dataserv, setfields, 	 	 	NULL, ZEND_ACC_PUBLIC)

	PHP_ME(dataserv, create, 	 	 	NULL, ZEND_ACC_PUBLIC)
	PHP_ME(dataserv, loaddata, 	 	 	NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}	/* Must be the last line in dataserv_functions[] */
};
/* }}} */

/* {{{ dataserv_module_entry
 */
zend_module_entry dataserv_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"dataserv",
	dataserv_functions,
	PHP_MINIT(dataserv),
	PHP_MSHUTDOWN(dataserv),
	PHP_RINIT(dataserv),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(dataserv),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(dataserv),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_DATASERV
ZEND_GET_MODULE(dataserv)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("dataserv.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_dataserv_globals, dataserv_globals)
    STD_PHP_INI_ENTRY("dataserv.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_dataserv_globals, dataserv_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_dataserv_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_dataserv_init_globals(zend_dataserv_globals *dataserv_globals)
{
	dataserv_globals->global_value = 0;
	dataserv_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(dataserv)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/

	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "DataServ", dataserv_functions);

	data_serv_ce = zend_register_internal_class_ex(&ce, 	  NULL, NULL TSRMLS_CC);
	zend_declare_property_null(data_serv_ce, ZEND_STRL("host"),    ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(data_serv_ce, ZEND_STRL("user"),    ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(data_serv_ce, ZEND_STRL("passwd"),    ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(data_serv_ce, ZEND_STRL("db"),    ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(data_serv_ce, ZEND_STRL("table"),    ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(data_serv_ce, ZEND_STRL("start"),    ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(data_serv_ce, ZEND_STRL("end"),    ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(data_serv_ce, ZEND_STRL("fields"),    ZEND_ACC_PUBLIC TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(dataserv)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(dataserv)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(dataserv)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(dataserv)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "dataserv support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_dataserv_compiled(string arg)
   Return a string to confirm that the module is compiled in */



/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
