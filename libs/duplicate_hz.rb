arr = IO.readlines("hz-gbk.lib")
puts "uniq hz pinyin is #{arr.length}"
$hzs=[]
$map = {}

def to_hz(arr)
 arr.each do |line|
  line.scan(/../).each do |word|
   $hzs << word;
  end
 end
 ""
end


def add(x)
 $map[x].nil?? $map[x]=1 :  $map[x]+=1
end

to_hz(arr)
$hzs.each {|x| add(x)}
puts "all hz is :#{$hzs.length}, but uniq hz is :#{$map.length}"

$dups = $map.sort{|x,y| y[1] <=> x[1] }.reject {| key, value | value<=1 } 

$dups.each {|x| puts "#{x[0]} : #{x[1]}" }