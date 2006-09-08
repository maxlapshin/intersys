#include "intersys.h"

VALUE string_to_wchar(VALUE self) {
	wchar_t w_chars[LEN(self) + 1];
	int size;
	RUN(cbind_utf8_to_uni(STR(self), (byte_size_t)LEN(self), w_chars, (char_size_t)sizeof(w_chars), &size));
	w_chars[size] = 0;
	return rb_str_new((char *)w_chars, (size+1)*sizeof(wchar_t));
}

VALUE string_from_wchar(VALUE self) {
	char chars[LEN(self) + 1];
	bzero(chars, sizeof(chars));
	int size;
	if(LEN(self) == 0 || !STR(self)) {
		return rb_str_new2("");
	}
    RUN(cbind_uni_to_utf8(WCHARSTR(self), wcslen(WCHARSTR(self)), chars, sizeof(chars), &size));
	return rb_str_new(chars, size);
}


int run(int err, char *file, int line) {
	if (err != 0) {
		rb_raise(rb_eStandardError, "Intersystems Cache error %d: %s in file %s: %d", err, cbind_get_last_err_msg(), file, line);
	}
	return err;
}

VALUE wcstr_new(const wchar_t *w_str, const char_size_t len) {
	VALUE result;
	int size, capa;
	if(!w_str) {
		return rb_funcall(rb_str_new2(""), rb_intern("to_wchar"), 0);
	}
	size = (int)(len)*sizeof(wchar_t);
	capa = (int)(len + 1)*sizeof(wchar_t);
	
    result = rb_str_buf_new(capa);
	bzero(STR(result) + size, capa-size);
	rb_str_buf_cat(result, (char *)w_str, size);

	rb_str_freeze(result);
	return result;
}

VALUE rb_wcstr_new(const wchar_t *w_str, const char_size_t len) {
	return wcstr_new(w_str, len);
}

VALUE rb_wcstr_new2(const wchar_t *w_str) {
	return wcstr_new(w_str, w_str ? wcslen(w_str) : -1);
}

static void intersys_status_mark(struct rbStatus* status) {
	rb_gc_mark(status->message);
}

VALUE intersys_status_s_allocate(VALUE klass) {
	struct rbStatus* status = ALLOC(struct rbStatus);
	bzero(status, sizeof(struct rbStatus));
	return Data_Wrap_Struct(klass, intersys_status_mark, 0, status);
}

VALUE intersys_status_initialize(VALUE self, VALUE code, VALUE message) {
	struct rbStatus* status;
	Data_Get_Struct(self, struct rbStatus, status);
	status->code = code;
	status->message = message;
	rb_call_super(1, &message);
	return self;
}

VALUE intersys_status_code(VALUE self) {
	struct rbStatus* status;
	Data_Get_Struct(self, struct rbStatus, status);
	return status->code;
}

VALUE intersys_status_message(VALUE self) {
	struct rbStatus* status;
	Data_Get_Struct(self, struct rbStatus, status);
	return status->message;
}

VALUE intersys_status_to_s(VALUE self) {
	struct rbStatus* status;
	Data_Get_Struct(self, struct rbStatus, status);
	return status->message;
}

