#include "intersys.h"


static void intersys_global_mark(struct rbGlobal* global) {
	rb_gc_mark(global->name);
}

VALUE intersys_global_s_allocate(VALUE klass) {
	struct rbGlobal* global = ALLOC(struct rbGlobal);
	global->name = Qnil;
	return Data_Wrap_Struct(klass, intersys_global_mark, 0, global);
}

VALUE intersys_global_initialize(VALUE self, VALUE name) {
	struct rbGlobal* global;
	Data_Get_Struct(self, struct rbGlobal, global);
	global->name = name;
	return self;
}

VALUE intersys_global_set(int argc, VALUE *argv, VALUE self) {
	struct rbGlobal* global;
	int i;
	VALUE value = argv[argc - 1];
	argc--;
	Data_Get_Struct(self, struct rbGlobal, global);
	for(i = 0; i < argc; i++) {
		// marshall dimension
	}
	// perform setting with value
	return Qnil;
	return value;
}


VALUE intersys_global_get(int argc, VALUE *argv, VALUE self) {
	struct rbGlobal* global;
	int i;
	VALUE value = Qnil;
	Data_Get_Struct(self, struct rbGlobal, global);
	for(i = 0; i < argc; i++) {
		// marshall dimension
	}
	// perform getting with value
	return value;
}

VALUE intersys_global_delete(int argc, VALUE *argv, VALUE self) {
	struct rbGlobal* global;
	int i;
	Data_Get_Struct(self, struct rbGlobal, global);
	for(i = 0; i < argc; i++) {
		// marshall dimension
	}
	// perform getting with value
	return Qnil;
}


VALUE intersys_global_name(VALUE self) {
	struct rbGlobal* global;
	Data_Get_Struct(self, struct rbGlobal, global);
	return global->name;
}


