module Intersys
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
  
end
end
