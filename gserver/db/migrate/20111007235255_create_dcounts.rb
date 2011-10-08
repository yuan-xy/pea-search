class CreateDcounts < ActiveRecord::Migration
  def self.up
    create_table :dcounts do |t|
      t.string :ip

      t.timestamps
    end
  end

  def self.down
    drop_table :dcounts
  end
end
