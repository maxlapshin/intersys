#include "intersys.h"

VALUE mIntersys, cDatabase, cObject, cDefinition, cProperty, cMethod, cArgument, cGlobal;
VALUE cTime, cDate, cMarshallError, cUnMarshallError, cObjectNotFound, cIntersysException, cStatus;

#ifdef HAVE_SQL_H
VALUE cQuery;
#endif

static VALUE wchar_t_size(VALUE self) {	
	return INT2FIX(sizeof(wchar_t)); 
}
static VALUE wchar_t_size_type(VALUE self) {	
	return rb_str_new2(sizeof(wchar_t) == 4 ? "L" : sizeof(wchar_t) == 2 ? "s" : "c"); 
}


void Init_intersys_cache() {
	rb_define_method(rb_cString, "to_wchar", string_to_wchar, 0);
	rb_define_method(rb_cString, "from_wchar", string_from_wchar, 0);

	cTime = rb_const_get(rb_cObject, rb_intern("Time"));
	cDate = rb_const_get(rb_cObject, rb_intern("Date"));

	mIntersys = rb_define_module("Intersys");
	rb_define_singleton_method(mIntersys, "__wchar_t_size", wchar_t_size, 0);
	rb_define_singleton_method(mIntersys, "__wchar_t_size_type", wchar_t_size_type, 0);
	
	cIntersysException = rb_define_class_under(mIntersys, "IntersysException", rb_eStandardError);
	cObjectNotFound = rb_define_class_under(mIntersys, "ObjectNotFound", cIntersysException);
	cMarshallError = rb_define_class_under(mIntersys, "MarshallError", cIntersysException);
	cUnMarshallError = rb_define_class_under(mIntersys, "UnMarshallError", cIntersysException);
	
	
	cStatus = rb_define_class_under(mIntersys, "Status", cIntersysException);
	rb_define_alloc_func(cStatus, intersys_status_s_allocate);
	rb_define_method(cStatus, "initialize", intersys_status_initialize, 2);
	rb_define_method(cStatus, "code", intersys_status_code, 0);
	rb_define_method(cStatus, "message", intersys_status_message, 0);
	rb_define_method(cStatus, "to_s", intersys_status_to_s, 0);


	cDatabase = rb_define_class_under(mIntersys, "Database", rb_cObject);
	rb_define_alloc_func(cDatabase, intersys_base_s_allocate);
	rb_define_method(cDatabase, "initialize", intersys_base_initialize, 1);
	rb_define_method(cDatabase, "connect", intersys_base_connect, 1);
	rb_define_method(cDatabase, "begin_db_transaction", intersys_base_start, 0);
	rb_define_method(cDatabase, "start", intersys_base_start, 0);
	rb_define_method(cDatabase, "commit_db_transaction", intersys_base_commit, 0);
	rb_define_method(cDatabase, "commit", intersys_base_commit, 0);
	rb_define_method(cDatabase, "rollback_db_transaction", intersys_base_rollback, 0);
	rb_define_method(cDatabase, "rollback", intersys_base_rollback, 0);
	rb_define_method(cDatabase, "level", intersys_base_level, 0);
	rb_define_method(cDatabase, "close!", intersys_base_close, 0);

#ifdef HAVE_SQL_H
	cQuery = rb_define_class_under(mIntersys, "Query", rb_cObject);
	rb_define_alloc_func(cQuery, intersys_query_s_allocate);
	rb_define_method(cQuery, "initialize", intersys_query_initialize, 2);
	rb_define_method(cQuery, "bind_params", intersys_query_bind_params, 1);
	rb_define_method(cQuery, "execute", intersys_query_execute, 0);
	rb_define_method(cQuery, "fetch", intersys_query_fetch, 0);
	rb_define_method(cQuery, "each", intersys_query_each, 0);
	rb_define_method(cQuery, "column_name", intersys_query_column_name, 1);
	rb_define_method(cQuery, "get_data", intersys_query_get_data, 1);
	rb_define_method(cQuery, "close", intersys_query_close, 0);
	rb_define_method(cQuery, "limit", intersys_query_get_limit, 0);
	rb_define_method(cQuery, "limit=", intersys_query_set_limit, 1);
	rb_define_method(cQuery, "offset", intersys_query_get_offset, 0);
	rb_define_method(cQuery, "offset=", intersys_query_set_offset, 1);
#endif

	cObject = rb_define_class_under(mIntersys, "Object", rb_cObject);
	rb_define_alloc_func(cObject, intersys_object_s_allocate);
	rb_define_method(cObject, "initialize", intersys_object_initialize, 0);
	rb_define_singleton_method(cObject, "create", intersys_object_create, 0);
	rb_define_singleton_method(cObject, "open", intersys_object_open_by_id, 1);

	cDefinition = rb_define_class_under(mIntersys, "Definition", rb_cObject);
	rb_define_alloc_func(cDefinition, intersys_definition_s_allocate);
	rb_define_method(cDefinition, "initialize", intersys_definition_initialize, 3);
	rb_define_method(cDefinition, "cpp_type", intersys_definition_cpp_type, 0);
	rb_define_method(cDefinition, "cpp_name", intersys_definition_cpp_name, 0);
	rb_define_method(cDefinition, "cache_type", intersys_definition_cache_type, 0);
	rb_define_method(cDefinition, "name", intersys_definition_name, 0);
	rb_define_method(cDefinition, "in_name", intersys_definition_in_name, 0);

	cProperty = rb_define_class_under(mIntersys, "Property", cDefinition);
	rb_define_method(cProperty, "initialize", intersys_property_initialize, 4);
	rb_define_method(cProperty, "get", intersys_property_get, 0);
	rb_define_method(cProperty, "set", intersys_property_set, 1);

	cMethod = rb_define_class_under(mIntersys, "Method", cDefinition);
	rb_define_method(cMethod, "initialize", intersys_method_initialize, 4);
	rb_define_method(cMethod, "func?", intersys_method_is_func, 0);
	rb_define_method(cMethod, "class_method?", intersys_method_is_class_method, 0);
	rb_define_method(cMethod, "num_args", intersys_method_num_args, 0);
	rb_define_method(cMethod, "each_argument", intersys_method_each_argument, 0);
	rb_define_method(cMethod, "call!", intersys_method_call, 1);
	
	cArgument = rb_define_class_under(mIntersys, "Argument", cDefinition);
	rb_define_method(cArgument, "initialize", intersys_argument_initialize, 4);
	rb_define_method(cArgument, "default", intersys_argument_default_value, 0);
	rb_define_method(cArgument, "by_ref?", intersys_argument_is_by_ref, 0);
	rb_define_method(cArgument, "marshall_dlist_element", intersys_argument_marshall_dlist_elem, 1);
	rb_define_method(cArgument, "marshall_dlist", intersys_argument_marshall_dlist, 1);
	/*
	rb_eval_string(
	"class Argument \n" \
    "def marshall_dlist(list)\n" \
    "  list.each do |elem|\n" \
    "    marshall_dlist_element(elem)\n" \
    "  end\n" \
    "end\n" \
  	"end");
	*/
	
	cGlobal = rb_define_class_under(mIntersys, "Global", rb_cObject);
	rb_define_alloc_func(cGlobal, intersys_global_s_allocate);
	rb_define_method(cGlobal, "initialize", intersys_global_initialize, 1);
	rb_define_method(cGlobal, "[]=", intersys_global_set, -1);
	rb_define_method(cGlobal, "[]", intersys_global_get, -1);
	rb_define_method(cGlobal, "delete", intersys_global_delete, -1);
	rb_define_method(cGlobal, "name", intersys_global_name, 0);
}

