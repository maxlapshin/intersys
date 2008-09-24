#include "intersys.h"


static void intersys_base_real_close(struct rbDatabase* base) {
	if(base->closed) return;
	base->closed = 1;
	RUN(cbind_free_db(base->database));
	RUN(cbind_free_conn(base->connection));
}

void intersys_base_free(struct rbDatabase* base) {
	intersys_base_real_close(base);
	xfree(base);
}

VALUE intersys_base_s_allocate(VALUE klass) {
	struct rbDatabase* intersys_base = ALLOC(struct rbDatabase);
	memset(intersys_base, 0, sizeof(struct rbDatabase));
	return Data_Wrap_Struct(klass, 0, intersys_base_free, intersys_base);
}

VALUE intersys_base_initialize(VALUE self, VALUE options) {
	rb_iv_set(self, "@options", options);
	
	VALUE initialize_time = rb_funcall(rb_cTime, rb_intern("now"), 0);
	rb_cv_set(CLASS_OF(self), "@@last_query_at", rb_funcall(initialize_time, rb_intern("to_i"), 0));

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

VALUE intersys_base_connect(VALUE self, VALUE options) {
	struct rbDatabase* base;
	char conn_str[256];
	wchar_t w_conn_str[256];
	int size;
	bool_t is_uni;

	VALUE host, port, user, password, cache_namespace, timeout;

	host = connect_get_options(options, "host", "localhost", 0);
	//port = connect_get_options(options, "port", "1973", 0);
	port = connect_get_options(options, "port", "1972", 0);
	cache_namespace = connect_get_options(options, "namespace", "User", 0);
	//cache_namespace = connect_get_options(options, "namespace", "Samples", 0);

	user = connect_get_options(options, "user", "_SYSTEM", 1);
	password = connect_get_options(options, "password", "SYS", 1);
	timeout = rb_hash_aref(options, rb_intern("timeout"));
	if (timeout == Qnil) {
		timeout = INT2FIX(30);
	}

	Data_Get_Struct(self, struct rbDatabase, base);

	snprintf(conn_str, sizeof(conn_str), "%s[%s]:%s", RSTRING(host)->ptr, RSTRING(port)->ptr, RSTRING(cache_namespace)->ptr);

	RUN(cbind_utf8_to_uni(conn_str, (byte_size_t)strlen(conn_str), w_conn_str, (char_size_t)sizeof(w_conn_str),&size));
	w_conn_str[size] = 0;

	RUN(cbind_alloc_conn(w_conn_str, WCHARSTR(user), WCHARSTR(password),
		FIX2INT(timeout), &base->connection));
	RUN(cbind_alloc_db(base->connection, &base->database));
	RUN(cbind_is_uni_srv(base->database, &is_uni));
	if(!is_uni) {
		rb_warn("Warning! Cache database is not in Unicode mode. Perhaps, everything will fail working. In this case contact max@maxidoors.ru");
	}
	return self;
}

VALUE intersys_base_close(VALUE self) {
	struct rbDatabase* base;
	Data_Get_Struct(self, struct rbDatabase, base);
	intersys_base_real_close(base);
	return Qtrue;
}


VALUE intersys_base_start(VALUE self) {
	struct rbDatabase* base;
	Data_Get_Struct(self, struct rbDatabase, base);
	RUN(cbind_tstart(base->database));
	return self;
}

VALUE intersys_base_commit(VALUE self) {
	struct rbDatabase* base;
	Data_Get_Struct(self, struct rbDatabase, base);
	RUN(cbind_tcommit(base->database));
	return self;
}

VALUE intersys_base_rollback(VALUE self) {
	struct rbDatabase* base;
	Data_Get_Struct(self, struct rbDatabase, base);
	RUN(cbind_trollback(base->database));
	return self;
}

VALUE intersys_base_level(VALUE self) {
	struct rbDatabase* base;
	int level;
	Data_Get_Struct(self, struct rbDatabase, base);
	RUN(cbind_tlevel(base->database, &level));
	return INT2FIX(level);
}
