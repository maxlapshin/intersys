module Intersys

module Callable
  # Returns ClassDefinition object for current class
  def intersys_reflector
    @reflector ||= Intersys::Reflection::ClassDefinition.open(class_name)
  end
  
  # returns list of methods for this class
  def intersys_methods
    @methods ||= intersys_reflector._methods.to_a
  end
  
  def intersys_method_names
    @method_names ||= intersys_methods.map { |method| method.intersys_get('Name').underscore }
  end

  # returns list of properties for current class
  def intersys_properties
    properties = Intersys::Object.common_get_or_set('@properties', {})
    properties[class_name] ||= intersys_reflector.properties.to_a
  end
  
  def intersys_property_names
    property_names = Intersys::Object.common_get_or_set('@property_names', {})
    property_names[class_name] ||= intersys_properties.map { |prop| prop.intersys_get('Name').underscore }
  end
  
  def intersys_relations
    relations = Intersys::Object.common_get_or_set('@relations', {})
    relations[class_name] ||= intersys_properties.find_all { |prop| prop.relationship }
  end
  
  def intersys_relation_names
    relation_names = Intersys::Object.common_get_or_set('@relation_names', {})
    relation_names[class_name] ||= intersys_relations.map { |prop| prop.intersys_get('Name').underscore }
  end
  
  def intersys_attributes
    attributes = Intersys::Object.common_get_or_set('@attributes', {})
    attributes[class_name] ||= intersys_properties.find_all { |prop| !prop.relationship }
  end
  
  def intersys_attribute_names
    attribute_names = Intersys::Object.common_get_or_set('@attribute_names', {})
    attribute_names[class_name] ||= intersys_attributes.map { |prop| prop.intersys_get('Name').underscore }
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
    intersys_property_names.include?(property.to_s)
  end
  
  def intersys_has_method?(method)
    intersys_method_names.include?(method.to_s)
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
    
    if match_data = method_name.match(/(\w+)=/)
      return intersys_set(match_data.captures.first, args.first)
    end
    return intersys_get(method_name) if intersys_has_property?(method) && args.empty?
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
