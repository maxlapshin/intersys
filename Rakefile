require 'rubygems'
require 'rake/gempackagetask'

spec = Gem::Specification.new do |s|
  s.name = "intersys"
  s.version = "0.0.1"
  s.author = "Max Lapshin"
  s.email = "max@maxidoors.ru"
  s.homepage = "http://maxidoors.ru/"
  s.platform = Gem::Platform::RUBY
  s.summary = "Intersystems Cache ruby driver"
  s.files = FileList["lib/**/*"].to_a
  s.require_path = "lib"
  s.autorequire = "memcache_fragments"
  s.test_files = FileList["{test}/**/*test.rb"].to_a
  s.has_rdoc = true
  s.extra_rdoc_files = ["README"]
  #s.add_dependency("memcache-client", ">= 1.0.3")
end

Rake::GemPackageTask.new(spec) do |pkg|
pkg.need_tar = true
end