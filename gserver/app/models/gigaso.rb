class Gigaso < ActiveRecord::Base
	file_column :path

	def path2
	  return "" if path.nil?
	  return "/gigaso/path/"+self.path_relative_path
	end

	def filename
	  return "" if path.nil?
	  findex = self.path_relative_path.index("/")
	  return self.path_relative_path[findex+1, self.path_relative_path.length]
	end	
	
end
