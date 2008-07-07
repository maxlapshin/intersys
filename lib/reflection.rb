module Intersys
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
      intersys_call("GetAt", index + 1)
    end
    
    def each
      0.upto(count-1) do |i|
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
        @list << prop
      end unless @loaded
      @loaded = true
      @list
    end
  end
end
end
