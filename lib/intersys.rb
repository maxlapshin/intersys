#!/usr/bin/env ruby
require 'dl/import'
require 'rubygems'
gem 'activesupport'
require 'active_support'
require 'enumerator'

#
# Module, keeping all classes, required to work with Cache via object and SQL interfaces
module Intersys
  extend DL::Importable
  
  load_error = nil
  # try to load libraries for MacOSX and Linux
  %w{ libcbind.dylib libcbind.so }.each do |lib|
    begin
      dlload(lib)
      load_error = nil
      break
    rescue StandardError => load_error
    end
  end

  puts load_error if load_error
  
  require File.dirname(__FILE__) + '/intersys_cache'
  require File.dirname(__FILE__) + '/object'
  require File.dirname(__FILE__) + '/callable'
  require File.dirname(__FILE__) + '/reflection'


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

  
  module ActiveRecordEmulation
    def append_features(base)
      base.extend ClassMethods
    end
    
    module ClassMethods
      def create(attributes = nil)
        if attributes.is_a?(Array)
          attributes.collect { |attr| create(attr) }
        else
          object = intersys_call("%New")
          attributes.each { |att,value| object.send("#{att}=", value) }
          object.save
          object
        end
      end
      
      def exists?(id)
        exists_id(id)
      end
      
      def find_by_sql(sql)
        warn "Nop, no find_by_sql in Object interface"
        nil
      end
      
      def find(*args)
        if args.size == 1
          return open(args.first)
        end
        warn "No idea, how to implement method find"
        nil
      end
      
      
    end
  end

  require 'intersys_sql' if const_defined?(:Query)
end
