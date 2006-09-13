#include "intersys.h"
#ifdef HAVE_SQL_H

#ifdef __CYGWIN__
#include <windef.h>
#endif
#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>

static void query_close(struct rbQuery* query) {
	if(!query->closed && query->executed) {
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
	rb_iv_set(self, "@database", database);
	query->limit = -1;
	RUN(cbind_alloc_query(base->database, &query->query));
	RUN(cbind_prepare_gen_query(query->query, WCHARSTR(TOWCHAR(sql_query)), &sql_code));
	return self;
}

static void query_bind_one_param(h_query query, int index, VALUE obj) {
	int sql_type;
    RUN(cbind_query_get_par_sql_type(query, index, &sql_type));

    switch (sql_type) {
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR: {
			VALUE str = rb_funcall(obj, rb_intern("to_s"), 0);
            RUN(cbind_query_set_mb_str_par(query, index, STR(str), LEN(str)));
            break;
        }
        case SQL_BINARY:
        case SQL_LONGVARBINARY:
        case SQL_VARBINARY: {
			VALUE str = rb_funcall(obj, rb_intern("to_s"), 0);
            RUN(cbind_query_set_bin_par(query, index, STR(str), LEN(str)));
            break;
        }
        case SQL_TINYINT:
        case SQL_SMALLINT:
        case SQL_INTEGER:
        case SQL_BIGINT:
        case SQL_BIT:
        {
			VALUE num = rb_funcall(obj, rb_intern("to_i"), 0);
            RUN(cbind_query_set_int_par(query, index, NUM2INT(num)));
            break;
        }
        case SQL_FLOAT:
        case SQL_DOUBLE:
        case SQL_REAL:
        case SQL_NUMERIC:
        case SQL_DECIMAL:
        {
			VALUE f = rb_funcall(obj, rb_intern("to_f"), 0);
            RUN(cbind_query_set_double_par(query, index, RFLOAT(f)->value));
            break;
        }
        case SQL_TIME:
        {
			int hour = NUM2INT(CALL(obj, "hour"));
			int minute = NUM2INT(CALL(obj, "min"));
			int second = NUM2INT(CALL(obj, "sec"));
			RUN(cbind_query_set_time_par(query, index, hour, minute, second));
            break;
        }
        case SQL_DATE:
        {
			int year = NUM2INT(CALL(obj, "year"));
			int month = NUM2INT(CALL(obj, "month"));
			int day = NUM2INT(CALL(obj, "day"));
			RUN(cbind_query_set_date_par(query, index, year, month, day));
            break;
        }
        case SQL_TIMESTAMP:
        {
			int year = NUM2INT(CALL(obj, "year"));
			int month = NUM2INT(CALL(obj, "month"));
			int day = NUM2INT(CALL(obj, "day"));
			int hour = NUM2INT(CALL(obj, "hour"));
			int minute = NUM2INT(CALL(obj, "min"));
			int second = NUM2INT(CALL(obj, "sec"));
			int fraction = 0;
		  	RUN(cbind_query_set_timestamp_par(query, index, 
					year, month, day, hour, minute, second, fraction));
            break;
        }

        default:
            rb_raise(cMarshallError, "unknown sql type %d for parameter N %d", sql_type, index);
    }
}


VALUE intersys_query_bind_params(VALUE self, VALUE params) {
	int i;
	struct rbQuery* query;
	Check_Type(params, T_ARRAY);
	Data_Get_Struct(self, struct rbQuery, query);

	for(i = 0; i < RARRAY(params)->len; i++) {
		query_bind_one_param(query->query, i, RARRAY(params)->ptr[i]);
	}
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
	VALUE data = Qnil;
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
	return data;
}


VALUE intersys_query_each(VALUE self) {
	struct rbQuery* query;
	int i;
	Data_Get_Struct(self, struct rbQuery, query);
	if(query->offset > 0) {
		RUN(cbind_query_skip(query->query, query->offset));
	}
	for(i = query->offset; i < query->offset + query->limit; i++) {
		VALUE row = intersys_query_fetch(self);
		if(row == Qnil || RARRAY(row)->len == 0) {
			break;
		}
		rb_yield(row);
	}
	query_close(query);
	return self;
}



VALUE intersys_query_close(VALUE self) {
	struct rbQuery* query;
	Data_Get_Struct(self, struct rbQuery, query);
	query_close(query);
	return self;
}


VALUE intersys_query_set_limit(VALUE self, VALUE limit) {
	struct rbQuery* query;
	Data_Get_Struct(self, struct rbQuery, query);
	query->limit = NUM2INT(rb_funcall(limit, rb_intern("to_i"), 0));
	return limit;
}
VALUE intersys_query_get_limit(VALUE self) {
	struct rbQuery* query;
	Data_Get_Struct(self, struct rbQuery, query);
	return INT2FIX(query->limit);
}

VALUE intersys_query_set_offset(VALUE self, VALUE offset) {
	struct rbQuery* query;
	Data_Get_Struct(self, struct rbQuery, query);
	query->offset = NUM2INT(rb_funcall(offset, rb_intern("to_i"), 0));
	return offset;
}

VALUE intersys_query_get_offset(VALUE self) {
	struct rbQuery* query;
	Data_Get_Struct(self, struct rbQuery, query);
	return INT2FIX(query->offset);
}

#endif /* HAVE_SQL_H */
