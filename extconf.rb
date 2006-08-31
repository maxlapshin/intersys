#!/usr/bin/env ruby

require "mkmf"
#CONFIG["CPP"] = "g++"
CONFIG["CC"] = "gcc -g"


alias :old_cpp_include :cpp_include
#def cpp_include(header)
#  old_cpp_include(header) + <<-EOF
#  int main(void) {
#  }
#EOF
#end
#find_library "cbind", "cbind_alloc_db","/Applications/Cache/bin" 

find_header "c_api.h", "/Applications/Cache/dev/cpp/include/", "/Developer/Examples/Cache/cpp/include/"
$CFLAGS << " -I/Applications/Cache/dev/cpp/include/ -I/Developer/Examples/Cache/cpp/include/ -Wall" 
create_makefile 'cache'

