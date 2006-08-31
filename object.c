#include "intersys.h"


static void intersys_object_free(struct rbObject* object) {
	if (object->oref) {
		RUN(cbind_object_release(object->database, object->oref));
	}
	free(object);
}

static void intersys_object_mark(struct rbObject* object) {
	rb_gc_mark(object->class_name);
}


VALUE intersys_object_s_allocate(VALUE klass) {
	struct rbObject* object = ALLOC(struct rbObject);
	bzero(object, sizeof(struct rbObject));
	return Data_Wrap_Struct(klass, intersys_object_mark, intersys_object_free, object);
}

VALUE intersys_object_initialize(VALUE self) {
	struct rbObject* object;
	struct rbDatabase* base;
	VALUE klass = rb_funcall(self, rb_intern("class"), 0);
	VALUE database = rb_funcall(klass, rb_intern("database"), 0);
	Data_Get_Struct(self, struct rbObject, object);
	Data_Get_Struct(database, struct rbDatabase, base);
	object->database = base->database;
	object->class_name = rb_funcall(klass, rb_intern("class_name"), 0);
	return self;
}

VALUE intersys_object_open_by_id(VALUE self, VALUE oid) {
	int concurrency = rb_funcall(self, rb_intern("concurrency"), 0);
	int timeout = rb_funcall(self, rb_intern("timeout"), 0);
	int error;
	struct rbObject* object;
	VALUE r_object = rb_funcall(self, rb_intern("new"), 0);
	Data_Get_Struct(r_object, struct rbObject, object);
	
    error = cbind_openid(object->database, CLASS_NAME(object), WCHARSTR(oid), concurrency, timeout, &object->oref);
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

VALUE intersys_object_create(VALUE self) {
	wchar_t *init_val = NULL;
	struct rbObject *object;
	VALUE r_object = rb_funcall(self, rb_intern("new"), 0);
	
	Data_Get_Struct(r_object, struct rbObject, object);
	
	RUN(cbind_create_new(object->database, CLASS_NAME(object), init_val,&object->oref));
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



