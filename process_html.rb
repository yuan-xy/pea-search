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
  app_ico='images/app.icns'
  media_ico='images/media.icns'
  video_ico='images/media.icns'
rescue
  os='win'
  app_ico='icons/exe.ico'
  media_ico='icons/media.ico'
  video_ico='icons/video.ico'
end

File.open("#{web}/search2.htm","w") do |fo|
  fo << e.result
end