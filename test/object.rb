require 'test/unit'
require File.dirname(__FILE__) + '/../lib/intersys'


class Person < Intersys::Object
end

class QueryTest < Test::Unit::TestCase
  def test_open_and_save
    @name = "Anni Fyo"
    @id = 26001
    #assert Person.intersys_call("Populate", 1000)
    assert @p = Person.open(@id)
    @p.name = @name
    assert @p.save
    assert @p = Person.open(@id)
    assert_equal @name, @p.name
    assert Person.intersys_call("%DeleteExtent")
  end
  
  def test_create
    assert @p = Person.intersys_call("%New")
    @p.name = "Test user"
    assert @p.save
    puts @p.id
  end
end
