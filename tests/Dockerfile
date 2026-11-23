FROM ubuntu:24.04
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc libc6-dev openjdk-17-jdk-headless postgresql-16 postgresql-server-dev-16 \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /extension
COPY lib lib
COPY src/pl_clojure_handler.c src/
COPY tests/run-postgres.sh tests/integration.sql tests/
RUN JAVA_HOME="$(dirname "$(dirname "$(readlink -f "$(command -v javac)")")")" \
    && cc -fPIC -shared \
      -D'PLCLOJURE_JAR_DIR="/extension/lib"' \
      -I"$(pg_config --includedir-server)" -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" \
      src/pl_clojure_handler.c -L"$JAVA_HOME/lib/server" -Wl,-rpath,"$JAVA_HOME/lib/server" \
      -ljvm -o pl_clojure_handler.so
USER postgres
CMD ["sh", "tests/run-postgres.sh"]
