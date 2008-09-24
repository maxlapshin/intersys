module Intersys
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
      @@last_query_at = Time.now.to_i
      data
    end
    
    def active?
      now = Time.now.to_i
      (now - @@last_query_at) < connection_timeout
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
    
  private
    # return connection timeout setting in seconds
    def connection_timeout
      @connection_timeout ||= (@options[:connection_timeout] || 60) * 60
    end
  end

end
