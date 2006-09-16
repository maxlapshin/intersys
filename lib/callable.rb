module Intersys

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


class Object
  include Callable
end
Intersys::Object.extend(Callable)

end