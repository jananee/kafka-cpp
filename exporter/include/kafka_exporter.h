# pragma once

#include <string>
#include <librdkafka/rdkafkacpp.h>

#define POLL_TIMEOUT_MS 1000
#define DELIVERY_TIMEOUT_MS 10000


enum DeliveryStatus {
    SUCCESS = 0,
    FAILED = -1,
    UNKNOWN = 1
};


class KafkaExporter {

public:
    int32_t exportData(const std::string& message, const std::string& exportTo);

};
