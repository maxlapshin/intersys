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


int run(int error, char* file, int line);
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