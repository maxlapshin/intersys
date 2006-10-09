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
  rescue StandardError => e
    puts e
  end
  
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

  
  # Class representing one query
  # You shouldn't create it yourself
  class Query
    attr_reader :database
    
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
  protected
    def strip_param(query, name)
      if match_data = query.match(/#{name}(\s*)(\d+)/i)
        query[match_data.to_s] = ""
        match_data.captures.last
      end
    end
  
  public
    def create_query(query)
      @limit = strip_param(query, "LIMIT")
      @offset = strip_param(query, "OFFSET")
      q = Query.new(self, query)
      q.limit = @limit if @limit
      q.offset = @offset if @offset
      q
    end
    
    # This method creates SQL query, runs it, restores data
    # and closes query
    def query(query, params = [])
      data = []
      q = create_query(query).bind_params(params).execute.fill(data).close
      #1.upto(data.first.size) do |i|
      #  puts q.column_name(i)
      #end
      data
    end
    
    def execute(query, params = [])
      create_query(query).bind_params(params).execute.close
    end
    
    # TODO: /csp/docbook/DocBook.UI.Page.cls?KEY=RSQL_variables
    # Somehow, I should extract from Cache %ROWCOUNT and %ROWID
    def affected_rows
      query("select %ROWCOUNT")
    end
    
    def insert_id
      0
    end
  end
  
end
