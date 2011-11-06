$KCODE='u'  

def convert_line(x)
  x.gsub!(/wcs/, '_tcs')
  x.gsub!(/wprint/, '_tprint')
  x.gsub!(/WCHAR/, 'TCHAR')
  x.gsub!(/L" ((?: [^"\\] | \\. )+) "/x, '__T("\1")')  
  x.gsub!(/L' ((?: [^'\\] | \\. )+) '/x, '__T(\'\1\')')  
end


ARGV.each do |filename|
  s=""
  File.open(filename).each_line do |x| 
  	convert_line(x)
  	s << x
  end
  File.open(filename,"w") do |f|
    f << s
  end
end

