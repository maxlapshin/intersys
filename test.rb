#!/usr/bin/env ruby

require 'intersys-ruby'

exit

class Article < Cache::Object
  database Cache::Database.new(:user => "Admin", :password => "123", :namespace => "User")
  class_name "User.Article"
end

if nil
class Person < Cache::Object
  database Cache::Database.new(:user => "Admin", :password => "123", :namespace => "User")
  class_name "User.Person"
end
end

if nil
  @db = Cache::Database.new(:user => "_SYSTEM", :password => "SYS", :namespace => "User")


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


if true
  @a = Article.open(21)
  puts @a.name
  @a.name = "Anni Fyo"
  @a.save
  @a = Article.open(21)
  puts @a.name
  puts @a.id
end
if true
  puts Cache::Object.class_names.inspect
end