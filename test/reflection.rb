require 'test/unit'
require File.dirname(__FILE__) + '/../lib/intersys'


class Article < Intersys::Object
end
class Person < Intersys::Object
  prefix "Sample"
end

class ReflectionTest < Test::Unit::TestCase
  def test_methods
    @cdef = Intersys::Reflection::ClassDefinition.open("%Dictionary.ClassDefinition")
    assert_equal "%Dictionary.ClassDefinition", @cdef.name
    assert_equal "persistent", @cdef.class_type
    assert_equal "%Persistent,%Dictionary.ClassDefinitionQuery", @cdef.super
    assert_equal 58, @cdef.properties.size
    assert_equal 19, @cdef._methods.size
  end
  
  def test_class_names
    assert_equal Article, Article.lookup("User.Article")
    assert_equal Person, Article.lookup("Sample.Person")
  end
end