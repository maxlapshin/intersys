#ifndef __cache_ruby
#define __cache_ruby

#include "ruby.h"
#include "rubyio.h"
#include "intern.h"
#include <c_api.h>

struct rbDatabase {
	h_connection connection;
	h_database database;
};

struct rbQuery {
	h_query query;
};

struct rbObject {
	h_database database;
	int oref;
	VALUE class_name;
};

struct rbProperty {
	h_database database;
    h_prop_def prop_def;
    short cpp_type;
	h_class_def cl_def;
    const wchar_t* cache_type;
    const wchar_t* name;
	const wchar_t* in_name;
};



int run(int error, char* file, int line);
#define RUN(x) run((x), __FILE__, __LINE__)
#define QUERY_RUN(x) {int sql_code = 0; (x); run(sql_code, __FILE__, __LINE__);}
#define STR(x) (RSTRING(x)->ptr)
#define LEN(x) (RSTRING(x)->len)

#define WCHARSTR(x) ((wchar_t *)STR(x))

#define FROMWCHAR(x) (rb_funcall(x, rb_intern("from_wchar"), 0))
#define TOWCHAR(x) (rb_funcall(x, rb_intern("to_wchar"), 0))

#define FROMWCSTR(x) (FROMWCHAR(wcstr_new(x))) // From wchar_t* -> VALUE with utf8
#define PRINTABLE(x) STR(FROMWCHAR(x))         // from VALUE with ucs -> char * with utf8
#define CLASS_NAME(x) WCHARSTR(x->class_name)  // from object description -> wchar_t*

#endif /* __cache_ruby */