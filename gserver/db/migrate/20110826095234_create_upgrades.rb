class CreateUpgrades < ActiveRecord::Migration
  def self.up
    create_table :upgrades do |t|
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
    drop_table :upgrades
  end
end
