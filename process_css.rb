$KCODE='u'  
puts "begin process css source ...\n"

web = "#{File.dirname(__FILE__)}/browser/web"


pack_css = %w{jquery-ui-1.8.12.custom.css main.css thumbnail.css ui.jqgrid.css}

pack_css.each {|css| `java -jar #{File.dirname(__FILE__)}/yuicompressor-2.4.6.jar -o #{css}.tmp #{web}/#{css}` }

File.open("cz.lib",  "w+") do |f|
	pack_css.each {|css| File.open("#{css}.tmp").each_line {|x| f.puts x } }
end


