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
		VALUE handled = rb_funcall(mIntersys, rb_intern("handle_error"), 4, 
			INT2FIX(err), rb_str_new2(cbind_get_last_err_msg()), rb_str_new2(file), INT2FIX(line));
		if(handled == Qnil) {
			rb_raise(rb_eStandardError, "Intersystems Cache error %d: %s in file %s: %d", err, cbind_get_last_err_msg(), file, line);
		}
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
	
    result = rb_str_new(0, 0);

	RSTRING(result)->len = size;
    RSTRING(result)->aux.capa = capa;
    RSTRING(result)->ptr = ALLOC_N(char, capa);
	bzero(RSTRING(result)->ptr, capa);

	memcpy(RSTRING(result)->ptr, (char *)w_str, size);
	rb_str_freeze(result);
	return result;
}

VALUE rb_wcstr_new(const wchar_t *w_str, const char_size_t len) {
	return wcstr_new(w_str, len);
}

VALUE rb_wcstr_new2(const wchar_t *w_str) {
	return wcstr_new(w_str, w_str ? wcslen(w_str) : -1);
}