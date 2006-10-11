require 'test/unit'
require File.dirname(__FILE__) + '/../lib/intersys'

class StringTest < Test::Unit::TestCase
  #@@wide_string_be = "\000\000\000a\000\000\000b\000\000\000c\000\000\000\000"
  #@@wide_string_le = "a\000\000\000b\000\000\000c\000\000\000\000\000\000\000"

  def setup
    @simple_string = "abc"
    @wide_string = "abc\0".unpack("cccc").pack(Intersys.__wchar_t_size_type * 4)
  end

  def test_from_wchar
    assert_equal @simple_string, @wide_string.from_wchar
  end
  
  def test_to_wchar
    assert_equal @wide_string, @simple_string.to_wchar
  end
  
  def test_stress_wide
    assert_equal @wide_string, @wide_string.from_wchar.to_wchar.from_wchar.to_wchar
  end

  def test_stress
    assert_equal @simple_string, @simple_string.to_wchar.from_wchar.to_wchar.from_wchar
  end
end
