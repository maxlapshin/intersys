#include "cache.h"

const char MODULE_NAME[] = "Cache";
const char DB_NAME[] = "Database";
const char QUERY_NAME[] = "Query";
const char OBJECT_NAME[] = "Object";
const char OBJECTNOTFOUND_NAME[] = "ObjectNotFound";
const char DEFINITION_NAME[] = "Definition";
const char PROPERTY_NAME[] = "Property";
const char METHOD_NAME[] = "Method";
const char ARGUMENT_NAME[] = "Argument";

VALUE mCache, cDatabase, cQuery, cObject, cDefinition, cProperty, cMethod, cArgument, cObjectNotFound;


static VALUE string_to_wchar(VALUE self) {
	wchar_t w_chars[RSTRING(self)->len + 1];
	int size;
	RUN(cbind_utf8_to_uni(RSTRING(self)->ptr, (byte_size_t)RSTRING(self)->len, w_chars, (char_size_t)sizeof(w_chars), &size));
	w_chars[size] = 0;
	return rb_str_new((char *)w_chars, (size+1)*sizeof(wchar_t));
}

static VALUE string_from_wchar(VALUE self) {
	char chars[RSTRING(self)->len + 1];
	int size;
	if(LEN(self) == 0 || !STR(self)) {
		return rb_str_new2("");
	}
    RUN(cbind_uni_to_utf8(WCHARSTR(self), wcslen(WCHARSTR(self)), chars, sizeof(chars), &size));
	chars[size] = 0;
	return rb_str_new(chars, size + 1);
}


int run(int err, char *file, int line) {
	if (err != 0) {
		rb_raise(rb_eStandardError, "Cache error %d: %s in file %s: %d", err, cbind_get_last_err_msg(), file, line);
	}
	return err;
}

static VALUE wcstr_new(const wchar_t *w_str) {
	if(!w_str) {
		return rb_funcall(rb_str_new2(""), rb_intern("to_wchar"), 0);
	}
	return rb_str_new((char *)w_str, wcslen(w_str));
}


static void cache_base_free(struct rbDatabase* base) {
	RUN(cbind_free_db(base->database));
	RUN(cbind_free_conn(base->connection));
	free(base);
}

static VALUE cache_base_s_allocate(VALUE klass) {
	struct rbDatabase* cache_base = ALLOC(struct rbDatabase);
	bzero(cache_base, sizeof(struct rbDatabase));
	return Data_Wrap_Struct(klass, 0, cache_base_free, cache_base);
}

static VALUE cache_base_initialize(VALUE self, VALUE options) {
	return rb_funcall(self, rb_intern("connect"), 1, options);
}

static VALUE connect_get_options(VALUE options, char *opt_name, char *opt_default, int convert) {
	VALUE res = rb_hash_aref(options, ID2SYM(rb_intern(opt_name)));
	if(res == Qnil) {
		res = rb_str_new2(opt_default);
	}
	if(convert) {
		return rb_funcall(res, rb_intern("to_wchar"), 0);	
	}
	return res;
}

static VALUE cache_base_connect(VALUE self, VALUE options) {
	struct rbDatabase* base;
	char conn_str[256];
	wchar_t w_conn_str[256];
	int size;

	VALUE host, port, user, password, namespace, timeout;

	host = connect_get_options(options, "host", "localhost", 0);
	port = connect_get_options(options, "port", "1972", 0);
	namespace = connect_get_options(options, "namespace", "Samples", 0);

	user = connect_get_options(options, "user", "_SYSTEM", 1);
	password = connect_get_options(options, "password", "SYS", 1);
	timeout = rb_hash_aref(options, rb_intern("timeout"));
	if (timeout == Qnil) {
		timeout = INT2FIX(30);
	}

	Data_Get_Struct(self, struct rbDatabase, base);

	snprintf(conn_str, sizeof(conn_str), "%s[%s]:%s", RSTRING(host)->ptr, RSTRING(port)->ptr, RSTRING(namespace)->ptr);

	RUN(cbind_utf8_to_uni(conn_str, (byte_size_t)strlen(conn_str), w_conn_str, (char_size_t)sizeof(w_conn_str),&size));
	w_conn_str[size] = 0;

	RUN(cbind_alloc_conn(w_conn_str, WCHARSTR(user), WCHARSTR(password),
		FIX2INT(timeout), &base->connection));
	RUN(cbind_alloc_db(base->connection, &base->database));
	return self;
}

static VALUE cache_base_start(VALUE self) {
	struct rbDatabase* base;
	Data_Get_Struct(self, struct rbDatabase, base);
	RUN(cbind_tstart(base->database));
	return Qtrue;
}

static VALUE cache_base_commit(VALUE self) {
	struct rbDatabase* base;
	Data_Get_Struct(self, struct rbDatabase, base);
	RUN(cbind_tcommit(base->database));
	return Qtrue;
}

static VALUE cache_base_rollback(VALUE self) {
	struct rbDatabase* base;
	Data_Get_Struct(self, struct rbDatabase, base);
	RUN(cbind_trollback(base->database));
	return Qtrue;
}

static VALUE cache_base_level(VALUE self) {
	struct rbDatabase* base;
	int level;
	Data_Get_Struct(self, struct rbDatabase, base);
	RUN(cbind_tlevel(base->database, &level));
	return INT2FIX(level);
}


/*
 * Query
 */


static void cache_query_free(struct rbQuery* query) {
	RUN(cbind_free_query(query->query));
	free(query);
}

static VALUE cache_query_s_allocate(VALUE klass) {
	struct rbQuery* query = ALLOC(struct rbQuery);
	bzero(query, sizeof(struct rbQuery));
	return Data_Wrap_Struct(klass, 0, cache_query_free, query);
}

static VALUE cache_query_initialize(VALUE self, VALUE database, VALUE sql_query) {
	struct rbQuery* query;
	struct rbDatabase* base;
	Data_Get_Struct(self, struct rbQuery, query);
	Data_Get_Struct(database, struct rbDatabase, base);
	RUN(cbind_alloc_query(base->database, &query->query));
	QUERY_RUN(cbind_prepare_gen_query(query->query, WCHARSTR(sql_query), &sql_code));
	return self;
}

static VALUE cache_query_execute(VALUE self) {
	struct rbQuery* query;
	Data_Get_Struct(self, struct rbQuery, query);
	QUERY_RUN(cbind_query_execute(query->query, &sql_code));
	return self;
}

static VALUE cache_query_column_name(h_query query, int i) {
	int len;
	const wchar_t *res;
	RUN(cbind_query_get_col_name_len(query, i, &len));
	RUN(cbind_query_get_col_name(query, i, &res));
	return rb_funcall(rb_str_new((char *)res, len),rb_intern("from_wchar"), 0);
}

static VALUE cache_query_fetch(VALUE self) {
	struct rbQuery* query;
	VALUE columns, data;
	Data_Get_Struct(self, struct rbQuery, query);
	int num_cols = 0;
	int i = 0;
	int sql_code;

	RUN(cbind_query_fetch(query->query, &sql_code));

	data = rb_ary_new();
	rb_iv_set(self, "@data", data);
	columns = rb_ary_new2(num_cols);
	rb_iv_set(self, "@columns", columns);

	if(sql_code == 100) {
		rb_funcall(self, rb_intern("close"), 0);
		return data;
	}
	if(sql_code) {
		rb_raise(rb_eStandardError, "Error in SQL: %d", sql_code);
	}
	
	RUN(cbind_query_get_num_cols(query->query, &num_cols));
	for(i = 0; i < num_cols ; i++) {
		rb_ary_push(columns, cache_query_column_name(query->query, i));
	}
	
	rb_funcall(self, rb_intern("close"), 0);
	return data;
}

static VALUE cache_query_close(VALUE self) {
	struct rbQuery* query;
	Data_Get_Struct(self, struct rbQuery, query);
	RUN(cbind_query_close(query->query));
	return self;
}


/*
 * Object
 */

static void cache_object_free(struct rbObject* object) {
	if (object->oref) {
		RUN(cbind_object_release(object->database, object->oref));
	}
	free(object);
}

static VALUE cache_object_s_allocate(VALUE klass) {
	struct rbObject* object = ALLOC(struct rbObject);
	bzero(object, sizeof(struct rbObject));
	return Data_Wrap_Struct(klass, 0, cache_object_free, object);
}

static VALUE cache_object_initialize(VALUE self) {
	struct rbObject* object;
	struct rbDatabase* base;
	VALUE klass = rb_funcall(self, rb_intern("class"), 0);
	VALUE database = rb_funcall(klass, rb_intern("database"), 0);
	Data_Get_Struct(self, struct rbObject, object);
	Data_Get_Struct(database, struct rbDatabase, base);
	object->database = base->database;
	return self;
}

static VALUE cache_object_open_by_id(VALUE self, VALUE r_database, VALUE name, VALUE oid) {
	int concurrency = 1;
	int timeout = 5;
	int error;
	struct rbObject* object;
	struct rbDatabase *database;
	VALUE r_object = rb_class_new_instance(0, 0, self);

	Data_Get_Struct(r_database, struct rbDatabase, database);
	Data_Get_Struct(r_object, struct rbObject, object);
	object->database = database->database;
	object->class_name = name;
	
    error = cbind_openid(database->database, CLASS_NAME(object), WCHARSTR(oid), concurrency, timeout, &object->oref);
	switch(error) {
		case 0:
			return r_object;
		case -9: 
			rb_raise(cObjectNotFound, "Object with id %s not found", PRINTABLE(oid));
			return Qnil;
		default: 
			RUN(error);
			return Qnil;
	}
	return r_object;
}

static VALUE cache_object_create(VALUE self, VALUE r_database, VALUE name) {
	wchar_t *init_val = NULL;
	struct rbDatabase *database;
	struct rbObject *object;
	VALUE r_object = rb_class_new_instance(0, 0, self);
	
	Data_Get_Struct(r_database, struct rbDatabase, database);
	Data_Get_Struct(r_object, struct rbObject, object);
	object->database = database->database;
	object->class_name = name;
	
	RUN(cbind_create_new(database->database, CLASS_NAME(object), init_val,&object->oref));
	return r_object;
}


static VALUE cache_object_methods(VALUE self) {
	struct rbObject* object;
	Data_Get_Struct(self, struct rbObject, object);
	return rb_ary_new();
}

static VALUE cache_object_properties(VALUE self) {
	struct rbObject* object;
	Data_Get_Struct(self, struct rbObject, object);
	return rb_ary_new();
}



static VALUE cache_object_get(VALUE self, VALUE r_property) {
	struct rbObject* object;
	struct rbDefinition* property;

	Data_Get_Struct(self, struct rbObject, object);
	Data_Get_Struct(r_property, struct rbDefinition, property);

	rb_funcall(r_property, rb_intern("set_as_result!"), 0);
    RUN(cbind_get_prop(object->database, object->oref, property->in_name));
	return rb_funcall(self, rb_intern("intern_result"), 2, INT2FIX(0), r_property);
}

static VALUE cache_object_set(VALUE self, VALUE r_property, VALUE value) {
	struct rbObject* object;
	struct rbDefinition* property;

	Data_Get_Struct(self, struct rbObject, object);
	Data_Get_Struct(r_property, struct rbDefinition, property);

    RUN(cbind_reset_args(object->database));
	rb_funcall(self, rb_intern("intern_param"), 2, value, r_property);
	RUN(cbind_set_prop(object->database, object->oref, property->in_name));
	return self;
}



static VALUE cache_object_result(VALUE self, VALUE index, VALUE r_property) {
	struct rbObject* object;
	struct rbDefinition* property;
    bool_t is_null;
	int argnum = FIX2INT(index);


	Data_Get_Struct(self, struct rbObject, object);
	Data_Get_Struct(r_property, struct rbDefinition, property);

    
    RUN(cbind_get_is_null(object->database, argnum, &is_null));
	if(is_null) {
		return Qnil;
	}
	
	switch(property->cpp_type) {
		case CBIND_VOID: return Qnil;
		
		case CBIND_STATUS_ID:
		case CBIND_OBJ_ID: 
        case CBIND_TIME_ID:
        case CBIND_DATE_ID:
        case CBIND_TIMESTAMP_ID:
        case CBIND_CURRENCY_ID:
        case CBIND_DLIST_ID:
			rb_raise(rb_eStandardError, "Cannot unmarshall type %d now", property->cpp_type);
			return Qnil;
		
		case CBIND_INT_ID: {
			int val;
			RUN(cbind_get_arg_as_int(object->database, argnum, &val, &is_null));
			return INT2FIX(val);
		}
		
		case CBIND_DOUBLE_ID: {
			double val;
            RUN(cbind_get_arg_as_double(object->database, argnum, &val, &is_null));
			return rb_float_new(val);
		}
		
		case CBIND_BINARY_ID: {
            byte_size_t size;
            char *buf;
			VALUE result = rb_str_new2(0);

            RUN(cbind_get_arg_as_bin(object->database, argnum, NULL, 0, &size, &is_null));
			buf = ALLOC_N(char, size + 1);
            RUN(cbind_get_arg_as_bin(object->database, argnum, buf, size, &size, &is_null));
			
			RSTRING(result)->ptr = buf;
			RSTRING(result)->len = size;
		    RSTRING(result)->aux.capa = size;
			return result;
		}
		
		case CBIND_STRING_ID: {
            byte_size_t size;
            char *buf;
			VALUE result = rb_str_new(0, 0);

            RUN(cbind_get_arg_as_str(object->database, argnum, NULL, 0, CPP_UNICODE, &size, &is_null));
			buf = ALLOC_N(char, size + 1);
            RUN(cbind_get_arg_as_str(object->database, argnum, buf, size, CPP_UNICODE, &size, &is_null));
			
			RSTRING(result)->ptr = buf;
			RSTRING(result)->len = size;
		    RSTRING(result)->aux.capa = size;
			return rb_funcall(result, rb_intern("from_wchar"), 0);
		}
        case CBIND_BOOL_ID:
        {
            bool_t val;
            RUN(cbind_get_arg_as_bool(object->database, argnum, &val, &is_null));
			if(val) {
				return Qtrue;
			}
			return Qfalse;
        }
		
		
	}
	return Qnil;
}

static VALUE cache_object_param(VALUE self, VALUE obj, VALUE r_property) {
	struct rbObject* object;
	struct rbObject* param;
	struct rbDefinition* property;
	int by_ref = 0;


	Data_Get_Struct(self, struct rbObject, object);
	Data_Get_Struct(r_property, struct rbDefinition, property);
	
	if(obj == Qnil) {
        RUN(cbind_set_next_arg_as_null(object->database, property->cpp_type, by_ref));
        return obj;
	}
    switch (property->cpp_type) {
        case CBIND_VOID:
            break;
        case CBIND_OBJ_ID:
        {
			Data_Get_Struct(obj, struct rbObject, param);
            RUN(cbind_set_next_arg_as_obj(object->database, param->oref, WCHARSTR(param->class_name), by_ref));
            break;
        }
        case CBIND_INT_ID:
        {
			VALUE i = rb_funcall(obj, rb_intern("to_i"), 0);
			RUN(cbind_set_next_arg_as_int(object->database, NUM2INT(i), by_ref));
            break;
        }
        case CBIND_DOUBLE_ID:
		{
			VALUE f = rb_funcall(obj, rb_intern("to_f"), 0);
            RUN(cbind_set_next_arg_as_double(object->database, RFLOAT(f)->value, by_ref));
            break;
		}
        case CBIND_BINARY_ID:
        {
			VALUE res = Qnil;
			if(rb_respond_to(obj, rb_intern("to_s"))) {
				res = rb_funcall(obj, rb_intern("to_s"), 0);
			} else if (rb_respond_to(obj, rb_intern("read"))) {
				res = rb_funcall(obj, rb_intern("read"), 0);
			} else {
				rb_raise(rb_eStandardError, "Cannot marshall object");
				break;
			}
            RUN(cbind_set_next_arg_as_bin(object->database, STR(res), LEN(res), by_ref));
            break;
        }
        case CBIND_STRING_ID:
        {
			VALUE res = (rb_funcall(obj, rb_intern("to_s"), 0));
            RUN(cbind_set_next_arg_as_str(object->database, STR(res), LEN(res), MULTIBYTE, by_ref));
            break;
        }
        case CBIND_STATUS_ID:
        {
            // TBD
            break;
        }
        case CBIND_BOOL_ID:
        {
			bool_t res = RTEST(obj);
            RUN(cbind_set_next_arg_as_bool(object->database, res, by_ref));
            break;
        }
        default:
            rb_raise(rb_eStandardError,"unknown type for argument, type = %d", 
				property->cpp_type, CLASS_NAME(object));
            return Qnil;
	}
	return obj;
	
}

/*
 * Property
 */

static void cache_definition_free(struct rbDefinition* definition) {
	switch(definition->type) {
		case D_PROPERTY:
			RUN(cbind_free_prop_def(definition->def));
	}
	RUN(cbind_free_class_def(definition->database, definition->cl_def));
	free(definition);
}

static VALUE cache_definition_s_allocate(VALUE klass) {
	struct rbDefinition* definition = ALLOC(struct rbDefinition);
	bzero(definition, sizeof(struct rbDefinition));
	return Data_Wrap_Struct(klass, 0, cache_definition_free, definition);
}

static VALUE cache_definition_initialize(VALUE self, VALUE r_database, VALUE class_name, VALUE name) {
	struct rbDatabase* database;
	struct rbDefinition* definition;
	
	Data_Get_Struct(r_database, struct rbDatabase, database);
	Data_Get_Struct(self, struct rbDefinition, definition);
	
	definition->database = database->database;
	definition->in_name = WCHARSTR(name);
	
	RUN(cbind_alloc_class_def(database->database, WCHARSTR(class_name), &definition->cl_def));
	return self;
}

static VALUE cache_definition_cpp_type(VALUE self) {
	struct rbDefinition* definition;
	Data_Get_Struct(self, struct rbDefinition, definition);
	return INT2FIX(definition->cpp_type);
}


static VALUE cache_definition_cache_type(VALUE self) {
	struct rbDefinition* definition;
	Data_Get_Struct(self, struct rbDefinition, definition);
	return FROMWCSTR(definition->cache_type);
}

static VALUE cache_definition_name(VALUE self) {
	struct rbDefinition* definition;
	Data_Get_Struct(self, struct rbDefinition, definition);
	return FROMWCSTR(definition->name);
}

static VALUE cache_definition_in_name(VALUE self) {
	struct rbDefinition* definition;
	Data_Get_Struct(self, struct rbDefinition, definition);
	return FROMWCSTR(definition->in_name);
}


static VALUE cache_property_initialize(VALUE self, VALUE r_database, VALUE class_name, VALUE name) {
	struct rbDefinition* property;
	VALUE args[] = {r_database, class_name, name};
	rb_call_super(3, args);
	
	Data_Get_Struct(self, struct rbDefinition, property);

	property->type = D_PROPERTY;
    RUN(cbind_alloc_prop_def(&property->def));
    RUN(cbind_get_prop_def(property->cl_def, property->in_name, property->def));
    RUN(cbind_get_prop_cpp_type(property->def, &property->cpp_type));
    RUN(cbind_get_prop_cache_type(property->def, &property->cache_type));
    RUN(cbind_get_prop_name(property->def, &property->name));
	return self;
}

static VALUE cache_property_set_result(VALUE self) {
	struct rbDefinition* property;
	Data_Get_Struct(self, struct rbDefinition, property);
    RUN(cbind_set_next_arg_as_res(property->database, property->cpp_type));    
	return self;
}

static VALUE cache_method_initialize(VALUE self) {
	struct rbDefinition* method;
	Data_Get_Struct(self, struct rbDefinition, method);

	method->type = D_METHOD;
    RUN(cbind_alloc_mtd_def(&method->def));
    RUN(cbind_get_mtd_def(method->cl_def, method->in_name, method->def));
    RUN(cbind_get_mtd_is_func(method->def, &method->is_func));
    RUN(cbind_get_mtd_cpp_type(method->def, &method->cpp_type));
    RUN(cbind_get_mtd_cache_type(method->def, &method->cache_type));
    RUN(cbind_get_mtd_is_cls_mtd(method->def, &method->is_class_method));
    RUN(cbind_get_mtd_num_args(method->def, &method->num_args));
	method->num_args = 0;
    RUN(cbind_get_mtd_args_info(method->def, &method->args_info));
    RUN(cbind_get_mtd_name(method->def, &method->name));
	return self;
}

static VALUE cache_method_is_func(VALUE self) {
	struct rbDefinition* method;
	Data_Get_Struct(self, struct rbDefinition, method);
	return method->is_func ? Qtrue : Qfalse;
}


static VALUE cache_method_is_class_method(VALUE self) {
	struct rbDefinition* method;
	Data_Get_Struct(self, struct rbDefinition, method);
	return method->is_class_method ? Qtrue : Qfalse;
}

static VALUE cache_method_num_args(VALUE self) {
	struct rbDefinition* method;
	Data_Get_Struct(self, struct rbDefinition, method);
	return INT2FIX(method->num_args);
}

static VALUE cache_method_prepare_call(VALUE self) {
	struct rbDefinition* method;
	
	Data_Get_Struct(self, struct rbDefinition, method);
	
	RUN(cbind_reset_args(method->database));
	RUN(cbind_mtd_rewind_args(method->def));
	return self;
}

static VALUE cache_method_call(VALUE self, VALUE r_object) {
	struct rbDefinition* method;
	struct rbObject* object;
	
	Data_Get_Struct(self, struct rbDefinition, method);
	Data_Get_Struct(r_object, struct rbObject, object);

    RUN(cbind_run_method(method->database, object->oref, CLASS_NAME(object), method->in_name));
	return self;
}

static VALUE cache_argument_initialize(VALUE self, VALUE r_database, VALUE class_name, VALUE name, VALUE r_method) {
	struct rbDefinition* argument;
	struct rbDefinition* method;
	VALUE args[] = {r_database, class_name, name};
	rb_call_super(3, args);
	
	Data_Get_Struct(self, struct rbDefinition, argument);
	Data_Get_Struct(r_method, struct rbDefinition, method);


	argument->type = D_ARGUMENT;
    RUN(cbind_mtd_arg_get(method->def, argument->def));
    RUN(cbind_get_arg_cpp_type(argument->def, &argument->cpp_type));
    RUN(cbind_get_arg_cache_type(argument->def, &argument->cache_type));
    RUN(cbind_get_arg_name(argument->def, &argument->name));
    RUN(cbind_get_arg_is_by_ref(argument->def, &argument->is_by_ref));
    RUN(cbind_get_arg_is_default(argument->def, &argument->is_default));    
    RUN(cbind_get_arg_def_val(argument->def, &argument->default_value));
    RUN(cbind_get_arg_def_val_size(argument->def, &argument->default_value_size));
    RUN(cbind_mtd_arg_next(method->def));
	argument->arg_number = method->arg_counter;
	method->arg_counter++;
	return self;
}

static VALUE cache_argument_default_value(VALUE self) {
	struct rbDefinition* argument;
	Data_Get_Struct(self, struct rbDefinition, argument);
	if(!argument->is_default) {
		return Qnil;
	}
	return rb_str_new(argument->default_value, argument->default_value_size);
}


void Init_cache() {
	rb_define_method(rb_cString, "to_wchar", string_to_wchar, 0);
	rb_define_method(rb_cString, "from_wchar", string_from_wchar, 0);


	mCache = rb_define_module(MODULE_NAME);
	cObjectNotFound = rb_const_get(mCache, rb_intern(OBJECTNOTFOUND_NAME));


	cDatabase = rb_define_class_under(mCache, DB_NAME, rb_cObject);
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

	cQuery = rb_define_class_under(mCache, QUERY_NAME, rb_cObject);
	rb_define_alloc_func(cQuery, cache_query_s_allocate);
	rb_define_method(cQuery, "initialize", cache_query_initialize, 1);
	rb_define_method(cQuery, "execute", cache_query_execute, 0);
	rb_define_method(cQuery, "fetch", cache_query_fetch, 0);
	rb_define_method(cQuery, "close", cache_query_close, 0);

	cObject = rb_define_class_under(mCache, OBJECT_NAME, rb_cObject);
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

	cDefinition = rb_const_get(mCache, rb_intern(DEFINITION_NAME));

	rb_define_alloc_func(cDefinition, cache_definition_s_allocate);
	rb_define_method(cDefinition, "intern_initialize", cache_definition_initialize, 3);
	rb_define_method(cDefinition, "cpp_type", cache_definition_cpp_type, 0);
	rb_define_method(cDefinition, "cache_type", cache_definition_cache_type, 0);
	rb_define_method(cDefinition, "name", cache_definition_name, 0);
	rb_define_method(cDefinition, "in_name", cache_definition_in_name, 0);

	cProperty = rb_const_get(mCache, rb_intern(PROPERTY_NAME));
	rb_define_method(cProperty, "initialize", cache_property_initialize, 3);
	rb_define_method(cProperty, "set_as_result!", cache_property_set_result, 0);

	cMethod = rb_const_get(mCache, rb_intern(METHOD_NAME));
	rb_define_method(cMethod, "method_initialize", cache_method_initialize, 0);
	rb_define_method(cMethod, "is_func?", cache_method_is_func, 0);
	rb_define_method(cMethod, "is_class_method?", cache_method_is_class_method, 0);
	rb_define_method(cMethod, "num_args", cache_method_num_args, 0);
	rb_define_method(cMethod, "prepare_call!", cache_method_prepare_call, 0);
	rb_define_method(cMethod, "intern_call!", cache_method_call, 1);
	
	cArgument = rb_const_get(mCache, rb_intern(ARGUMENT_NAME));
	rb_define_method(cArgument, "initialize", cache_argument_initialize, 4);
	rb_define_method(cArgument, "default", cache_argument_default_value, 0);

}

