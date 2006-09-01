#!/usr/bin/env ruby
require 'dl/import'
require 'rubygems'
require_gem 'activesupport'
require 'active_support'

#
# Module, keeping all classes, required to work with Cache via object and SQL interfaces
module Intersys
  extend DL::Importable
  dlload("/Applications/Cache/bin/libcbind.dylib")
  
  # Basic exception, thrown in intersys driver
  class IntersysException < StandardError
  end
  
  # Exception, thrown from Object.open method, when no such ID in database
  class ObjectNotFound < IntersysException
  end
  
  # Error of marshalling arguments
  class MarshallError < IntersysException
  end

  # Error of unmarshalling results
  class UnMarshallError < IntersysException
  end
  
  # Error establishing connection with database
  class ConnectionError < IntersysException
  end
  
  def self.handle_error(error_code, message, file, line)
    #raise ConnectionError if error_code == 461
  end

  # Basic class for arguments, methods and properties description
  # for internal use only
  class Definition
    def initialize(database, class_name, name)
      @database = database
      @class_name = class_name
      @name = name
      intern_initialize(database, class_name, name)
    end
  end
  
  # Class representing one object property
  # for internal use only
  class Property < Definition
  end
  
  # Class representing one object method
  # for internal use only
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
  
  # Class representing one method argument
  # for internal use only
  class Argument < Definition
    def marshall_dlist(list)
      list.each do |elem|
        marshall_dlist_element(elem)
      end
    end
  end

  # Method can return Cache %Status. It is marshalled to this class
  class Status
    attr_accessor :code
    attr_accessor :message
    def initialize(code, message)
      @code = code
      @message = message
    end
  end

  require File.dirname(__FILE__) + '/intersys_cache'
  

  # Basic class for all classes, through which is performed access to Cache classes
  # For each Cache class must be created ruby class, inherited from Intersys::Object
  # 
  # By default prefix "User" is selected. If Cache class has another prefix, it must
  # be provided explicitly via method "prefix":
  #   class List < Intersys::Object
  #     prefix "%Library"
  #   end
  #
  # By default name of Cache class is taken the same as name of ruby class.
  # Thus in this example this class List will be marshalled to Cache class 
  # %Library.List
  #
  class Object

    class << self
    protected      
      def class_names
        common_get_or_set("@class_names", {})
      end

      def prefix=(name)
        @prefix = name
        @class_name = @prefix + "." + (@class_name ? @class_name.split(".").last : self.to_s)
        register_name!
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

      # Register class name of current class in global list
      def register_name!
        if i = class_names.index(self)
          class_names.delete(i)
        end
        class_names[class_name] = self
        class_name
      end
    public    
      # Help to work with instance variables of Intersys::Object class
      # required, because list of registered descendants of Intersys::Object,
      # database connection etc. should be in one place
      def common_get_or_set(name, default_value = nil)
        unless var = Intersys::Object.instance_variable_get(name)
          default = block_given? ? yield : default_value
          var = Intersys::Object.instance_variable_set(name, default)
        end
        var
      end

      # Takes Cache class name and try to resolve it to Ruby class
      def lookup(class_name)
        class_names[class_name] || raise(UnMarshallError, "Couldn't find registered class with Cache name '#{class_name}'")
      end
    
      # Each Cache class has prefix before it's name: namespace.
      # this method set's prefix for current class is provided,
      # or just returns current prefix
      def prefix(name = nil)
        self.prefix = name if name
        @prefix ||= "User"
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
            
      # This method takes block and executes it between
      # START TRANSACTION and COMMIT TRANSACTION
      #
      # In case of exception ROLLBACK TRANSACTION is called
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

      # Returns ClassDefinition object for current class
      def reflector
        @reflector ||= Intersys::Reflection::ClassDefinition.open(class_name)
      end
      
      # returns list of methods for this class
      def intersys_methods
        @methods ||= reflector.intersys_get("Methods")
      end

      # returns list of properties for current class
      def intersys_properties
        @properties ||= reflector.properties
      end
      
      def inherited(klass)
        class_names[klass.class_name] = klass
      end
    
      def concurrency
        1
      end
    
      # timeout for connection
      def timeout
        5
      end
    
      # create new instance of this class
      def create
        create_intern
      end

      # try to load class instance from database for this id
      # ID can be not integer
      def open(id)
        open_intern(id.to_s.to_wchar)
      end
      
      # Loads property definition with required name for required object
      # for internal use only
      def property(name, object)
        Property.new(database, class_name, name.to_s.to_wchar, object)
      end
  
      # Loads method definition with required name for required object
      # for internal use only
      def method(name, object)
        Method.new(database, class_name, name.to_s.to_wchar, object)
      end
      
      # call class method
      def call(method_name, *args)
        method(method_name, nil).call(*args)
      end
      alias :intersys_call :call
    #def self.method_missing(method_name, *args)
    #end
    
    end
    
    # Get the specified property
    def intersys_get(property)
      self.class.property(property, self).get
    end
    
    # Set the specified property
    def intersys_set(property, value)
      self.class.property(property, self).set(value)
    end
    
    # Call the specified method
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
      rescue StandardError => e
        puts e
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
    
    # Returns id of current object
    def id
      intersys_call("%Id").to_i
    end
  end
  
  # Class representing one query
  # You shouldn't create it yourself
  class Query
    attr_reader :database
    
    def initialize(database, query)
      @database = database
      native_initialize(database, query.to_wchar)
    end
  end
  
  # Class representing Cache database connection
  class Database
    def create_query(query)
      Query.new(self, query)
    end
    
    # This method creates SQL query, runs it, restores data
    # and closes query
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
  
  # Module reflection keeps classes required to get information
  # about methods and properties of Cache classes
  module Reflection
    
    # This class is basic reflection class
    # If has class method Open(class_name), that creates instance of
    # this class, representing its internals
    # 
    # Usually creates via Intersys::Object.reflector
    #
    # Then it is possible to call such methods as _methods, properties
    # to get access to methods and properties of Cache class
    class ClassDefinition < Intersys::Object
      class_name "%Dictionary.ClassDefinition"
      
      # After all changes to class definition required to call save
      def save
        intersys_call("%Save")
      end
      
      # short alias to intersys_get("Methods")
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
    
    # This is a proxy object to Cache RelationshipObject, which is just like Rails Association object
    #
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
