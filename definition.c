#include "intersys.h"

void intersys_definition_free(struct rbDefinition* definition) {
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

VALUE intersys_definition_s_allocate(VALUE klass) {
	struct rbDefinition* definition = ALLOC(struct rbDefinition);
	bzero(definition, sizeof(struct rbDefinition));
	return Data_Wrap_Struct(klass, 0, intersys_definition_free, definition);
}

VALUE intersys_definition_initialize(VALUE self, VALUE r_database, VALUE class_name, VALUE name) {
	struct rbDatabase* database;
	struct rbDefinition* definition;
	
	Data_Get_Struct(r_database, struct rbDatabase, database);
	Data_Get_Struct(self, struct rbDefinition, definition);
	
	definition->database = database->database;
	definition->in_name = WCHARSTR(name);
	
	RUN(cbind_alloc_class_def(database->database, WCHARSTR(class_name), &definition->cl_def));
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


VALUE intersys_property_initialize(VALUE self, VALUE r_database, VALUE class_name, VALUE name) {
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

VALUE intersys_property_set_result(VALUE self) {
	struct rbDefinition* property;
	Data_Get_Struct(self, struct rbDefinition, property);
    RUN(cbind_set_next_arg_as_res(property->database, property->cpp_type));    
	return self;
}

VALUE intersys_method_initialize(VALUE self) {
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

VALUE intersys_method_prepare_call(VALUE self) {
	struct rbDefinition* method;
	
	Data_Get_Struct(self, struct rbDefinition, method);
	
	RUN(cbind_reset_args(method->database));
	RUN(cbind_mtd_rewind_args(method->def));
	return self;
}

VALUE intersys_method_call(VALUE self, VALUE r_object) {
	struct rbDefinition* method;
	struct rbObject* object;
	
	Data_Get_Struct(self, struct rbDefinition, method);
	Data_Get_Struct(r_object, struct rbObject, object);

    if (method->cpp_type != CBIND_VOID) {    
        RUN(cbind_set_next_arg_as_res(method->database, method->cpp_type));
    }
    RUN(cbind_run_method(method->database, object->oref, CLASS_NAME(object), method->in_name));
	return self;
}

VALUE intersys_method_extract_retval(VALUE self, VALUE r_object) {
	struct rbDefinition* method;
	struct rbObject* object;
	
	Data_Get_Struct(self, struct rbDefinition, method);
	Data_Get_Struct(r_object, struct rbObject, object);
	if(method->cpp_type == CBIND_VOID) {
		return Qnil;
	}
	return rb_funcall(r_object, rb_intern("intern_result"), 2, method->num_args, self);
}

VALUE intersys_argument_initialize(VALUE self, VALUE r_database, VALUE class_name, VALUE name, VALUE r_method) {
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

VALUE intersys_argument_default_value(VALUE self) {
	struct rbDefinition* argument;
	Data_Get_Struct(self, struct rbDefinition, argument);
	if(!argument->is_default) {
		return Qnil;
	}
	return rb_str_new(argument->default_value, argument->default_value_size);
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
