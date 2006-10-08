#!/usr/bin/env ruby

require "mkmf"

WIN32 = RUBY_PLATFORM.match(/mswin/)
MACOS = RUBY_PLATFORM.match(/darwin/)

CONFIG["CC"] = "gcc -g" unless WIN32


#alias :old_cpp_include :cpp_include
#def cpp_include(header)
#  old_cpp_include(header) + <<-EOF
#  int main(void) {
#  }
#EOF
#end

@cache_placements = ["/home/max/cache", "/Applications/Cache", "/cygdrive/c/Progra~1/Cache", "/cygdrive/c/Cachesys", "C:/Cachesys", 'C:/Program Files (x86)/Cache']

def locations(suffix)
  # .map{|place| place.split("/").join(WIN32 ? "\\" : "/")}
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

if WIN32
  $CFLAGS << ' -I"C:\\Program Files\\Microsoft Platform SDK\\Include\\crt" '
  $CFLAGS << ' -I"C:\\Program Files\\Microsoft Platform SDK\\Include" '
  $CFLAGS << ' -I"C:\\Program Files\\Microsoft Visual Studio 8\\VC\\include" -D_WIN32 '
  
  $LDFLAGS << ' cbind.lib '
  
  VCINSTALLDIR = ENV["VCINSTALLDIR"]
  
  if VCINSTALLDIR.nil? then 
	$LDFLAGS << ' -libpath:"C:/Program Files (x86)/Microsoft Visual Studio 8/VC/lib" '
	$LDFLAGS << ' -libpath:"C:/Program Files/Microsoft Visual Studio 8/VC/lib" '  
  else 
    $LDFLAGS << ' -libpath:"' + VCINSTALLDIR + '" '
  end
  
  LIB = ENV["lib"]
  unless LIB
    LIB.split(";").each{ |path| $LDFLAGS << ' -libpath:"' + path.strip + '" ' }
  end  

  $LDFLAGS << ' -libpath:"C:/CacheSys/dev/cpp/lib" '
  $LDFLAGS << ' -libpath:"C:/Program Files (x86)/Cache/dev/cpp/lib" '
end


def link_flags
  " "+(library_locations.map { |place| WIN32 ? "-libpath:\"#{place}\"" :  ("-L"+place)} + ["-Wall"]).join(" ")
end

$CFLAGS << include_flags
$LDFLAGS << link_flags


have_header "c_api.h"
unless have_header "sql.h" 
  $CFLAGS << ' -Isql_include '
  have_header "sql.h"
end
have_header "sqlext.h"

unless MACOS
  find_library "cbind", "cbind_alloc_db",*library_locations
end

if WIN32
	CONFIG["LDSHARED"] = "link" 
	CONFIG["prefix"] = '"' + CONFIG["prefix"] + '"' 
	$ruby = '"' + $ruby + '"' 	
end


create_makefile 'intersys_cache'


