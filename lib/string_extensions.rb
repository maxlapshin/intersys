# Some methods from ActiveSupport gem to remove it from dependencies

module Intersys
  module StringExtensions
    # By default, +camelize+ converts strings to UpperCamelCase. If the argument to camelize
    # is set to <tt>:lower</tt> then camelize produces lowerCamelCase.
    #
    # +camelize+ will also convert '/' to '::' which is useful for converting paths to namespaces.
    #
    #   "active_record".camelize                # => "ActiveRecord"
    #   "active_record".camelize(:lower)        # => "activeRecord"
    #   "active_record/errors".camelize         # => "ActiveRecord::Errors"
    #   "active_record/errors".camelize(:lower) # => "activeRecord::Errors"
    def camelize(first_letter = :upper)
            # def camelize(lower_case_and_underscored_word, first_letter_in_uppercase = true)
#         if first_letter_in_uppercase
#           lower_case_and_underscored_word.to_s.gsub(/\/(.?)/) { "::#{$1.upcase}" }.gsub(/(?:^|_)(.)/) { $1.upcase }
#         else
#           lower_case_and_underscored_word.first.downcase + camelize(lower_case_and_underscored_word)[1..-1]
#         end
#       end
      case first_letter
        when :upper then self.gsub(/\/(.?)/) { "::#{$1.upcase}" }.gsub(/(?:^|_)(.)/) { $1.upcase }
        when :lower then self.first.downcase + self[1..-1].camelize
      end
    end
    
    # The reverse of +camelize+. Makes an underscored, lowercase form from the expression in the string.
    # 
    # +underscore+ will also change '::' to '/' to convert namespaces to paths.
    #
    #   "ActiveRecord".underscore         # => "active_record"
    #   "ActiveRecord::Errors".underscore # => active_record/errors
    def underscore
      self.gsub(/::/, '/').
        gsub(/([A-Z]+)([A-Z][a-z])/,'\1_\2').
        gsub(/([a-z\d])([A-Z])/,'\1_\2').
        tr("-", "_").
        downcase
    end
  end
end

class String
  include Intersys::StringExtensions
end
