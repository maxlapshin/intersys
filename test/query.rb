require 'test/unit'
require File.dirname(__FILE__) + '/../lib/intersys'

class Person < Intersys::Object
end


class QueryTest < Test::Unit::TestCase
  def test_query1
    return
    @db = Intersys::Database.new({})

    Person.populate(100)
    @db.start
#    @data = @db.query("delete from sample.person")
    @data = @db.query("select Id,name,DOB from sample.person")
    assert_equal 0, @data.size
    Person.populate(100)
    @data = @db.query("select count(Id) from sample.person")
    assert_equal 100, @data.first.first
    @data = @db.query("delete from sample.person where %Id > 6")
    @data = @db.query("select count(Id) from sample.person")
    assert_equal 6, @data.first.first
    @data = @db.query("select ID,name from sample.person")
    @data.each do |row|
      puts row
    end
    assert @data.size > 0
    @db.rollback
  end
  
  def test_select
    @db = Intersys::Database.new({})
    assert @db.query("select Id,name,DOB from sample.person")
  end
  
  def test_alter
    return
    @db = Intersys::Database.new({})

    @db.start
    assert @db.query("alter table sample.person add column DOB1 varchar(255)")
    assert @db.query("select %Id,name,DOB1 from sample.person")
    @db.rollback
  end

  def test_limit_offset
    @db = Intersys::Database.new({})
    assert @db.query("select * from sample.person limit 4 offset 2")
  end
end
