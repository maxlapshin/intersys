#!/usr/bin/env ruby

require 'intersys-ruby'
#my_debug

class Article < Intersys::Object
end

class Person < Intersys::Object
end

if false
  @db = Intersys::Database.new({})

  @data = @db.query("select * from sample.person")
  puts "Result: #{@data.inspect}"
  puts @data.size
  #@db.query("insert into sample.person (name,SSN) values ('test','335-66-7438')")
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
  
  if true
    @p = Person.open(1)
    puts @p
  end
end