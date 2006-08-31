#ifndef __cache_ruby
#define __cache_ruby

#include "ruby.h"
#include "rubyio.h"
#include "intern.h"
#include <time.h>
#include <c_api.h>

struct rbDatabase {
	h_connection connection;
	h_database database;
};

struct rbQuery {
	h_query query;
	bool_t empty;
};

struct rbObject {
	h_database database;
	int oref;
	VALUE class_name;
};

struct rbDefinition {
	int type;
	h_database database;
    void *def;
    short cpp_type;
	h_class_def cl_def;
    const wchar_t* cache_type;
    const wchar_t* name;
	wchar_t* in_name;
	// Method definitions
	bool_t is_func;
	bool_t is_class_method;
	int num_args;
	void *args_info;
	int arg_counter;
	//Argument definitions
	bool_t is_by_ref;
	bool_t is_default;
	const char* default_value;
	long default_value_size;
	int arg_number;
	
	char* current_dlist;
	int current_dlist_size;
};

enum { D_PROPERTY, D_METHOD, D_ARGUMENT};

enum {
	ERR_CONN_LINK_FAILURE = 461
};


extern VALUE mCache, cDatabase, cQuery, cObject, cDefinition, cProperty, cMethod, cArgument, cObjectNotFound, cStatus;
extern VALUE cTime, cMarshallError, cUnMarshallError;


/****** Common functions ******/

VALUE string_to_wchar(VALUE self);
int run(int error, char* file, int line);
VALUE string_from_wchar(VALUE self);
VALUE wcstr_new(const wchar_t *w_str);

/******* Database functions *******/

VALUE intersys_base_s_allocate(VALUE klass);
VALUE intersys_base_initialize(VALUE self, VALUE options);
VALUE intersys_base_connect(VALUE self, VALUE options);
VALUE intersys_base_start(VALUE self);
VALUE intersys_base_commit(VALUE self);
VALUE intersys_base_rollback(VALUE self);
VALUE intersys_base_level(VALUE self);


/******* Query functions *******/

VALUE intersys_query_s_allocate(VALUE klass);
VALUE intersys_query_initialize(VALUE self, VALUE database, VALUE sql_query);
VALUE intersys_query_execute(VALUE self);
VALUE intersys_query_column_name(h_query query, int i);
VALUE intersys_query_get_data(VALUE self, VALUE index);
VALUE intersys_query_fetch(VALUE self);
VALUE intersys_query_close(VALUE self);



/******* Object functions *******/

VALUE intersys_object_s_allocate(VALUE klass);
VALUE intersys_object_initialize(VALUE self);
VALUE intersys_object_open_by_id(VALUE self, VALUE r_database, VALUE name, VALUE oid);
VALUE intersys_object_create(VALUE self, VALUE r_database, VALUE name);
VALUE intersys_object_methods(VALUE self);
VALUE intersys_object_properties(VALUE self);
VALUE intersys_object_get(VALUE self, VALUE r_property);
VALUE intersys_object_set(VALUE self, VALUE r_property, VALUE value);
VALUE intersys_object_result(VALUE self, VALUE index, VALUE r_property);
VALUE intersys_object_param(VALUE self, VALUE obj, VALUE r_property);


/******* Properties, definitions, arguments, methods functions *******/

VALUE intersys_definition_s_allocate(VALUE klass);
VALUE intersys_definition_initialize(VALUE self, VALUE r_database, VALUE class_name, VALUE name);
VALUE intersys_definition_cpp_type(VALUE self);
VALUE intersys_definition_cache_type(VALUE self);
VALUE intersys_definition_name(VALUE self);
VALUE intersys_definition_in_name(VALUE self);
VALUE intersys_property_initialize(VALUE self, VALUE r_database, VALUE class_name, VALUE name);
VALUE intersys_property_set_result(VALUE self);
VALUE intersys_method_initialize(VALUE self);
VALUE intersys_method_is_func(VALUE self);
VALUE intersys_method_is_class_method(VALUE self);
VALUE intersys_method_num_args(VALUE self);
VALUE intersys_method_prepare_call(VALUE self);
VALUE intersys_method_call(VALUE self, VALUE r_object);
VALUE intersys_method_extract_retval(VALUE self, VALUE r_object);
VALUE intersys_argument_initialize(VALUE self, VALUE r_database, VALUE class_name, VALUE name, VALUE r_method);
VALUE intersys_argument_default_value(VALUE self);
VALUE intersys_argument_marshall_dlist_elem(VALUE self, VALUE elem);







#define RUN(x) run((x), __FILE__, __LINE__)
#define QUERY_RUN(x) {int sql_code = 0; (x); run(sql_code, __FILE__, __LINE__);}
#define STR(x) (RSTRING(x)->ptr)
#define LEN(x) (RSTRING(x)->len)
#define CALL(x, method) (rb_funcall((x), rb_intern(method), 0))

#define WCHARSTR(x) ((wchar_t *)STR(x))

#define FROMWCHAR(x) (rb_funcall((x), rb_intern("from_wchar"), 0))
#define TOWCHAR(x) (rb_funcall((x), rb_intern("to_wchar"), 0))

#define FROMWCSTR(x) (FROMWCHAR(wcstr_new(x))) // From wchar_t* -> VALUE with utf8
#define PRINTABLE(x) STR(FROMWCHAR(x))         // from VALUE with ucs -> char * with utf8
#define CLASS_NAME(x) WCHARSTR((x)->class_name)  // from object description -> wchar_t*

#define DLISTSIZE (argument->current_dlist_amount - argument->current_dlist_size)
#define PUT_DLIST(func, value) RUN(func(argument->current_dlist, DLISTSIZE, value, &elem_size))


#endif /* __cache_ruby */