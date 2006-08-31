#include "intersys.h"


void intersys_object_free(struct rbObject* object) {
	if (object->oref) {
		RUN(cbind_object_release(object->database, object->oref));
	}
	free(object);
}

VALUE intersys_object_s_allocate(VALUE klass) {
	struct rbObject* object = ALLOC(struct rbObject);
	bzero(object, sizeof(struct rbObject));
	return Data_Wrap_Struct(klass, 0, intersys_object_free, object);
}

VALUE intersys_object_initialize(VALUE self) {
	struct rbObject* object;
	struct rbDatabase* base;
	VALUE klass = rb_funcall(self, rb_intern("class"), 0);
	VALUE database = rb_funcall(klass, rb_intern("database"), 0);
	Data_Get_Struct(self, struct rbObject, object);
	Data_Get_Struct(database, struct rbDatabase, base);
	object->database = base->database;
	return self;
}

VALUE intersys_object_open_by_id(VALUE self, VALUE r_database, VALUE name, VALUE oid) {
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

VALUE intersys_object_create(VALUE self, VALUE r_database, VALUE name) {
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


VALUE intersys_object_methods(VALUE self) {
	struct rbObject* object;
	Data_Get_Struct(self, struct rbObject, object);
	return rb_ary_new();
}

VALUE intersys_object_properties(VALUE self) {
	struct rbObject* object;
	Data_Get_Struct(self, struct rbObject, object);
	return rb_ary_new();
}



VALUE intersys_object_get(VALUE self, VALUE r_property) {
	struct rbObject* object;
	struct rbDefinition* property;

	Data_Get_Struct(self, struct rbObject, object);
	Data_Get_Struct(r_property, struct rbDefinition, property);

	rb_funcall(r_property, rb_intern("set_as_result!"), 0);
    RUN(cbind_get_prop(object->database, object->oref, property->in_name));
	return rb_funcall(self, rb_intern("intern_result"), 2, INT2FIX(0), r_property);
}

VALUE intersys_object_set(VALUE self, VALUE r_property, VALUE value) {
	struct rbObject* object;
	struct rbDefinition* property;

	Data_Get_Struct(self, struct rbObject, object);
	Data_Get_Struct(r_property, struct rbDefinition, property);

    RUN(cbind_reset_args(object->database));
	rb_funcall(self, rb_intern("intern_param"), 2, value, r_property);
	RUN(cbind_set_prop(object->database, object->oref, property->in_name));
	return self;
}


VALUE extract_next_dlist_elem(char *dlist, int* elem_size) {
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


VALUE intersys_object_result(VALUE self, VALUE index, VALUE r_property) {
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
		case CBIND_VOID: {
			return Qnil;
		}
		
		case CBIND_OBJ_ID: {
			rb_raise(cUnMarshallError, "Cannot unmarshall type %d now", property->cpp_type);
			return Qnil;
		}
		
        case CBIND_TIME_ID:
        {
			int hour, minute, second;
			RUN(cbind_get_arg_as_time(object->database, argnum, &hour, &minute, &second, &is_null));
			return Qnil;
            break;
        }
        case CBIND_DATE_ID:
        {
			int year, month,day;
			RUN(cbind_get_arg_as_date(object->database, argnum, &year, &month, &day, &is_null));
			return Qnil;
            break;
        }
        case CBIND_TIMESTAMP_ID:
        {
			int year, month, day, hour, minute, second, fraction;
		  	RUN(cbind_get_arg_as_timestamp(object->database, argnum, 
					&year, &month, &day, &hour, &minute, &second, &fraction, &is_null));
			return Qnil;
            break;
        }
		
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
        case CBIND_CURRENCY_ID:
        {
            double val;
            RUN(cbind_get_arg_as_cy(object->database, argnum, &val, &is_null));
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
		case CBIND_STATUS_ID:{
            byte_size_t size;
            char *buf;
			int code;
			VALUE args[2];

            RUN(cbind_get_arg_as_status(object->database, argnum, &code, NULL, 0, MULTIBYTE, &size, &is_null));
			buf = ALLOC_N(char, size + 1);
            RUN(cbind_get_arg_as_status(object->database, argnum, &code, buf, size, MULTIBYTE, &size, &is_null));
			
			args[0] = INT2NUM(code);
			args[1] = FROMWCHAR(rb_str_new(buf, size));
			free(buf);
			return rb_class_new_instance(2, args, cStatus);
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
		
        case CBIND_DLIST_ID:
        {
            char *buf;
            char *p;
            byte_size_t size;
            int num_elems;
            int i;
			VALUE list;

            RUN(cbind_get_arg_as_dlist(object->database, argnum, NULL, 0, &size, &is_null));
            buf = ALLOC_N(char, size);
            RUN(cbind_get_arg_as_dlist(object->database, argnum, buf, size, &size, &is_null));	    

            RUN(cbind_dlist_calc_num_elems(buf, size, &num_elems));
			list = rb_ary_new2(num_elems);
            p = buf;
			for (i=0; i < num_elems; i++) {
	            int elem_size;
				rb_ary_push(list, extract_next_dlist_elem(p, &elem_size));
				p += elem_size;
			}
			free(buf);
            return list;
        }
		
	}
	return Qnil;
}


VALUE intersys_object_param(VALUE self, VALUE obj, VALUE r_property) {
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
        case CBIND_TIME_ID:
        {
			int hour = NUM2INT(CALL(obj, "hour"));
			int minute = NUM2INT(CALL(obj, "min"));
			int second = NUM2INT(CALL(obj, "sec"));
			RUN(cbind_set_next_arg_as_time(object->database, hour, minute, second, by_ref));
            break;
        }
        case CBIND_DATE_ID:
        {
			int year = NUM2INT(CALL(obj, "year"));
			int month = NUM2INT(CALL(obj, "month"));
			int day = NUM2INT(CALL(obj, "day"));
			RUN(cbind_set_next_arg_as_date(object->database, year, month, day, by_ref));
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
		  	RUN(cbind_set_next_arg_as_timestamp(object->database, 
					year, month, day, hour, minute, second, fraction, by_ref));
            break;
        }
        case CBIND_BOOL_ID:
        {
			bool_t res = RTEST(obj);
            RUN(cbind_set_next_arg_as_bool(object->database, res, by_ref));
            break;
        }
        case CBIND_DLIST_ID:
        {
            char buf[327];
			
			property->current_dlist_size = sizeof(buf);
			property->current_dlist = buf;
			rb_funcall(r_property, rb_intern("marshall_dlist"), 1, obj);
            RUN(cbind_set_next_arg_as_dlist(object->database, buf, sizeof(buf) - property->current_dlist_size, by_ref));
			property->current_dlist_size = 0;
			property->current_dlist = 0;
            break;

        }

        default:
            rb_raise(rb_eStandardError,"unknown type for argument, type = %d", 
				property->cpp_type, CLASS_NAME(object));
            return Qnil;
	}
	return obj;
	
}
