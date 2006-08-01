#!/usr/bin/env ruby
require 'dl/import'


module Cache
  extend DL::Importable
  dlload("/Applications/Cache/bin/libcbind.dylib")
  class CacheException < StandardError
  end
  
  class ObjectNotFound < CacheException
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
    
    alias :native_methods :methods
    def methods
      native_methods + cache_methods + cache_properties + cache_setters
    end
    
    def method_missing(method, *args)
      return cache_call(method.to_s.to_wchar, args) if cache_methods.include?(method)
      return cache_get(method.to_s.to_wchar) if cache_properties.include?(method)
      return cache_set(method.to_s.to_wchar, *args) if cache_setters.include?("#{method}=")
      super(method, *args)
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

