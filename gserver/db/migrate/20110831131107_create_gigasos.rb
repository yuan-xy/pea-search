class CreateGigasos < ActiveRecord::Migration
  def self.up
    create_table :gigasos do |t|
      t.string :version
      t.string :desc
      t.string :path

      t.timestamps
    end
  end

  def self.down
    drop_table :gigasos
  end
end
