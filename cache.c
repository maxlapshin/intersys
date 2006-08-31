#include "cache.h"

VALUE mCache, cDatabase, cQuery, cObject, cDefinition, cProperty, cMethod, cArgument, cObjectNotFound, cStatus;
VALUE cTime, cMarshallError, cUnMarshallError;


void Init_cache() {

	rb_define_method(rb_cString, "to_wchar", string_to_wchar, 0);
	rb_define_method(rb_cString, "from_wchar", string_from_wchar, 0);


	mCache = rb_define_module("Cache");
	cObjectNotFound = rb_const_get(mCache, rb_intern("ObjectNotFound"));
	cMarshallError = rb_const_get(mCache, rb_intern("MarshallError"));
	cUnMarshallError = rb_const_get(mCache, rb_intern("UnMarshallError"));
	cStatus = rb_const_get(mCache, rb_intern("Status"));
	cTime = rb_const_get(rb_cObject, rb_intern("Time"));


	cDatabase = rb_define_class_under(mCache, "Database", rb_cObject);
	rb_define_alloc_func(cDatabase, cache_base_s_allocate);
	rb_define_method(cDatabase, "initialize", cache_base_initialize, 1);
	rb_define_method(cDatabase, "connect", cache_base_connect, 1);
	rb_define_method(cDatabase, "begin_db_transaction", cache_base_start, 0);
	rb_define_method(cDatabase, "start", cache_base_start, 0);
	rb_define_method(cDatabase, "commit_db_transaction", cache_base_commit, 0);
	rb_define_method(cDatabase, "commit", cache_base_commit, 0);
	rb_define_method(cDatabase, "rollback_db_transaction", cache_base_rollback, 0);
	rb_define_method(cDatabase, "rollback", cache_base_rollback, 0);
	rb_define_method(cDatabase, "level", cache_base_level, 0);

	cQuery = rb_define_class_under(mCache, "Query", rb_cObject);
	rb_define_alloc_func(cQuery, cache_query_s_allocate);
	rb_define_method(cQuery, "native_initialize", cache_query_initialize, 2);
	rb_define_method(cQuery, "execute", cache_query_execute, 0);
	rb_define_method(cQuery, "fetch", cache_query_fetch, 0);
	rb_define_method(cQuery, "get_data", cache_query_get_data, 1);
	rb_define_method(cQuery, "close", cache_query_close, 0);

	cObject = rb_define_class_under(mCache, "Object", rb_cObject);
	rb_define_alloc_func(cObject, cache_object_s_allocate);
	rb_define_method(cObject, "initialize", cache_object_initialize, 0);
	rb_define_singleton_method(cObject, "create_intern", cache_object_create, 2);
	rb_define_singleton_method(cObject, "open_intern", cache_object_open_by_id, 3);
	rb_define_method(cObject, "intern_methods", cache_object_methods, 0);
	rb_define_method(cObject, "intern_properties", cache_object_properties, 0);
	rb_define_method(cObject, "intern_get", cache_object_get, 1);
	rb_define_method(cObject, "intern_set", cache_object_set, 2);
	rb_define_method(cObject, "intern_result", cache_object_result, 2);
	rb_define_method(cObject, "intern_param", cache_object_param, 2);

	cDefinition = rb_const_get(mCache, rb_intern("Definition"));
	rb_define_alloc_func(cDefinition, cache_definition_s_allocate);
	rb_define_method(cDefinition, "intern_initialize", cache_definition_initialize, 3);
	rb_define_method(cDefinition, "cpp_type", cache_definition_cpp_type, 0);
	rb_define_method(cDefinition, "cache_type", cache_definition_cache_type, 0);
	rb_define_method(cDefinition, "name", cache_definition_name, 0);
	rb_define_method(cDefinition, "in_name", cache_definition_in_name, 0);

	cProperty = rb_const_get(mCache, rb_intern("Property"));
	rb_define_method(cProperty, "initialize", cache_property_initialize, 3);
	rb_define_method(cProperty, "set_as_result!", cache_property_set_result, 0);

	cMethod = rb_const_get(mCache, rb_intern("Method"));
	rb_define_method(cMethod, "method_initialize", cache_method_initialize, 0);
	rb_define_method(cMethod, "is_func?", cache_method_is_func, 0);
	rb_define_method(cMethod, "is_class_method?", cache_method_is_class_method, 0);
	rb_define_method(cMethod, "num_args", cache_method_num_args, 0);
	rb_define_method(cMethod, "prepare_call!", cache_method_prepare_call, 0);
	rb_define_method(cMethod, "intern_call!", cache_method_call, 1);
	rb_define_method(cMethod, "extract_retval!", cache_method_extract_retval, 1);
	
	cArgument = rb_const_get(mCache, rb_intern("Argument"));
	rb_define_method(cArgument, "initialize", cache_argument_initialize, 4);
	rb_define_method(cArgument, "default", cache_argument_default_value, 0);
	rb_define_method(cArgument, "marshall_dlist_element", cache_argument_marshall_dlist_elem, 1);

}

