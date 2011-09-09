$KCODE='u'  
require "iconv"

#require 'pathname'
#puts Pathname.new(__FILE__).realpath
#puts File.dirname(__FILE__)
#puts Dir.pwd

puts "begin process html source ...\n"

s='wchar_t web_source[]=';

File.open("#{File.dirname(__FILE__)}/browser/web/search.htm").each_line do |x| 
  x.gsub!(/\n/,'')
  x.gsub!(/\\/,"\\\\\\\\")
  x.gsub!(/\"/,'\"')
  s << 'L"'
  s << x
  s << "\\n\"\n"
end

s = Iconv.conv("gbk", "utf-8", s)
$KCODE=''  

File.open("html_source.h","w") do |f|
  f << s
  f << ";"
end