#include "intersys.h"

VALUE mIntersys, cDatabase, cQuery, cObject, cDefinition, cProperty, cMethod, cArgument, cObjectNotFound, cStatus;
VALUE cTime, cMarshallError, cUnMarshallError;

void Init_intersys_cache() {
	rb_define_method(rb_cString, "to_wchar", string_to_wchar, 0);
	rb_define_method(rb_cString, "from_wchar", string_from_wchar, 0);


	mIntersys = rb_define_module("Intersys");
	cObjectNotFound = rb_const_get(mIntersys, rb_intern("ObjectNotFound"));
	cMarshallError = rb_const_get(mIntersys, rb_intern("MarshallError"));
	cUnMarshallError = rb_const_get(mIntersys, rb_intern("UnMarshallError"));
	cStatus = rb_const_get(mIntersys, rb_intern("Status"));
	cTime = rb_const_get(rb_cObject, rb_intern("Time"));


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

	cQuery = rb_define_class_under(mIntersys, "Query", rb_cObject);
	rb_define_alloc_func(cQuery, intersys_query_s_allocate);
	rb_define_method(cQuery, "native_initialize", intersys_query_initialize, 2);
	rb_define_method(cQuery, "execute", intersys_query_execute, 0);
	rb_define_method(cQuery, "fetch", intersys_query_fetch, 0);
	rb_define_method(cQuery, "column_name", intersys_query_column_name, 1);
	rb_define_method(cQuery, "get_data", intersys_query_get_data, 1);
	rb_define_method(cQuery, "close", intersys_query_close, 0);

	cObject = rb_define_class_under(mIntersys, "Object", rb_cObject);
	rb_define_alloc_func(cObject, intersys_object_s_allocate);
	rb_define_method(cObject, "initialize", intersys_object_initialize, 0);
	rb_define_singleton_method(cObject, "create_intern", intersys_object_create, 0);
	rb_define_singleton_method(cObject, "open_intern", intersys_object_open_by_id, 1);

	cDefinition = rb_const_get(mIntersys, rb_intern("Definition"));
	rb_define_alloc_func(cDefinition, intersys_definition_s_allocate);
	rb_define_method(cDefinition, "intern_initialize", intersys_definition_initialize, 3);
	rb_define_method(cDefinition, "cpp_type", intersys_definition_cpp_type, 0);
	rb_define_method(cDefinition, "cache_type", intersys_definition_cache_type, 0);
	rb_define_method(cDefinition, "name", intersys_definition_name, 0);
	rb_define_method(cDefinition, "in_name", intersys_definition_in_name, 0);

	cProperty = rb_const_get(mIntersys, rb_intern("Property"));
	rb_define_method(cProperty, "initialize", intersys_property_initialize, 4);
	rb_define_method(cProperty, "get", intersys_property_get, 0);
	rb_define_method(cProperty, "set", intersys_property_set, 1);

	cMethod = rb_const_get(mIntersys, rb_intern("Method"));
	rb_define_method(cMethod, "method_initialize", intersys_method_initialize, 1);
	rb_define_method(cMethod, "func?", intersys_method_is_func, 0);
	rb_define_method(cMethod, "class_method?", intersys_method_is_class_method, 0);
	rb_define_method(cMethod, "num_args", intersys_method_num_args, 0);
	rb_define_method(cMethod, "call!", intersys_method_call, 1);
	
	cArgument = rb_const_get(mIntersys, rb_intern("Argument"));
	rb_define_method(cArgument, "initialize", intersys_argument_initialize, 4);
	rb_define_method(cArgument, "default", intersys_argument_default_value, 0);
	rb_define_method(cArgument, "marshall_dlist_element", intersys_argument_marshall_dlist_elem, 1);
}

