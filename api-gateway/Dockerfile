FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Копируем библиотеки и исходники
COPY libs/ /app/libs/
COPY api-gateway/ /app/api-gateway/

# Создаём корневой CMakeLists.txt
RUN echo "cmake_minimum_required(VERSION 3.20)" > /app/CMakeLists.txt && \
    echo "project(antiplagiat_app)" >> /app/CMakeLists.txt && \
    echo "add_subdirectory(api-gateway)" >> /app/CMakeLists.txt

# Собираем
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --target api-gateway

# Runtime образ
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Копируем собранный бинарник
COPY --from=builder /app/build/api-gateway/api-gateway .

EXPOSE 8080

CMD ["./api-gateway"]