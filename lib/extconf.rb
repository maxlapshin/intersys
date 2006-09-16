#!/usr/bin/env ruby

require "mkmf"

CONFIG["CC"] = "gcc -g" unless RUBY_PLATFORM.match /mswin/

WIN32 = RUBY_PLATFORM.match(/mswin/)
MACOS = RUBY_PLATFORM.match(/darwin/)

#alias :old_cpp_include :cpp_include
#def cpp_include(header)
#  old_cpp_include(header) + <<-EOF
#  int main(void) {
#  }
#EOF
#end

@cache_placements = ["/home/max/cache", "/Applications/Cache", "/cygdrive/c/Progra~1/Cache", "/cygdrive/c/Cachesys", "C:/Cachesys"]

def locations(suffix)
  @cache_placements.map {|place| place + suffix}.map{|place| place.split("/").join(WIN32 ? "\\" : "/") }
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

if WIN32
  $CFLAGS << ' -I"C:\\Program Files\\Microsoft Visual Studio .NET 2003\\Vc7\\PlatformSDK\\Include" '
  $CFLAGS << ' -I"C:\\Program Files\\Microsoft Visual Studio .NET 2003\\Vc7\\include" '
  $LDFLAGS << ' -libpath:"C:\\CacheSys\\dev\\cpp\\lib" '
  $LDFLAGS << ' -libpath:"C:\\Program Files\\Microsoft Visual Studio .NET 2003\\Vc7\\lib" '
  $LDFLAGS << ' -libpath:"C:\\Program Files\\Microsoft Visual Studio .NET 2003\\Vc7\PlatformSDK\\Lib" '
end


def link_flags
  " "+(library_locations.map { |place| "-L"+place} + ["-Wall"]).join(" ")
end

$CFLAGS << include_flags
$LDFLAGS << link_flags

have_header "c_api.h"
have_header "sql.h"
have_header "sqlext.h"

unless MACOS
  find_library "cbind", "cbind_alloc_db",*library_locations
end


create_makefile 'intersys_cache'

