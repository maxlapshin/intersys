require 'rubygems'
require 'rake/gempackagetask'
require 'rake/testtask'
require 'rake/rdoctask'
require 'rake/packagetask'
require 'rake/contrib/rubyforgepublisher'
#require 'lib/intersys'

PKG_NAME = "intersys"
PKG_VERSION = "0.2.1"
PKG_AUTHOR = "Max Lapshin"
PKG_EMAIL = "max@maxidoors.ru"
PKG_HOMEPAGE = "http://maxidoors.ru/"
PKG_SUMMARY = "Intersystems Cache ruby driver"
PKG_SVN = "http://svn.maxidoors.ru/cache-ruby"
PKG_RDOC_OPTS = ['--main=README',
                 '--line-numbers',
                 '--webcvs='+PKG_SVN,
                 '--charset=utf-8',
                 '--promiscuous']


spec = Gem::Specification.new do |s|
  s.name = PKG_NAME
  s.version = PKG_VERSION
  s.author = PKG_AUTHOR
  s.email = PKG_EMAIL
  s.homepage = PKG_HOMEPAGE
  s.platform = Gem::Platform::RUBY
  s.summary = PKG_SUMMARY
  s.require_path = "lib"
  s.rubyforge_project = PKG_NAME
  s.files = FileList["{bin,test,lib}/**/*"].exclude("rdoc").exclude(".svn").exclude(".DS_Store").exclude("**/*.o").exclude("**/*.bundle").exclude("**/*.log").to_a
  s.files << ["Rakefile", "README", "init.rb"]
  s.test_files = FileList["{test}/**/*test.rb"].to_a
  s.autorequire = "intersys"
  s.has_rdoc = true
  s.extra_rdoc_files = ["README"]
  s.rdoc_options = PKG_RDOC_OPTS
  s.add_dependency("activesupport", ">= 1.0")
  s.extensions << 'lib/extconf.rb'
end

Rake::GemPackageTask.new(spec) do |pkg|
  pkg.need_tar = true
end

task :default => [ :test ]

desc "Run all tests"
Rake::TestTask.new("test") { |t|
  t.libs << "test"
  t.pattern = 'test/*.rb'
  t.verbose = true
}


desc "Report KLOCs"
task :stats  do
  require 'code_statistics'
  CodeStatistics.new(
    ["Libraries", "lib"], 
    ["Units", "test"]
  ).to_s
end


desc "Generate RDoc documentation"
Rake::RDocTask.new("doc") do |rdoc|
  rdoc.rdoc_dir = 'doc'
  rdoc.title  = PKG_SUMMARY
  rdoc.rdoc_files.include('README')
#  rdoc.rdoc_files.include('CHANGELOG')
#  rdoc.rdoc_files.include('TODO')
  rdoc.options = PKG_RDOC_OPTS
  rdoc.rdoc_files.include "lib/intersys.rb"
end

#Rake::GemPackageTask.new(spec) do |p|
#  p.gem_spec = spec
#  p.need_tar = true
#  p.need_zip = true
#end

desc "Remove packaging products (doc and pkg) - they are not source-managed"
task :clobber do
	`rm -rf ./doc`
	`rm -rf ./pkg`
end

desc "Publish the new docs"
task :publish_docs => [:clobber, :doc] do
  push_docs
end

desc "Push docs to servers"
task :push_docs do
  user = "max_lapshin@intersys.rubyforge.org" 
  project = '/var/www/gforge-projects/intersys/doc'
  local_dir = 'doc'
  [ 
    Rake::SshDirPublisher.new( user, project, local_dir),
    #Rake::SshDirPublisher.new('julik', '~/www/code/rutils', local_dir),
  ].each { |p| p.upload }
end

desc "Build binary driver"
task :build do
  puts `cd lib; [ -e Makefile ] || ruby extconf.rb; make`
end

desc "Rebuild binary driver"
task :rebuild do
  puts `cd lib; ruby extconf.rb; make clean all`
end

desc "Mark files in SVN"
task :release => [:clobber, :package] do
    svn_aware_revision = 'r_' + PKG_VERSION.gsub(/-|\./, '_')
    puts `svn copy #{PKG_SVN}/trunk #{PKG_SVN}/tags/#{svn_aware_revision} -m "release #{svn_aware_revision}"`
end
