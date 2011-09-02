class Gigaso < ActiveRecord::Base
	file_column :path

	def path2
		"/gigaso/path/"+self.path_relative_path
	end
end
