cmake_minimum_required(VERSION 2.8.4)
project(exporter)

find_package(Kafka REQUIRED)
if(Kafka_FOUND)
    include_directories(${Kafka_INCLUDE_DIR})
    include_directories(${Kafka_INCLUDE_DIR}/librdkafka)
    set(Kafka_STATIC_LIB ${Kafka_STATIC_LIB_PATH}/librdkafka.a)
    set(Kafka_STATIC_LIB_CPP ${Kafka_STATIC_LIB_PATH}/librdkafka++.a)
else()
    message(WARNING "Kafka NOT found")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES src/kafka_exporter.cpp)

include_directories(${PROJECT_SRC_DIR}/include)
add_executable(exporter ${SOURCE_FILES})


target_link_libraries(exporter ${Kafka_STATIC_LIB_CPP} ${Kafka_STATIC_LIB} lz4 bz2 -lrdkafka++ -lrdkafka -lpthread -lstdc++ -lz)