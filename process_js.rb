$KCODE='u'  
puts "begin process js source ...\n"

web = "#{File.dirname(__FILE__)}/browser/web"
Closure = "java -jar #{File.dirname(__FILE__)}/3rd/compiler.jar "


pack_js = %w{search.js context.js disk.js message.js scroll.js subnav.js}
extern_js = %w{jquery-1.6.min.js jquery-ui-1.8.12.custom.min.js jquery-jtemplates.js  jquery.contextmenu.r2.js grid.locale-cn.js  jquery.jqGrid.min.js}

js_pack_cmd = "#{Closure} --compilation_level SIMPLE_OPTIMIZATIONS --js_output_file pack.js  "

pack_js.each {|x| js_pack_cmd << " --js #{web}/#{x} " }
extern_js.each {|x| js_pack_cmd << " --externs #{web}/#{x} " }

js_pack_cmd << " --externs #{web}/../extern-cef.js "
js_pack_cmd << '2>null'
system(js_pack_cmd)

extern_js.each {|x| `#{Closure} --compilation_level WHITESPACE_ONLY --js #{web}/#{x} --js_output_file #{x}.tmp ` }

File.open("jz.lib",  "w+") do |f|
	extern_js.each {|js| File.open("#{js}.tmp").each_line {|x| f.puts x } }
	File.open("pack.js").each_line {|x| f.puts x }
end
#extern_js.each {|js| File.delete("#{js}.tmp") }


