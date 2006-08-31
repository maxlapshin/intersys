#include "intersys.h"

VALUE string_to_wchar(VALUE self) {
	wchar_t w_chars[RSTRING(self)->len + 1];
	int size;
	RUN(cbind_utf8_to_uni(RSTRING(self)->ptr, (byte_size_t)RSTRING(self)->len, w_chars, (char_size_t)sizeof(w_chars), &size));
	w_chars[size] = 0;
	return rb_str_new((char *)w_chars, (size+1)*sizeof(wchar_t));
}

VALUE string_from_wchar(VALUE self) {
	char chars[RSTRING(self)->len + 1];
	int size;
	if(LEN(self) == 0 || !STR(self)) {
		return rb_str_new2("");
	}
    RUN(cbind_uni_to_utf8(WCHARSTR(self), wcslen(WCHARSTR(self)), chars, sizeof(chars), &size));
	chars[size] = 0;
	return rb_str_new(chars, size + 1);
}


int run(int err, char *file, int line) {
	if (err != 0) {
		VALUE handled = rb_funcall(mCache, rb_intern("handle_error"), 4, 
			INT2FIX(err), rb_str_new2(cbind_get_last_err_msg()), rb_str_new2(file), INT2FIX(line));
		if(handled == Qnil) {
			rb_raise(rb_eStandardError, "Cache error %d: %s in file %s: %d", err, cbind_get_last_err_msg(), file, line);
		}
	}
	return err;
}

VALUE wcstr_new(const wchar_t *w_str) {
	if(!w_str) {
		return rb_funcall(rb_str_new2(""), rb_intern("to_wchar"), 0);
	}
	return rb_str_new((char *)w_str, wcslen(w_str));
}
