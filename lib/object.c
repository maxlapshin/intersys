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
	VALUE database = rb_funcall(rb_obj_class(self), rb_intern("database"), 0);
	Data_Get_Struct(self, struct rbObject, object);
	Data_Get_Struct(database, struct rbDatabase, base);
	object->database = base->database;
	object->class_name = TOWCHAR(rb_funcall(rb_obj_class(self), rb_intern("class_name"), 0));
	return self;
}

VALUE intersys_object_open_by_id(VALUE self, VALUE oid) {
	int concurrency = rb_funcall(self, rb_intern("concurrency"), 0);
	int timeout = rb_funcall(self, rb_intern("timeout"), 0);
	int error;
	VALUE id = rb_funcall(oid, rb_intern("to_s"), 0);
	struct rbObject* object;
	VALUE r_object = rb_funcall(self, rb_intern("new"), 0);
	Data_Get_Struct(r_object, struct rbObject, object);
	
    error = cbind_openid(object->database, CLASS_NAME(object), WCHARSTR(TOWCHAR(id)), concurrency, timeout, &object->oref);
	switch(error) {
		case 0:
			return r_object;
		case -9: 
			rb_raise(cObjectNotFound, "Object with id %s not found", STR(id));
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


