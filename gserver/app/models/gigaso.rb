class Gigaso < ActiveRecord::Base
	file_column :path

	def path2
	  return "" if path.nil?
	  return "/gigaso/path/"+self.path_relative_path
	end
end
