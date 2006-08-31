#include "cache.h"

void cache_base_free(struct rbDatabase* base) {
	RUN(cbind_free_db(base->database));
	RUN(cbind_free_conn(base->connection));
	free(base);
}

VALUE cache_base_s_allocate(VALUE klass) {
	struct rbDatabase* cache_base = ALLOC(struct rbDatabase);
	bzero(cache_base, sizeof(struct rbDatabase));
	return Data_Wrap_Struct(klass, 0, cache_base_free, cache_base);
}

VALUE cache_base_initialize(VALUE self, VALUE options) {
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

VALUE cache_base_connect(VALUE self, VALUE options) {
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

VALUE cache_base_start(VALUE self) {
	struct rbDatabase* base;
	Data_Get_Struct(self, struct rbDatabase, base);
	RUN(cbind_tstart(base->database));
	return Qtrue;
}

VALUE cache_base_commit(VALUE self) {
	struct rbDatabase* base;
	Data_Get_Struct(self, struct rbDatabase, base);
	RUN(cbind_tcommit(base->database));
	return Qtrue;
}

VALUE cache_base_rollback(VALUE self) {
	struct rbDatabase* base;
	Data_Get_Struct(self, struct rbDatabase, base);
	RUN(cbind_trollback(base->database));
	return Qtrue;
}

VALUE cache_base_level(VALUE self) {
	struct rbDatabase* base;
	int level;
	Data_Get_Struct(self, struct rbDatabase, base);
	RUN(cbind_tlevel(base->database, &level));
	return INT2FIX(level);
}
