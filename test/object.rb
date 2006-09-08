require 'test/unit'
require File.dirname(__FILE__) + '/../lib/intersys'


class Person < Intersys::Object
end

class QueryTest < Test::Unit::TestCase
  def test_open_and_save
    Person.database.start
    @name = "Anni Fyo"
    @id = 31378
    #assert Person.populate(1000)
    assert @p = Person.open(@id)
    @p.name = @name
    assert @p.save
    assert @p = Person.open(@id)
    assert_equal @name, @p.name
    #assert Person.delete_extent
    Person.database.rollback
  end
  
  def test_create
    Person.database.start
    assert @p = Person.intersys_call("%New")
    @p.name = "Test user"
    assert @p.save
    #puts @p.id
    assert @p.destroy
    Person.database.rollback
  end
end
