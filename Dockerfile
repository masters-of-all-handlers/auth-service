FROM ghcr.io/userver-framework/docker-userver-build-base:v1a as build

WORKDIR /auth

ENV DYN_CONFIG_SERVER_ADRESS=http://10.21.0.234:8083/
ENV POSTGRES_DB=auth_db-1
ENV POSTGRES_USER=user
ENV POSTGRES_PASSWORD=password
ENV CC=
ENV CCACHE_DIR=/uservice-dynconf/.ccache
ENV CCACHE_HASHDIR=
ENV CCACHE_NOHASHDIR=
ENV CCACHE_PREFIX=
ENV CCACHE_SIZE=
ENV CMAKE_OPTS=
ENV CORES_DIR=/cores
ENV CXX=
ENV MAKE_OPTS=
ENV PREFIX=${PREFIX:-~/.local}

COPY ./ /auth
RUN ls  /auth
RUN git submodule update --init && cp -r ./third_party/userver/tools/docker/ /tools/
RUN make install

# New days  come
FROM debian:bullseye-slim

WORKDIR /auth

RUN apt-get update && apt-get install -y --allow-unauthenticated --no-install-recommends \
#	binutils-dev \
	libboost-filesystem1.74-dev \
	libboost-iostreams1.74-dev \
	libboost-locale1.74-dev \
	libboost-program-options1.74-dev \
	libboost-regex1.74-dev \
	libboost1.74-dev \
# 	libbson-dev \
	libc-ares-dev \
	libcctz-dev \
	libcurl4-openssl-dev \
	libev-dev \
 	libfmt-dev \
# 	libgrpc-dev \
#	libgrpc++-dev \
# 	libhiredis-dev \
	libhttp-parser-dev \
	libjemalloc-dev \
	libkrb5-dev \
 	libldap2-dev \
#	libmongoc-dev \
#	libpq-dev \
# 	libprotoc-dev \
#	libspdlog-dev \
	libssl-dev \
	libyaml-cpp-dev \
	postgresql-server-dev-13 \
# 	zlib1g-dev \
	sudo \
    && apt-get clean all \
	&& mkdir -p /root/.local && ln -s /auth/ /root/.local


COPY --from=build /root/.local/ ./
RUN sed -i  's/config_vars.yaml/config_vars.docker.yaml/g'  ./etc/auth/static_config.yaml

CMD ./bin/auth --config ./etc/auth/static_config.yaml
