#!/usr/bin/env ruby

require "mkmf"
#CONFIG["CPP"] = "g++"
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
  @cache_placements.map {|place| place + suffix}
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
find_header "c_api.h", *include_locations
find_header "sql.h", *include_locations
find_header "sqlext.h", *include_locations
$CFLAGS << include_flags
$LDFLAGS << link_flags
#$LDFLAGS << "-Wl,-no-export-libs,cbind.lib"
find_library "cbind", "cbind_alloc_db",*library_locations
create_makefile 'intersys_cache'

