#include "intersys.h"
#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>

static void query_close(struct rbQuery* query) {
	printf("Trying to close query (%d,%d)\n", query->closed, query->executed);
	if(!query->closed && query->executed) {
		printf("Closing query\n");
		RUN(cbind_query_close(query->query));
		query->closed = 1;
	}
}

void intersys_query_free(struct rbQuery* query) {
	query_close(query);
	RUN(cbind_free_query(query->query));
	free(query);
}

VALUE intersys_query_s_allocate(VALUE klass) {
	struct rbQuery* query = ALLOC(struct rbQuery);
	bzero(query, sizeof(struct rbQuery));
	return Data_Wrap_Struct(klass, 0, intersys_query_free, query);
}

VALUE intersys_query_initialize(VALUE self, VALUE database, VALUE sql_query) {
	struct rbQuery* query;
	struct rbDatabase* base;
	int sql_code;
	Data_Get_Struct(self, struct rbQuery, query);
	Data_Get_Struct(database, struct rbDatabase, base);
	RUN(cbind_alloc_query(base->database, &query->query));
	RUN(cbind_prepare_gen_query(query->query, WCHARSTR(sql_query), &sql_code));
	return self;
}

VALUE intersys_query_execute(VALUE self) {
	struct rbQuery* query;
	int sql_code;
	int res;
	Data_Get_Struct(self, struct rbQuery, query);
	RUN(cbind_query_execute(query->query, &sql_code));
    RUN(cbind_query_get_num_pars(query->query, &res));
	query->executed = 1;
	return self;
}

VALUE intersys_query_get_data(VALUE self, VALUE index) {
	struct rbQuery* query;
	int type = 0;
	VALUE ret = Qnil;
	bool_t is_null;
	Data_Get_Struct(self, struct rbQuery, query);

    RUN(cbind_query_get_col_sql_type(query->query, FIX2INT(index), &type));
	switch(type) {
        case SQL_WCHAR:
        case SQL_WVARCHAR:
        case SQL_WLONGVARCHAR:
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        {
            wchar_t buf[32767];
            int size;
            RUN(cbind_query_get_uni_str_data(query->query, buf, sizeof(buf), &size, &is_null));
            if (is_null) {
				return Qnil;
            }
			return FROMWCSTR(buf);
        }
        case SQL_BINARY:
        case SQL_LONGVARBINARY:
        case SQL_VARBINARY:
        {
            char buf[32767];
            int size;
            
            RUN(cbind_query_get_bin_data(query->query, buf, sizeof(buf), &size, &is_null));
            if (is_null) {
				return Qnil;
            }
			return rb_str_new(buf, size);
		}
        case SQL_TINYINT:
        case SQL_SMALLINT:
        case SQL_INTEGER:
        case SQL_BIGINT:
        case SQL_BIT:
        {
            int res;
            RUN(cbind_query_get_int_data(query->query, &res, &is_null));
            if (is_null) {
				return Qnil;
            }
			return INT2NUM(res);
        }
        case SQL_FLOAT:
        case SQL_DOUBLE:
        case SQL_REAL:
        case SQL_NUMERIC:
        case SQL_DECIMAL:
        {
            double res;
            RUN(cbind_query_get_double_data(query->query, &res, &is_null));
            if (is_null) {
				return Qnil;
            }
			return rb_float_new(res);
        }


		
	}
	return ret;
}

VALUE intersys_query_column_name(VALUE self, VALUE i) {
	struct rbQuery* query;
	Data_Get_Struct(self, struct rbQuery, query);
	int len;
	const wchar_t *res;
	RUN(cbind_query_get_col_name_len(query->query, FIX2INT(i), &len));
	RUN(cbind_query_get_col_name(query->query, FIX2INT(i), &res));
	return FROMWCSTR(res);
}

VALUE intersys_query_fetch(VALUE self) {
	struct rbQuery* query;
	VALUE data;
	Data_Get_Struct(self, struct rbQuery, query);
	int num_cols = 0;
	int i = 0;
	int sql_code;

	RUN(cbind_query_fetch(query->query, &sql_code));
	
	data = rb_ary_new();

	if(sql_code == 100) {
		query->query = 1;
		rb_funcall(self, rb_intern("close"), 0);
		return data;
	}
	if(sql_code) {
		return data;
	//	rb_raise(rb_eStandardError, "Error in SQL: %d", sql_code);
	}
	
	RUN(cbind_query_get_num_cols(query->query, &num_cols));
	for(i = 0; i < num_cols; i++) {
		rb_ary_push(data, rb_funcall(self, rb_intern("get_data"), 1, INT2FIX(i+1)));
	}
	rb_funcall(self, rb_intern("close"), 0);
	return data;
}

VALUE intersys_query_close(VALUE self) {
	struct rbQuery* query;
	Data_Get_Struct(self, struct rbQuery, query);
	query_close(query);
	return self;
}
