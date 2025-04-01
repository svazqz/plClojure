#!/bin/bash

# Build the handler
bazel build //src:pl_clojure_handler --verbose_failures

# Create lib directory if it doesn't exist
if [ ! -d "/opt/homebrew/opt/postgresql@14/lib/postgresql@14/lib" ]; then
  sudo mkdir -p /opt/homebrew/opt/postgresql@14/lib/postgresql@14/lib
fi

# Stop PostgreSQL and clear logs
brew services stop postgresql@14
sudo rm -f /opt/homebrew/var/log/postgresql@14.log

# Copy files and set permissions
sudo cp lib/* /opt/homebrew/opt/postgresql@14/lib/postgresql@14/lib/
sudo chmod 755 /opt/homebrew/opt/postgresql@14/lib/postgresql@14/lib/*
sudo cp bazel-bin/src/libpl_clojure_handler.dylib /opt/homebrew/opt/postgresql@14/lib/postgresql@14/pl_clojure_handler.so
sudo chmod 755 /opt/homebrew/opt/postgresql@14/lib/postgresql@14/pl_clojure_handler.so

# Start PostgreSQL with debug logging
brew services start postgresql@14

# Wait for PostgreSQL to be ready
until pg_isready; do
  echo "Waiting for PostgreSQL to start..."
  sleep 1
done

# Test the function separately
psql postgres -c "DROP FUNCTION IF EXISTS pl_clojure_call_array(text, anyarray);"
psql postgres -c "CREATE FUNCTION pl_clojure_call_array(text, anyarray) RETURNS text AS 'pl_clojure_handler', 'pl_clojure_call_array' LANGUAGE C STRICT;"

# Try the function and capture any errors
psql postgres -v ON_ERROR_STOP=1 -c "SELECT pl_clojure_call_array('(fn [& args] (apply str args))', ARRAY['Hello', ' ', 'World']);"

# If there's an error, check the logs
if [ $? -ne 0 ]; then
    echo "Error occurred. Checking PostgreSQL logs..."
    tail -n 50 /opt/homebrew/var/log/postgresql@14.log
fi