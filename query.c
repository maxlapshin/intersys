#include "cache.h"

void cache_query_free(struct rbQuery* query) {
	RUN(cbind_free_query(query->query));
	free(query);
}

VALUE cache_query_s_allocate(VALUE klass) {
	struct rbQuery* query = ALLOC(struct rbQuery);
	bzero(query, sizeof(struct rbQuery));
	return Data_Wrap_Struct(klass, 0, cache_query_free, query);
}

VALUE cache_query_initialize(VALUE self, VALUE database, VALUE sql_query) {
	struct rbQuery* query;
	struct rbDatabase* base;
	int sql_code;
	Data_Get_Struct(self, struct rbQuery, query);
	Data_Get_Struct(database, struct rbDatabase, base);
	RUN(cbind_alloc_query(base->database, &query->query));
	RUN(cbind_prepare_gen_query(query->query, WCHARSTR(sql_query), &sql_code));
	return self;
}

VALUE cache_query_execute(VALUE self) {
	struct rbQuery* query;
	int sql_code;
	int res;
	Data_Get_Struct(self, struct rbQuery, query);
	RUN(cbind_query_execute(query->query, &sql_code));
    RUN(cbind_query_get_num_pars(query->query, &res));
	printf("Hi: %d\n", res);
	return self;
}

VALUE cache_query_column_name(h_query query, int i) {
	int len;
	const wchar_t *res;
	RUN(cbind_query_get_col_name_len(query, i, &len));
	RUN(cbind_query_get_col_name(query, i, &res));
	return rb_funcall(rb_str_new((char *)res, len),rb_intern("from_wchar"), 0);
}

VALUE cache_query_get_data(VALUE self, VALUE index) {
	struct rbQuery* query;
	int type = 0;
	VALUE ret = Qnil;
	Data_Get_Struct(self, struct rbQuery, query);

    RUN(cbind_query_get_col_sql_type(query->query, FIX2INT(index), &type));
	switch(type) {
		
	}
	return ret;
}

VALUE cache_query_fetch(VALUE self) {
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
		query->query = 1;
		rb_funcall(self, rb_intern("close"), 0);
		return data;
	}
	if(sql_code) {
		rb_raise(rb_eStandardError, "Error in SQL: %d", sql_code);
	}
	
	RUN(cbind_query_get_num_cols(query->query, &num_cols));
	for(i = 0; i < num_cols; i++) {
		rb_ary_push(columns, cache_query_column_name(query->query, i));
	}
	
	for(i = 0; i < num_cols; i++) {
		rb_ary_push(data, rb_funcall(self, rb_intern("get_data"), 1, INT2FIX(i+1)));
	}
	rb_funcall(self, rb_intern("close"), 0);
	return data;
}

VALUE cache_query_close(VALUE self) {
	struct rbQuery* query;
	Data_Get_Struct(self, struct rbQuery, query);
	RUN(cbind_query_close(query->query));
	return self;
}
