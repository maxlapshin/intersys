require 'test/unit'
require File.dirname(__FILE__) + '/../lib/intersys'


class QueryTest < Test::Unit::TestCase
  def test_query1
    return
    @db = Intersys::Database.new({})

    @data = @db.query("select * from sample.person")
    assert @data.size > 0
  end
end
