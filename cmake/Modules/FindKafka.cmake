message(STATUS "KAFKA_HOME: $ENV{KAFKA_HOME}")

find_path(Kafka_INCLUDE_DIR
  NAMES librdkafka/rdkafkacpp.h
        librdkafka/rdkafka.h
  HINTS $ENV{KAFKA_HOME}/include/ /usr/local/include/ /usr/include/
)

# find static lib path
find_path(Kafka_STATIC_LIB_PATH
  NAMES librdkafka++.a
  HINTS $ENV{KAFKA_HOME}/usr/local/lib/ /usr/local/lib/ /usr/lib/
)

if (Kafka_STATIC_LIB_PATH)
  set(Kafka_FOUND TRUE)
else ()
  set(Kafka_FOUND FALSE)
endif ()

if (Kafka_FOUND)
  message(STATUS "Kafka include dir: ${Kafka_INCLUDE_DIR}")
  message(STATUS "Kafka static library: ${Kafka_STATIC_LIB_PATH}")
endif ()