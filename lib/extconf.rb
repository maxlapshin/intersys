#!/usr/bin/env ruby

require "mkmf"



raise 'Use VS2005 solution from win32 directory to build this library' if RUBY_PLATFORM.match(/mswin/)
  

CONFIG["CC"] = "gcc -g" 


#alias :old_cpp_include :cpp_include
#def cpp_include(header)
#  old_cpp_include(header) + <<-EOF
#  int main(void) {
#  }
#EOF
#end

@cache_placements = ["/home/max/cache", "/Applications/Cache", "/cygdrive/c/Progra~1/Cache", "/cygdrive/c/Cachesys"]

def locations(suffix)
  @cache_placements.map {|place| place + suffix }
end                                                                                             

def include_locations
  locations("/dev/cpp/include") + ["./sql_include"]
end

def library_locations
  locations("/dev/cpp/lib")
end


def include_flags
  " "+(include_locations.map { |place| "-I"+place} + ["-Wall"]).join(" ")
end


def link_flags
  " "+(library_locations.map { |place| "-L"+place} + ["-Wall"]).join(" ")
end

$CFLAGS << include_flags
$LDFLAGS << link_flags


have_header "c_api.h"
unless have_header "sql.h" 
  $CFLAGS << ' -Isql_include '
  have_header "sql.h"
end
have_header "sqlext.h"

unless RUBY_PLATFORM.match(/darwin/)
  find_library "cbind", "cbind_alloc_db",*library_locations
end

create_makefile 'intersys_cache'


