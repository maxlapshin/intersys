require 'test/unit'
require File.dirname(__FILE__) + '/../lib/intersys'

class GlobalTest < Test::Unit::TestCase

  def test_global
    assert @g = Intersys::Global.new("^Employees")
    @name = "John Smith"
    assert_equal @name, (@g["OIT", "Database", "manager"] = @name)
    assert_nil @g["OIT", "Database", "manager"]
  end
end