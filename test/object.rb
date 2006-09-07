require 'test/unit'
require File.dirname(__FILE__) + '/../lib/intersys'


class Person < Intersys::Object
end

class QueryTest < Test::Unit::TestCase
  def test_open_and_save
    @name = "Anni Fyo"
    @id = 4
    assert Person.intersys_call("Populate", 1000)
    assert @p = Person.open(@id)
    @p.name = @name
    assert @p.save
    assert @p = Person.open(@id)
    assert_equal @name, @p.name
    assert Person.intersys_call("%DeleteExtent")
  end
end
