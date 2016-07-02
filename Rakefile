require 'fileutils'
require 'rake/clean'

def use_bundler()
  if system("bundle > /dev/null 2>&1")
    "bundle exec "
  else
    ""
  end
end

def build(mode, chapter)
  sh "#{use_bundler}review-compile --target=#{mode} --footnotetext --singledirmode --stylesheet=style.css #{chapter} > tmp"
  mode_ext = {
    "html" => "html",
    "text" => "txt",
    "latex" => "tex",
    "idgxml" => "xml",
    "inao" => "inao"
  }
  FileUtils.mv "tmp", chapter.gsub(/re\z/, mode_ext[mode])
end

def build_all(mode)
  sh "#{use_bundler}review-compile --all --target=#{mode} --footnotetext --singledirmode --stylesheet=style.css"
end

["html", "text", "idgxml"].each{|mode|
  desc "build #{mode} (Usage: rake build re=target.re)"
  task mode.to_sym do
    if ENV['re'].nil?
      puts "Usage: rake build re=target.re"
      exit
    end
    build(mode, ENV['re'])
  end

  desc "build all #{mode}"
  task "#{mode}_all".to_sym do
    build_all(mode)
  end
}

task :default => [:lint, :pdf]

desc 'generate PDF and EPUB file'
task :all => [:lint, :pdf, :epub]

desc 'generate PDF file'
task :pdf => ["book.pdf", "book-print.pdf"]

desc 'generate EPUB file'
task :epub => "book.epub"

SRC = FileList['*.re'] + ["config.yml"]
PRINT_SRC = FileList['*.re'] + ['config_print.yml']

task :lint do
  for re in SRC do
    sh "./node_modules/.bin/textlint #{re}"
  end
end

file "book.pdf" => SRC do
  sh "rm -f book.pdf"
  sh "rm -rf book book-pdf"
  sh "#{use_bundler}review-pdfmaker config.yml"
end

file "book-print.pdf" => PRINT_SRC do
  sh "rm -f book-print.pdf"
  sh "rm -rf book book-print-pdf"
  sh "#{use_bundler}review-pdfmaker config_print.yml"
end

file "book.epub" => SRC do
  sh "rm -f book.epub"
  sh "rm -rf book book-epub"
  sh "#{use_bundler}review-epubmaker config.yml"
end

CLEAN.include(["book", "book-pdf", "book.pdf", "book-print.pdf","book-print-pdf","book-epub", "book.epub"])
Dir["#{File.dirname(__FILE__)}/*.re"].sort.each do |path|
  ["txt", "html", "xml"].each{|ext|
    file_path = "#{File.dirname(__FILE__)}/#{File.basename(path, '.re')}.#{ext}"
    if File.exists? file_path
      CLEAN.include("#{File.basename(path, '.re')}.#{ext}")
    end
  }
end
