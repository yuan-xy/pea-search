require 'erb'

$KCODE='u'  
puts "begin process html source ...\n"

web = "#{File.dirname(__FILE__)}/browser/web"

fi=File.open("#{web}/search.erb.htm")
e=ERB.new(fi.read)
fi.close

begin
  if `uname`.match(/Darwin/)
    os='mac'
  else
    os='linux'
  end
rescue
  os='win'
end

File.open("#{web}/search2.htm","w") do |fo|
  fo << e.result
end