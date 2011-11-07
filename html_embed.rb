$KCODE='u'  
require "iconv"

web = "#{File.dirname(__FILE__)}/browser/web"

#require 'pathname'
#puts Pathname.new(__FILE__).realpath
#puts File.dirname(__FILE__)
#puts Dir.pwd

puts "begin process html source ...\n"

s='wchar_t web_source[]=';

def convert_line(s,x)
  x.gsub!(/\n/,'')
  x.gsub!(/\\/,"\\\\\\\\")
  x.gsub!(/\"/,'\"')
  s << 'L"'
  s << x
  s << "\\n\"\n"
end

def convert_file(s,file)
	File.open("#{web}/#{file}").each_line do |x| 
		convert_line(s,x)
	end
end

File.open("#{web}/search2.htm").each_line do |x| 
	next if x.include? 'link rel="'
	next if x.include? 'script src="'
	if x.include? '</head>'
	  convert_line(s, '<link rel="stylesheet" href="./cz.lib" type="text/css" />') 
	  convert_line(s, '<script src="../jz.lib" type="text/javascript"></script>') 
	end
	convert_line(s,x)
end

#s = Iconv.conv("gbk", "utf-8", s)
#$KCODE=''  

File.open("html_source.h","w") do |f|
  f << "\xEF\xBB\xBF"
  f << s
  f << ";"
end