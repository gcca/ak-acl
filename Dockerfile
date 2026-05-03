FROM alpine:latest AS builder

RUN apk add --no-cache \
    g++ \
    cmake \
    ninja \
    git \
    openssl-dev \
    sqlite-dev \
    zlib-dev \
    util-linux-dev \
    jsoncpp-dev \
    c-ares-dev \
    brotli-dev

RUN git clone --depth 1 --branch v1.9.12 https://github.com/drogonframework/drogon.git /tmp/drogon && \
    cd /tmp/drogon && \
    git submodule update --init && \
    cmake -B build -GNinja \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_EXAMPLES=OFF \
        -DBUILD_CTL=OFF && \
    cmake --build build && \
    cmake --install build && \
    rm -rf /tmp/drogon

WORKDIR /app

COPY . .

RUN cmake -B build -GNinja -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

FROM alpine:latest

RUN apk add --no-cache \
    libstdc++ \
    openssl \
    sqlite-libs \
    zlib \
    util-linux \
    jsoncpp \
    c-ares \
    brotli-libs

WORKDIR /app

COPY --from=builder /usr/local/lib/libdrogon.so* /usr/local/lib/
COPY --from=builder /usr/local/lib/libtrantor.so* /usr/local/lib/
COPY --from=builder /app/build/ak-acl /app/
COPY --from=builder /app/build/ak-acl_cmd_create-application /app/
COPY --from=builder /app/build/ak-acl_cmd_create-user /app/

RUN ldconfig /usr/local/lib

CMD ["./ak-acl"]
