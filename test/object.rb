require 'test/unit'
require File.dirname(__FILE__) + '/../lib/intersys'


class Person < Intersys::Object
end

class QueryTest < Test::Unit::TestCase
  def test_open_and_save
    @p = Person.open(21)
    @name = "Anni Fyo"
    @p.name = @name
    @p.save
    @p = Article.open(21)
    assert_equal @name, @p.name
  end

   def test_populate
     assert Person.intersys_call("Populate", 1000)
     assert Person.intersys_call("%DeleteExtent")
   end
end
