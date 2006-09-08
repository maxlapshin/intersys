#!/usr/bin/env ruby
require 'dl/import'
require 'rubygems'
require_gem 'activesupport'
require 'active_support'
require 'enumerator'

#
# Module, keeping all classes, required to work with Cache via object and SQL interfaces
module Intersys
  extend DL::Importable
  begin
    dlload("libcbind.dylib")
  rescue
  end
  
  require File.dirname(__FILE__) + '/intersys_cache'

  class Method
    def description
      args = []
      each_argument do |arg|
        descr = ""
        descr << "out " if arg.by_ref?
        descr << "#{arg.cache_type} #{arg.name}"
        descr << (' = '+arg.default) if arg.default
        args << descr
      end
      "#{cache_type} #{name}("+args.join(', ')+")"
    end
  end

  module Callable
    # Returns ClassDefinition object for current class
    def intersys_reflector
      @reflector ||= Intersys::Reflection::ClassDefinition.open(class_name)
    end
    
    # returns list of methods for this class
    def intersys_methods
      @methods ||= intersys_reflector._methods
    end

    # returns list of properties for current class
    def intersys_properties
      @properties ||= intersys_reflector.properties
    end
    
  #protected
    # Loads property definition with required name for required object
    # for internal use only
    def intersys_property(name)
      Property.new(database, class_name, name.to_s, self)
    end

    # Loads method definition with required name for required object
    # for internal use only
    def intersys_method(name)
      Method.new(database, class_name, name.to_s, self)
    end
    
  public
    # call class method
    def intersys_call(method_name, *args)
      intersys_method(method_name).call!(args)
    end
    alias :call :intersys_call
    
    def intersys_has_property?(property)
      self.intersys_reflector.properties.to_a.include?(property)
    end
    
    def intersys_has_method?(method)
      self.intersys_reflector._methods.to_a.include?(method)
    end
    
    # Get the specified property
    def intersys_get(property)
      intersys_property(property).get
    end
    
    # Set the specified property
    def intersys_set(property, value)
      intersys_property(property).set(value)
    end

    def method_missing(method, *args)
      method_name = method.to_s.camelize
      if match_data = method_name.match(/intersys_(.*)/)
        # Protection from errors in this method
        return super(method, *args)
      end
      if match_data = method_name.match(/(\w+)=/)
        return intersys_set(match_data.captures.first, args.first)
      end
      return intersys_get(method_name) if intersys_has_property?(method_name) && args.empty?
      begin
        return intersys_call(method_name, *args)
      rescue NoMethodError => e
      end
      begin
        return intersys_call("%"+method_name, *args)
      rescue NoMethodError => e
      end
      super(method, *args)
    end
    
  end

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
      # Nice function, that generates description of Cache class, looking just as C++ one
      # Maybe, later, it will be possible even to generate IDL, using this code
      def intersys_description
        "class #{class_name} { \n" + intersys_reflector.all_methods.map do |mtd| 
          begin
            "\t"+intersys_method(mtd).description+";\n"
          rescue
            "\tundefined #{mtd}\n"
          end
        end.join("") + "};"
      end

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

      # Returns Cache class name, if called without parameters, or sets one, if passed
      def class_name(name = nil)
        self.class_name = name if name
        self.class_name = (prefix + "." + to_s) unless @class_name
        @class_name 
      end

      # Returns database, if called without parameters, or sets one, if passed
      # Once established, it is not possible now to connect to another database
      def database(db_options = {})
        common_get_or_set("@database") do
          Intersys::Database.new({:user => "_SYSTEM", :password => "SYS", :namespace => "User"}.merge(db_options))
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

      # :nodoc
      def inherited(klass)
        class_names[klass.class_name] = klass
      end
    
      # Look into Cache documentation for what is concurrency. I don't know
      def concurrency
        1
      end
    
      # timeout for connection
      def timeout
        5
      end
    
      # Nice method, that deletes all instances of class. 
      # You can just Person.delete_extent, but Person.delete_all looks more like ActiveRecord
      def delete_all
        intersys_call("%DeleteExtent")
      end
    
    end
    
    # You can ask for database from instance
    def database
      self.class.database
    end
    
    # You can ask from instance it's Cache class name
    def class_name
      self.class.class_name
    end
    
    # Returns id of current object.
    # You can remove this method and You will get string ID, so leave it here
    # However, if You ask reflector for id, it will give incorrect answer,
    # because Cache allows id to be string
    def id
      intersys_call("%Id").to_i
    end
    
    # Destroys current object
    def destroy
      self.class.intersys_call("%DeleteId", id)
    end
    
    include Callable
  end
  Intersys::Object.extend(Callable)
  
  # Class representing one query
  # You shouldn't create it yourself
  class Query
    attr_reader :database
    
    def initialize(database, query)
      @database = database
      native_initialize(database, query.to_wchar)
    end
    
    def each
      while (row = self.fetch) && row.size > 0
        #puts "Loaded row #{row}"
        yield row
      end
    end
    
    def to_a
      data = []
      self.each {|row| data << row}
      data
    end
    
    def fill(data)
      self.each {|row| data << row}
      self
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
      data = []
      q = create_query(query).execute.fill(data).close
      #1.upto(data.first.size) do |i|
      #  puts q.column_name(i)
      #end
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
        @methods ||= intersys_get("Methods")
      end
      
      def properties
        @properties ||= intersys_get("Properties")
      end
      
      def all_methods
        _methods.to_a + self.super.split(",").map do |klass|
          klass = klass.strip
          if match_data = klass.match(/^%([^\.]+)$/)
            klass = "%Library.#{match_data.captures.first}"
          end
          self.class.open(klass).all_methods
        end.flatten
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
        @empty ||= intersys_call("IsEmpty")
      end
      
      def count
        @count ||= intersys_call("Count")
      end
      alias :size :count
      
      def [](index)
        return @list[index] if @loaded
        intersys_call("GetAt", index.to_s)
      end
      
      def each
        1.upto(count) do |i|
          yield self[i]
        end
      end
      
      include Enumerable
      
      def to_a
        load_list
      end
      
      def include?(obj)
        load_list.include?(obj)
      end
      
      def inspect
        load_list.inspect
      end
      alias :to_s :inspect
      
      def <<(object)
        intersys_call("Insert", object)
      end
      alias :insert :<<
      
      def reload
        @list = nil
        @loaded = nil
        @empty = nil
        @count = nil
      end
      
    protected
      def load_list
        @list ||= []
        self.each do |prop|
          @list << prop.intersys_get("Name")
        end unless @loaded
        @loaded = true
        @list
      end
    end
  end
end
