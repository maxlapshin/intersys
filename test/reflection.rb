require 'test/unit'
require File.dirname(__FILE__) + '/../lib/intersys'


class Article < Intersys::Object
  prefix "Sample"
end
class Person < Intersys::Object
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
    assert_equal Article, Article.lookup("Sample.Article")
    assert_equal Person, Article.lookup("User.Person")
  end
  
  def test_class_create
    @cdef = Intersys::Reflection::ClassDefinition.call("%New", "User.Person")
    @cdef.class_type = "persistent"
    @cdef.super = "%Persistent,%Populate,%XML.Adaptor"
    @props = @cdef.properties

    @prop = Intersys::Reflection::PropertyDefinition.intersys_call("%New", "Person:Name")
    assert @props << @prop
    @prop.type = "%String"

    @prop = Intersys::Reflection::PropertyDefinition.intersys_call("%New", "Person:SSN")
    assert @props << @prop
    @prop.type = "%String"

    @prop = Intersys::Reflection::PropertyDefinition.intersys_call("%New", "Person:Title")
    assert @props << @prop
    @prop.type = "%String"

    @prop = Intersys::Reflection::PropertyDefinition.intersys_call("%New", "Person:DOB")
    assert @props << @prop
    @prop.type = "%String"
    
    @prop = Intersys::Reflection::PropertyDefinition.intersys_call("%New", "Person:Home")
    assert @props << @prop
    @prop.type = "Sample.Address"
    
    @prop = Intersys::Reflection::PropertyDefinition.intersys_call("%New", "Person:Business")
    assert @props << @prop
    @prop.type = "Sample.Address"
    
    assert_equal "User.Person", @cdef.name
    assert_equal 6, @cdef.properties.size
    #@cdef.save
  end
end