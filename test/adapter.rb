require 'test/unit'
require File.dirname(__FILE__) + '/../lib/intersys'
require File.dirname(__FILE__) + '/../lib/intersys_adapter'

class QueryTest < Test::Unit::TestCase
  def test_adapter
    @database = Intersys::Database.new({})
    @adapter = ActiveRecord::ConnectionAdapters::IntersysAdapter.new(@database, nil, {})
    
    assert_equal 'Intersys Caché', @adapter.adapter_name
    assert !@adapter.supports_migrations?
    #assert @adapter.active?
    assert @adapter.reconnect!
    assert @cols = @adapter.columns("person")
    assert @cols.last.name
    assert @cols.last.klass
  end
end