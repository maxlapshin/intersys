#!/usr/bin/env ruby

require 'intersys-ruby'
#my_debug

class Article < Intersys::Object
end

class Person < Intersys::Object
end

if nil
  @db = Intersys::Database.new(:user => "_SYSTEM", :password => "SYS", :namespace => "User")


  @data = @db.query("insert into articles (name) values ('test name')")
  puts "Result: #{@data.inspect}"


  @data = @db.query("select name from articles where id = 23")
  puts "Result: #{@data.inspect}"
end

if nil
  puts Article.new.instance_variable_get("@class_name")
  @a = Article.create
  puts @a.class
  Article.property("name")
  puts Article.new.cache_get("name")
end

if nil
  @a = Article.open(21)
  puts @a.name
  @a.name = "Anni Fyo"
  @a.save
  @a = Article.open(21)
  puts @a.name
  puts @a.id
end
if true
  
  
  Article.transaction do
    @cdef = Intersys::Reflection::ClassDefinition.open("%Dictionary.ClassDefinition")
    #puts @cdef.name
    #puts @cdef.intersys_get("ClassType")
    #puts @cdef.intersys_get("Super")
    #@props = @cdef.intersys_get("Properties")
    @props = @cdef.properties
    puts @props.inspect
    #@methods = @cdef.intersys_get("Methods")
    #puts Intersys::Reflection::ClassDefinition.intersys_methods.inspect
    #GC.start
    #puts Intersys::Reflection::ClassDefinition.intersys_properties.inspect
  end if false
  
  if false
    @cdef = Intersys::Reflection::ClassDefinition.call("%New", "User.Person")
    @cdef.class_type = "persistent"
    @cdef.super = "%Persistent,%Populate,%XML.Adaptor"
    @props = @cdef.properties

    @prop = Intersys::Reflection::PropertyDefinition.intersys_call("%New", "Person:Name")
    @props << @prop
    @prop.type = "%String"

    @prop = Intersys::Reflection::PropertyDefinition.intersys_call("%New", "Person:SSN")
    @props << @prop
    @prop.type = "%String"

    @prop = Intersys::Reflection::PropertyDefinition.intersys_call("%New", "Person:Title")
    @props << @prop
    @prop.type = "%String"

    @prop = Intersys::Reflection::PropertyDefinition.intersys_call("%New", "Person:DOB")
    @props << @prop
    @prop.type = "%String"
    
    @prop = Intersys::Reflection::PropertyDefinition.intersys_call("%New", "Person:Home")
    @props << @prop
    @prop.type = "Sample.Address"
    
    @prop = Intersys::Reflection::PropertyDefinition.intersys_call("%New", "Person:Business")
    @props << @prop
    @prop.type = "Sample.Address"
    
    @cdef.save
  end
  #@cdef = Intersys::Reflection::ClassDefinition.open("User.Person")
  if false
    @p = Person.intersys_call("PopulateSerial")
    puts @p.id
    Person.intersys_call("Populate", 1000)
    puts Person.intersys_properties.inspect
  end
  
  
end