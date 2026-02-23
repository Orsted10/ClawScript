FROM alpine:3.19 AS build
RUN apk add --no-cache build-base cmake ninja llvm clang musl-dev git
WORKDIR /src
# Expect code to be mounted at /src
# Configure for Release with musl toolchain
RUN cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --config Release --target claw claw_fmt claw_lsp

FROM alpine:3.19
WORKDIR /app
COPY --from=build /src/build/bin /app/bin
ENTRYPOINT ["/app/bin/claw"]
