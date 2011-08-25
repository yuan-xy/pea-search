# Be sure to restart your server when you modify this file.

# Your secret key for verifying cookie session data integrity.
# If you change this key, all old sessions will become invalid!
# Make sure the secret is at least 30 characters and all random, 
# no regular words or you'll be exposed to dictionary attacks.
ActionController::Base.session = {
  :key         => '_gserver_session',
  :secret      => '703880c8ef87a3773e814db1289e3f458e2448f44df3382c2e3c93a797471d11478433748370e11409bc41bbc834f0c1f371f55f5b3b2d2ba5ae1bcb0415c19b'
}

# Use the database for sessions instead of the cookie-based default,
# which shouldn't be used to store highly confidential information
# (create the session table with "rake db:sessions:create")
# ActionController::Base.session_store = :active_record_store
