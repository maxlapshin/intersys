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
#find_library "cbind", "cbind_alloc_db","/Applications/Cache/bin" 

@cache_placements = ["/Applications/Cache", "/cygdrive/c/Program Files/Cache", "/cygdrive/c/Cachesys"]
def include_locations
  @cache_placements.map {|place| '"' + place + "/dev/cpp/include\""}
end

def include_flags
  " "+(include_locations.map { |place| "-I"+place} + ["-Wall"]).join(" ")
end
find_header "c_api.h", *include_locations
$CFLAGS << include_flags
create_makefile 'intersys_cache'

