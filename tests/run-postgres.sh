#!/bin/sh
set -eu
runtime=$(mktemp -d)
export PGHOST="$runtime"
pg_bin="$(pg_config --bindir)"
trap '"$pg_bin/pg_ctl" -D "$runtime/data" -m fast stop >/dev/null 2>&1 || true; rm -rf "$runtime"' EXIT

java -cp '/extension/lib/*' clojure.main -e '(assert (= 15 (reduce + [1 2 3 4 5])))'
"$pg_bin/initdb" -D "$runtime/data" --no-locale --encoding UTF8 --auth=trust >/dev/null
"$pg_bin/pg_ctl" -D "$runtime/data" -l "$runtime/server.log" -o "-k $runtime -c listen_addresses=''" -w start
if ! "$pg_bin/psql" -X -v ON_ERROR_STOP=1 -d postgres -f tests/integration.sql; then
  cat "$runtime/server.log" >&2
  exit 1
fi
