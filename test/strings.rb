require 'test/unit'
require File.dirname(__FILE__) + '/../lib/intersys'

class StringTest < Test::Unit::TestCase
  @@wide_string_be = "\000\000\000a\000\000\000b\000\000\000c\000\000\000\000"
  @@wide_string_le = "a\000\000\000b\000\000\000c\000\000\000\000\000\000\000"

  def big_endian?
     [0xDEADBEAF].pack("l")[0] == 0xDE
  end

  def wide_string
    puts Intersys.__wchar_t_size
    return @@wide_string_be if big_endian?
	 @@wide_string_le
  end

  @@simple_string = "abc"
  def test_from_wchar
    assert_equal @@simple_string, self.wide_string.from_wchar
  end
  
  def test_to_wchar
    assert_equal self.wide_string, @@simple_string.to_wchar
  end
  
  def test_stress_wide
    assert_equal self.wide_string, self.wide_string.from_wchar.to_wchar.from_wchar.to_wchar
  end

  def test_stress
    assert_equal @@simple_string, @@simple_string.to_wchar.from_wchar.to_wchar.from_wchar
  end
end
