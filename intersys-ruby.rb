#!/usr/bin/env ruby
require 'dl/import'
require 'rubygems'
require_gem 'activesupport'
require 'active_support'

module Intersys
  extend DL::Importable
  dlload("/Applications/Cache/bin/libcbind.dylib")
  class IntersysException < StandardError
  end
  
  class ObjectNotFound < IntersysException
  end
  
  class MarshallError < IntersysException
  end

  class UnMarshallError < IntersysException
  end
  
  class ConnectionError < IntersysException
  end
  
  def self.handle_error(error_code, message, file, line)
    #raise ConnectionError if error_code == 461
  end
  
  class Definition
    def initialize(database, class_name, name)
      @database = database
      @class_name = class_name
      @name = name
      intern_initialize(database, class_name, name)
    end
  end
  
  class Property < Definition
  end
  
  class Method < Definition
    attr_accessor :args
  protected
    def arg!
      Argument.new(@database, @class_name, @name, self)
    end
  public
    
    def initialize(database, class_name, name, object)
      super(database, class_name, name.to_s)
      method_initialize(object)
      @args = []
      num_args.times do
        @args << arg!
      end
    end
    
    def call(*method_args)
      prepare_call!
      raise ArgumentError, "wrong number of arguments (#{method_args.size} for #{args.size})" if method_args.size > args.size 
      args.each_with_index do |arg, i|
        arg.marshall!(method_args[i])
      end
      intern_call!
      extract_retval!
    end
  end
  
  class Argument < Definition
    def marshall_dlist(list)
      list.each do |elem|
        marshall_dlist_element(elem)
      end
    end
  end
  
  class Status
    attr_accessor :code
    attr_accessor :message
    def initialize(code, message)
      @code = code
      @message = message
    end
  end

  require 'intersys'
  

  class Object

    class << self
      def common_get_or_set(name, default_value = nil)
        unless var = Intersys::Object.instance_variable_get(name)
          default = block_given? ? yield : default_value
          var = Intersys::Object.instance_variable_set(name, default)
        end
        var
      end
      
      def class_names
        common_get_or_set("@class_names", {})
      end
      
      def lookup(class_name)
        class_names[class_name] || raise(UnMarshallError, "Couldn't find registered class with Cache name '#{class_name}'")
      end

      def prefix=(name)
        @prefix = name
        @class_name = @prefix + "." + (@class_name ? @class_name.split(".").last : self.to_s)
        register_name!
      end
    
      def prefix(name = nil)
        self.prefix = name if name
        @prefix ||= "User"
      end

      def class_name=(name)
        if name.index(".")
          self.prefix = name.split(".").first
          @class_name = name
        else
          @class_name = self.prefix + "." + name
        end
        register_name!
      end

      def class_name(name = nil)
        self.class_name = name if name
        self.class_name = (prefix + "." + to_s) unless @class_name
        @class_name 
      end

      def database(db = nil)
        common_get_or_set("@database") do
          db || Intersys::Database.new(:user => "_SYSTEM", :password => "SYS", :namespace => "User")
        end
      end
            
      def register_name!
        if i = class_names.index(self)
          class_names.delete(i)
        end
        class_names[class_name] = self
        class_name
      end
      
      def transaction
        return unless block_given?
        database.start
        begin
          yield
          database.commit
        rescue StandardError => e
          database.rollback
          raise e
        end
      end

      def reflector
        @reflector ||= Intersys::Reflection::ClassDefinition.open(class_name)
      end
      
      def intersys_methods
        @methods ||= reflector.intersys_get("Methods")
      end

      def intersys_properties
        @properties ||= reflector.properties
      end
      
      def inherited(klass)
        class_names[klass.class_name] = klass
      end
    
      def concurrency
        1
      end
    
      def timeout
        5
      end
    
      def create
        create_intern
      end

      def open(id)
        open_intern(id.to_s.to_wchar)
      end
    
      def property(name, object)
        Property.new(database, class_name, name.to_s.to_wchar, object)
      end
    
      def method(name, object)
        Method.new(database, class_name, name.to_s.to_wchar, object)
      end
    
      def call(method_name, *args)
        method(method_name, nil).call(*args)
      end
      alias :intersys_call :call
    #def self.method_missing(method_name, *args)
    #end
    
    end
    
    
    def intersys_get(property)
      self.class.property(property, self).get
    end
    
    def intersys_set(property, value)
      self.class.property(property, self).set(value)
    end
    
    def intersys_call(method, *args)
      self.class.method(method, self).call(*args)
    end
    
    def method_missing(method, *args)
      method_name = method.to_s.camelize
      if match_data = method_name.match(/(\w+)=/)
        return intersys_set(match_data.captures.first, args.first)
      end
      begin
        return intersys_get(method_name)
      rescue
        begin
          return intersys_call(method_name, *args)
        rescue
        end
      end
      super(method, *args)
    end
    
    def save
      intersys_call("%Save")
    end
    
    def id
      intersys_call("%Id").to_i
    end
  end
  
  class Query
    attr_reader :database
    
    def initialize(database, query)
      @database = database
      native_initialize(database, query.to_wchar)
    end
  end
  
  class Database
    def create_query(query)
      Query.new(self, query)
    end
    
    def query(query)
      q = create_query(query).execute
      data = []
      while (row = q.fetch) && row.size > 0
        data << row
      end
      q.close
      1.upto(data.first.size) do |i|
        puts q.column_name(i)
      end
      data
    end
  end
  
  module Reflection
    class ClassDefinition < Intersys::Object
      class_name "%Dictionary.ClassDefinition"
      
      def save
        intersys_call("%Save")
      end
      
      def _methods
        intersys_get("Methods")
      end
    end

    class PropertyDefinition < Intersys::Object
      class_name "%Dictionary.PropertyDefinition"
    end
    
    class MethodDefinition < Intersys::Object
      class_name "%Dictionary.MethodDefinition"
    end
    
    class RelationshipObject < Intersys::Object
      class_name "%Library.RelationshipObject"
      
      def empty?
        intersys_call("IsEmpty")
      end
      
      def count
        intersys_call("Count")
      end
      alias :size :count
      
      def [](index)
        intersys_call("GetAt", index.to_s)
      end
      
      def each
        1.upto(count) do |i|
          yield self[i]
        end
      end
      
      def each_with_index
        1.upto(count) do |i|
          yield self[i], i
        end
      end
      
      def inspect
        list = []
        each do |prop|
          list << prop.name
        end
        list.inspect
      end
      alias :to_s :inspect
      
      def <<(object)
        intersys_call("Insert", object)
      end
      alias :insert :<<
    end
  end
end
