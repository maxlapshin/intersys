#include "intersys.h"

static void intersys_definition_free(struct rbDefinition* definition) {
	switch(definition->type) {
		case D_PROPERTY: {
			RUN(cbind_free_prop_def(definition->def));
			break;
		}
		case D_METHOD: {
			RUN(cbind_free_mtd_def(definition->def));
			break;
		}
		case D_ARGUMENT: {
			RUN(cbind_free_arg_def(definition->def));
			break;
		}
	}
	RUN(cbind_free_class_def(definition->database, definition->cl_def));
	free(definition);
}

static void intersys_definition_mark(struct rbDefinition* definition) {
	rb_gc_mark(definition->class_name);
}

VALUE intersys_definition_s_allocate(VALUE klass) {
	struct rbDefinition* definition = ALLOC(struct rbDefinition);
	bzero(definition, sizeof(struct rbDefinition));
	return Data_Wrap_Struct(klass, intersys_definition_mark, intersys_definition_free, definition);
}

VALUE intersys_definition_initialize(VALUE self, VALUE r_database, VALUE class_name, VALUE name) {
	struct rbDatabase* database;
	struct rbDefinition* definition;
	
	Data_Get_Struct(r_database, struct rbDatabase, database);
	Data_Get_Struct(self, struct rbDefinition, definition);
	
	definition->database = database->database;
	definition->in_name = WCHARSTR(name);
	definition->class_name = TOWCHAR(class_name);
	
	RUN(cbind_alloc_class_def(definition->database, CLASS_NAME(definition), &definition->cl_def));
	return self;
}

VALUE intersys_definition_cpp_type(VALUE self) {
	struct rbDefinition* definition;
	Data_Get_Struct(self, struct rbDefinition, definition);
	return INT2FIX(definition->cpp_type);
}


VALUE intersys_definition_cache_type(VALUE self) {
	struct rbDefinition* definition;
	Data_Get_Struct(self, struct rbDefinition, definition);
	return FROMWCSTR(definition->cache_type);
}

VALUE intersys_definition_name(VALUE self) {
	struct rbDefinition* definition;
	Data_Get_Struct(self, struct rbDefinition, definition);
	return FROMWCSTR(definition->name);
}

VALUE intersys_definition_in_name(VALUE self) {
	struct rbDefinition* definition;
	Data_Get_Struct(self, struct rbDefinition, definition);
	return FROMWCSTR(definition->in_name);
}

VALUE intersys_property_initialize(VALUE self, VALUE r_database, VALUE class_name, VALUE name, VALUE object) {
	struct rbDefinition* property;
	struct rbObject* obj;
	VALUE args[] = {r_database, class_name, name};
	rb_call_super(3, args);
	
	Data_Get_Struct(self, struct rbDefinition, property);
	Data_Get_Struct(object, struct rbObject, obj);

	property->type = D_PROPERTY;
    RUN(cbind_alloc_prop_def(&property->def));
    RUN(cbind_get_prop_def(property->cl_def, property->in_name, property->def));
    RUN(cbind_get_prop_cpp_type(property->def, &property->cpp_type));
    RUN(cbind_get_prop_cache_type(property->def, &property->cache_type));
    RUN(cbind_get_prop_name(property->def, &property->name));
	property->oref = obj->oref;
	return self;
}


VALUE intersys_property_get(VALUE self) {
	struct rbDefinition* property;

	Data_Get_Struct(self, struct rbDefinition, property);

    RUN(cbind_reset_args(property->database));    
    RUN(cbind_set_next_arg_as_res(property->database, property->cpp_type));    
    RUN(cbind_get_prop(property->database, property->oref, property->in_name));
	return intersys_method_extract_retval(self);
}

VALUE intersys_property_set(VALUE self, VALUE value) {
	struct rbDefinition* property;

	Data_Get_Struct(self, struct rbDefinition, property);

    RUN(cbind_reset_args(property->database));
	intersys_argument_set(self, value);
	RUN(cbind_set_prop(property->database, property->oref, property->in_name));
	return self;
}



VALUE intersys_method_initialize(VALUE self, VALUE object) {
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
    RUN(cbind_get_mtd_args_info(method->def, &method->args_info));
    RUN(cbind_get_mtd_name(method->def, &method->name));
	if(object != Qnil) {
		struct rbObject* obj;
		Data_Get_Struct(object, struct rbObject, obj);
		method->oref = obj->oref;
		rb_iv_set(self, "@object", object);
	} else {
		method->oref = -1;
	}
	return self;
}

VALUE intersys_method_is_func(VALUE self) {
	struct rbDefinition* method;
	Data_Get_Struct(self, struct rbDefinition, method);
	return method->is_func ? Qtrue : Qfalse;
}


VALUE intersys_method_is_class_method(VALUE self) {
	struct rbDefinition* method;
	Data_Get_Struct(self, struct rbDefinition, method);
	return method->is_class_method ? Qtrue : Qfalse;
}

VALUE intersys_method_num_args(VALUE self) {
	struct rbDefinition* method;
	Data_Get_Struct(self, struct rbDefinition, method);
	return INT2FIX(method->num_args);
}


static VALUE extract_next_dlist_elem(char *dlist, int* elem_size) {
	bool_t flag;
	
    RUN(cbind_dlist_is_elem_null(dlist,&flag));
    if (flag) { 
        RUN(cbind_dlist_get_elem_size(dlist, elem_size));
		return Qnil;
    }

    RUN(cbind_dlist_is_elem_int(dlist,&flag));
    if (flag) { // process integer
        int val;
        RUN(cbind_dlist_get_elem_as_int(dlist, &val, elem_size));
		return NUM2INT(val);
    }
    RUN(cbind_dlist_is_elem_double(dlist,&flag));
    if (flag) { // process double
        double val;
        RUN(cbind_dlist_get_elem_as_double(dlist, &val, elem_size));
		return rb_float_new(val);
    }
    RUN(cbind_dlist_is_elem_str(dlist,&flag));
    if (flag) { // process string
        const char *str;
        int size;
        bool_t is_uni;
		VALUE val;

        RUN(cbind_dlist_get_str_elem(dlist, &is_uni, &str, &size, elem_size));
		val = rb_str_new(str, size);
        if (is_uni) {
			val = FROMWCHAR(val);
        }
		return val;
    }
	rb_raise(cUnMarshallError, "Couldn't unmarshall dlist element");
}





VALUE intersys_method_call(VALUE self, VALUE args) {
	struct rbDefinition* method;
	int i;
	Check_Type(args, T_ARRAY);
	Data_Get_Struct(self, struct rbDefinition, method);
	if(method->num_args < RARRAY(args)->len) {
		rb_raise(rb_eArgError, "wrong number of arguments (%d for %d)", RARRAY(args)->len, method->num_args);
	}
	VALUE database = rb_iv_get(self, "@database");
	VALUE class_name = rb_iv_get(self, "@class_name");
	VALUE name = rb_iv_get(self, "@name");
	
	RUN(cbind_reset_args(method->database));
	RUN(cbind_mtd_rewind_args(method->def));
	
	for(i = 0; i < RARRAY(args)->len; i++) {
		VALUE arg = rb_funcall(cArgument, rb_intern("new"), 4, database, class_name, name, self);
		intersys_argument_set(arg, RARRAY(args)->ptr[i]);
		RUN(cbind_mtd_arg_next(method->def));
	}
	method->passed_args = RARRAY(args)->len;

    if (method->cpp_type != CBIND_VOID) {
        RUN(cbind_set_next_arg_as_res(method->database, method->cpp_type));
    }
    RUN(cbind_run_method(method->database, method->oref, CLASS_NAME(method), method->in_name));
	return intersys_method_extract_retval(self);
}


VALUE intersys_argument_initialize(VALUE self, VALUE r_database, VALUE class_name, VALUE name, VALUE r_method) {
	struct rbDefinition* argument;
	struct rbDefinition* method;
	VALUE args[] = {r_database, class_name, name};
	rb_call_super(3, args);
	
	Data_Get_Struct(self, struct rbDefinition, argument);
	Data_Get_Struct(r_method, struct rbDefinition, method);


	argument->type = D_ARGUMENT;
	RUN(cbind_alloc_arg_def(&argument->def));
    RUN(cbind_mtd_arg_get(method->def, argument->def));
    RUN(cbind_get_arg_cpp_type(argument->def, &argument->cpp_type));
    RUN(cbind_get_arg_cache_type(argument->def, &argument->cache_type));
    RUN(cbind_get_arg_name(argument->def, &argument->name));
    RUN(cbind_get_arg_is_by_ref(argument->def, &argument->is_by_ref));
    RUN(cbind_get_arg_is_default(argument->def, &argument->is_default));    
    RUN(cbind_get_arg_def_val(argument->def, &argument->default_value));
    RUN(cbind_get_arg_def_val_size(argument->def, &argument->default_value_size));
	argument->arg_number = method->arg_counter;
	method->arg_counter++;
	return self;
}

VALUE intersys_argument_default_value(VALUE self) {
	struct rbDefinition* argument;
	Data_Get_Struct(self, struct rbDefinition, argument);
	if(!argument->is_default) {
		return Qnil;
	}
	return rb_str_new(argument->default_value, argument->default_value_size);
}

VALUE intersys_method_extract_retval(VALUE self) {
	struct rbDefinition* method;
	bool_t is_null;
	
	Data_Get_Struct(self, struct rbDefinition, method);
	if(method->cpp_type == CBIND_VOID) {
		return Qnil;
	}


    RUN(cbind_get_is_null(method->database, method->passed_args, &is_null));
	if(is_null) {
		return Qnil;
	}
	
	switch(method->cpp_type) {
		case CBIND_VOID: {
			return Qnil;
		}
		
		case CBIND_OBJ_ID: {
            int oref = 0;
			char_size_t len = 0;
			bool_t is_null = 1;
			VALUE result = Qnil, klass;
			VALUE class_name_w, class_name;
            const wchar_t *cl_name = 0;
			struct rbObject* object;
            RUN(cbind_get_arg_as_obj(method->database, method->passed_args, &oref, &cl_name, &len, &is_null));
			if(is_null) {
				printf("Loaded NULL object\n");
				return Qnil;
			}
			class_name_w = rb_wcstr_new(cl_name, len);
			class_name = FROMWCHAR(class_name_w);
			klass = rb_funcall(cObject, rb_intern("lookup"), 1, class_name);
			result = rb_funcall(klass, rb_intern("new"), 0);
			Data_Get_Struct(result, struct rbObject, object);
			object->oref = oref;
			return result;
		}
		
        case CBIND_TIME_ID:
        {
			int hour, minute, second;
			RUN(cbind_get_arg_as_time(method->database, method->passed_args, &hour, &minute, &second, &is_null));
			return Qnil;
            break;
        }
        case CBIND_DATE_ID:
        {
			int year, month,day;
			RUN(cbind_get_arg_as_date(method->database, method->passed_args, &year, &month, &day, &is_null));
			if(is_null) {
				return Qnil;
			}
			return rb_funcall(rb_cTime, rb_intern("local"), 3, INT2NUM(year), INT2NUM(month), INT2NUM(day))
        }
        case CBIND_TIMESTAMP_ID:
        {
			int year, month, day, hour, minute, second, fraction;
		  	RUN(cbind_get_arg_as_timestamp(method->database, method->passed_args, 
					&year, &month, &day, &hour, &minute, &second, &fraction, &is_null));
			if(is_null) {
				return Qnil;
			}
			//TODO: fraction also should be included
			return rb_funcall(rb_cTime, rb_intern("local"), 6, 
				INT2NUM(year), INT2NUM(month), INT2NUM(day), INT2NUM(hour), INT2NUM(minute), INT2NUM(second));
        }
		
		case CBIND_INT_ID: {
			int val;
			RUN(cbind_get_arg_as_int(method->database, method->passed_args, &val, &is_null));
			return INT2FIX(val);
		}
		
		case CBIND_DOUBLE_ID: {
			double val;
            RUN(cbind_get_arg_as_double(method->database, method->passed_args, &val, &is_null));
			return rb_float_new(val);
		}
        case CBIND_CURRENCY_ID:
        {
            double val;
            RUN(cbind_get_arg_as_cy(method->database, method->passed_args, &val, &is_null));
			return rb_float_new(val);
        }
		
		case CBIND_BINARY_ID: {
            byte_size_t size;
			VALUE result = Qnil;

            RUN(cbind_get_arg_as_bin(method->database, method->passed_args, NULL, 0, &size, &is_null));
			result = rb_str_buf_new((int)size);
            RUN(cbind_get_arg_as_bin(method->database, method->passed_args, STR(result), size, &size, &is_null));
			LEN(result) = size;
			return result;
		}
		case CBIND_STATUS_ID:{
            byte_size_t size;
			VALUE buf;
			int code;

			//TODO: if code is not OK, we should throw exception. No class Status is required
            RUN(cbind_get_arg_as_status(method->database, method->passed_args, &code, NULL, 0, MULTIBYTE, &size, &is_null));
			if(!code || is_null) {
				return Qtrue;
			}
			buf = rb_str_buf_new(size);
            RUN(cbind_get_arg_as_status(method->database, method->passed_args, &code, STR(buf), RSTRING(buf)->aux.capa, MULTIBYTE, &size, &is_null));
			STR(buf)[size] = '\0';
			rb_exc_raise(rb_funcall(cStatus, rb_intern("new"), 2, INT2FIX(code), buf));
			return Qnil;
		}

		case CBIND_STRING_ID: {
            byte_size_t size;
			VALUE result = Qnil;

            RUN(cbind_get_arg_as_str(method->database, method->passed_args, NULL, 0, CPP_UNICODE, &size, &is_null));
			//It is important to add wchar_t to end, because for wcslen we need more than 1 terminating zero.
			//I don't know exactly, how works wcslen, but I add 4 (sizeof wchar_t) terminating zeroes
			result = rb_str_buf_new(size + sizeof(wchar_t));
			bzero(STR(result) + size, sizeof(wchar_t));
            RUN(cbind_get_arg_as_str(method->database, method->passed_args, STR(result), size, CPP_UNICODE, &size, &is_null));
			LEN(result) = size;
			return FROMWCHAR(result);
		}

        case CBIND_BOOL_ID:
        {
            bool_t val;
            RUN(cbind_get_arg_as_bool(method->database, method->passed_args, &val, &is_null));
			if(val) {
				return Qtrue;
			}
			return Qfalse;
        }
		
        case CBIND_DLIST_ID:
        {
			VALUE buf = Qnil;
            char *p;
            byte_size_t size;
            int num_elems;
            int i;
			VALUE list;

            RUN(cbind_get_arg_as_dlist(method->database, method->passed_args, NULL, 0, &size, &is_null));
			buf = rb_str_buf_new(size);
            RUN(cbind_get_arg_as_dlist(method->database, method->passed_args, STR(buf), size, &size, &is_null));
			LEN(buf) = size;

            RUN(cbind_dlist_calc_num_elems(STR(buf), LEN(buf), &num_elems));
			list = rb_ary_new2(num_elems);
            p = STR(buf);
			for (i=0; i < num_elems; i++) {
	            int elem_size;
				rb_ary_push(list, extract_next_dlist_elem(p, &elem_size));
				p += elem_size;
			}
            return list;
        }
		
	}
	return Qnil;
}


VALUE intersys_argument_marshall_dlist_elem(VALUE self, VALUE elem) {
	struct rbDefinition* argument;
	Data_Get_Struct(self, struct rbDefinition, argument);
	int elem_size;
	
	switch(TYPE(elem)) {
		case T_NIL: {
			RUN(cbind_dlist_put_null_elem(argument->current_dlist, argument->current_dlist_size, &elem_size));
			break;
		}
		case T_FIXNUM: {
			RUN(cbind_dlist_put_int_elem(argument->current_dlist, argument->current_dlist_size, FIX2INT(elem), &elem_size));
			break;
	    }
		case T_FLOAT: {
			RUN(cbind_dlist_put_double_elem(argument->current_dlist, argument->current_dlist_size, RFLOAT(elem)->value, &elem_size));		
			break;
		}
		case T_STRING: {
			VALUE val = TOWCHAR(elem);
	        RUN(cbind_dlist_put_str_elem(argument->current_dlist, argument->current_dlist_size, 1, STR(val), LEN(val), &elem_size));
			break;
		}
		default: {
			rb_raise(cMarshallError, "couldn't marshall to dlist element: %s", STR(CALL(elem, "inspect")));
	        return Qnil;
		}
	}
	argument->current_dlist_size -= elem_size;
	argument->current_dlist += elem_size;
	return elem;
}


VALUE intersys_argument_set(VALUE self, VALUE obj) {
	struct rbDefinition* property;
	Data_Get_Struct(self, struct rbDefinition, property);
	
	if(obj == Qnil) {
        RUN(cbind_set_next_arg_as_null(property->database, property->cpp_type, property->is_by_ref));
        return self;
	}
    switch (property->cpp_type) {
        case CBIND_VOID:
            break;
        case CBIND_OBJ_ID:
        {
			struct rbObject* param;
			Data_Get_Struct(obj, struct rbObject, param);
            RUN(cbind_set_next_arg_as_obj(property->database, param->oref, WCHARSTR(param->class_name), property->is_by_ref));
            break;
        }
        case CBIND_INT_ID:
        {
			VALUE i = rb_funcall(obj, rb_intern("to_i"), 0);
			RUN(cbind_set_next_arg_as_int(property->database, NUM2INT(i), property->is_by_ref));
            break;
        }
        case CBIND_DOUBLE_ID:
		{
			VALUE f = rb_funcall(obj, rb_intern("to_f"), 0);
            RUN(cbind_set_next_arg_as_double(property->database, RFLOAT(f)->value, property->is_by_ref));
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
            RUN(cbind_set_next_arg_as_bin(property->database, STR(res), LEN(res), property->is_by_ref));
            break;
        }
        case CBIND_STRING_ID:
        {
			VALUE res = rb_funcall(obj, rb_intern("to_s"), 0);
            RUN(cbind_set_next_arg_as_str(property->database, STR(res), LEN(res), MULTIBYTE, property->is_by_ref));
            break;
        }
        case CBIND_STATUS_ID:
        {
            // TBD
            break;
        }
        case CBIND_TIME_ID:
        {
			int hour = NUM2INT(CALL(obj, "hour"));
			int minute = NUM2INT(CALL(obj, "min"));
			int second = NUM2INT(CALL(obj, "sec"));
			RUN(cbind_set_next_arg_as_time(property->database, hour, minute, second, property->is_by_ref));
            break;
        }
        case CBIND_DATE_ID:
        {
			int year = NUM2INT(CALL(obj, "year"));
			int month = NUM2INT(CALL(obj, "month"));
			int day = NUM2INT(CALL(obj, "day"));
			RUN(cbind_set_next_arg_as_date(property->database, year, month, day, property->is_by_ref));
            break;
        }
        case CBIND_TIMESTAMP_ID:
        {
			int year = NUM2INT(CALL(obj, "year"));
			int month = NUM2INT(CALL(obj, "month"));
			int day = NUM2INT(CALL(obj, "day"));
			int hour = NUM2INT(CALL(obj, "hour"));
			int minute = NUM2INT(CALL(obj, "min"));
			int second = NUM2INT(CALL(obj, "sec"));
			int fraction = 0;
		  	RUN(cbind_set_next_arg_as_timestamp(property->database, 
					year, month, day, hour, minute, second, fraction, property->is_by_ref));
            break;
        }
        case CBIND_BOOL_ID:
        {
			bool_t res = RTEST(obj);
            RUN(cbind_set_next_arg_as_bool(property->database, res, property->is_by_ref));
            break;
        }
        case CBIND_DLIST_ID:
        {
            char buf[327];
			
			property->current_dlist_size = sizeof(buf);
			property->current_dlist = buf;
			rb_funcall(self, rb_intern("marshall_dlist"), 1, obj);
            RUN(cbind_set_next_arg_as_dlist(property->database, buf, sizeof(buf) - property->current_dlist_size, property->is_by_ref));
			property->current_dlist_size = 0;
			property->current_dlist = 0;
            break;

        }

        default:
            rb_raise(rb_eStandardError,"unknown type for argument, type = %d", 
				property->cpp_type, CLASS_NAME(property));
            return self;
	}
	return self;
	
}
