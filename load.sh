bazel build //src:pl_clojure_handler --verbose_failures


# Create if not exists
if [ ! -d "/opt/homebrew/opt/postgresql@14/lib/postgresql@14/lib" ]; then
  sudo mkdir /opt/homebrew/opt/postgresql@14/lib/postgresql@14/lib
fi

#Copy if not exists jar files from lib folder
sudo cp lib/* /opt/homebrew/opt/postgresql@14/lib/postgresql@14/lib/
sudo chmod 755 /opt/homebrew/opt/postgresql@14/lib/postgresql@14/lib/*
sudo cp bazel-bin/src/libpl_clojure_handler.dylib /opt/homebrew/opt/postgresql@14/lib/postgresql@14/pl_clojure_handler.so
sudo chmod 755 /opt/homebrew/opt/postgresql@14/lib/postgresql@14/pl_clojure_handler.so
psql postgres -c "DROP FUNCTION IF EXISTS pl_clojure_call(text);"
psql postgres -c "CREATE FUNCTION pl_clojure_call(text) RETURNS text AS 'pl_clojure_handler', 'pl_clojure_call' LANGUAGE C STRICT;"
psql postgres -c "SELECT pl_clojure_call('(str \"Hello from \" \"Clojure!\")');"