require "iconv"

Dir["*.{c,h,cpp}"].each do |file|
  File.open("#{file}.tmp","w") do |f|
    f << "\xEF\xBB\xBF"
    File.open(file).each_line {|s| f << Iconv.conv("utf-8", "gbk", s) }
  end
end

Dir["*.tmp"].each do |file|
  File.rename(file,file[0,file.length-4])
end