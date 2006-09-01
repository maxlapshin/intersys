#!/usr/bin/env ruby

require 'intersys-ruby'


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
    @prop = @cdef.class.property("Properties")
    @props = @cdef.intersys_get("Properties")
    puts @props.intersys_call("%Count")
    #@m @cdef.intersys_get("Methods")
  end
end