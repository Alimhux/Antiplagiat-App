FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libssl-dev \
    libpq-dev \
    libpqxx-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Копируем библиотеки и исходники
COPY libs/ /app/libs/
COPY file-analysis-service/ /app/file-analysis-service/

# Создаём корневой CMakeLists.txt
RUN echo "cmake_minimum_required(VERSION 3.20)" > /app/CMakeLists.txt && \
    echo "project(antiplagiat_app)" >> /app/CMakeLists.txt && \
    echo "add_subdirectory(file-analysis-service)" >> /app/CMakeLists.txt

# Собираем
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --target file-analysis-service

# Runtime образ
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libssl3 \
    libpq5 \
    libpqxx-6.4 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Создаём директорию для отчётов
RUN mkdir -p /app/reports

# Копируем собранный бинарник
COPY --from=builder /app/build/file-analysis-service/file-analysis-service .

EXPOSE 8082

CMD ["./file-analysis-service"]