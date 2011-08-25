class CreateDumps < ActiveRecord::Migration
  def self.up
    create_table :dumps do |t|
      t.string :file
      t.string :os
      t.string :cpu
      t.string :disk
      t.string :ver
      t.string :user
      t.string :ip

      t.timestamps
    end
  end

  def self.down
    drop_table :dumps
  end
end
