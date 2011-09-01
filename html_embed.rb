$KCODE='u'  
require "iconv"

puts "begin process html source ...\n"

s='';

File.open("../browser/web/search.htm").each_line do |x| 
  x.gsub!(/\n/,'')
  x.gsub!(/\"/,'\"')
  x.gsub!(/\\/,'\\')
  s << 'L"'
  s << x
  s << "\"\n"
end

s = Iconv.conv("gbk", "utf-8", s)
$KCODE=''  

File.open("GIGASOConfig.h","a") do |f|
  f << s
  f << ";"
end