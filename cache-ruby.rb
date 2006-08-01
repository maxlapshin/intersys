#!/usr/bin/env ruby
require 'dl/import'


module Cache
  extend DL::Importable
  dlload("/Applications/Cache/bin/libcbind.dylib")
  class CacheException < StandardError
  end
  
  class ObjectNotFound < CacheException
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
    
    def initialize(database, class_name, name)
      super(database, class_name, name)
      method_initialize
      @args = []
      num_args.times do
        @args << arg!
      end
    end
    
    def call(object, *method_args)
      prepare_call!
      raise ArgumentError, "wrong number of arguments (#{method_args.size} for #{args.size})" if args.size < method_args.size
      args.each do |arg|
        object.intern_param(method_args.shift, arg)
      end
      intern_call!(object)
    end
  end
  
  class Argument < Definition
  end

  require 'cache'
  

  class Object
    
    def self.database(db = nil)
      @@database = db if db
      @@database
    end
    
    def self.class_name(name = nil)
      @@class_name = name.to_wchar if name
      @@class_name ? @@class_name.from_wchar : nil
    end
    
    def self.create
      create_intern(@@database, @@class_name)
    end

    def self.open(id)
      open_intern(@@database, @@class_name, id.to_s.to_wchar)
    end
    
    def self.property(name)
      Property.new(@@database, @@class_name, name.to_s.to_wchar)
    end
    
    def self.method(name)
      Method.new(@@database, @@class_name, name.to_s.to_wchar)
    end
    
    
    def cache_methods
      @@methods ||= intern_methods.map{|method| method.from_wchar.downcase.to_sym}
    end
    
    def cache_properties
      @@properties ||= intern_properties.map{|prop| prop.from_wchar.downcase.to_sym}
    end
    
    def cache_setters
      @@setters ||= cache_properties.map {|prop| "#{prop}=".to_sym}
    end
    
    def cache_get(property)
      intern_get(self.class.property(property))
    end
    
    def cache_set(property, value)
      intern_set(self.class.property(property), value)
    end
    
    def cache_call(method, *args)
      self.class.method(method).call(self, *args)
    end
    
    def method_missing(method, *args)
      if match_data = method.to_s.match(/(\w+)=/)
        return cache_set(match_data.captures.first, args.first)
      end
      begin
        return cache_get(method)
      rescue
        begin
          return cache_call(method, args)
        rescue
        end
      end
      super(method, *args)
    end
    
    def save
      cache_call("%Save")
    end
    
    def id
      cache_get("%Id")
    end
  end
  
  class Query
    attr_reader :database
    
    alias :native_initialize :initialize
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
      create_query(query).execute.fetch
    end
  end
end

