#include <iostream>
#include <librdkafka/rdkafkacpp.h>
#include <signal.h>
#include <chrono>
#include <thread>

#include "../include/kafka_exporter.h"

using namespace std;

class MessageDeliveryReportCb : public RdKafka::DeliveryReportCb {
public:
    void dr_cb (RdKafka::Message &message) {
        std::cout << "Delivery called" << std::endl;
        if(message.errstr() == "Success")
            std::cout << "Message delivery for (" << message.len() << " bytes): " <<
            message.errstr() << std::endl;
        else {
            std::cout << "Message delivery for (" << message.len() << " bytes): " <<
            message.errstr() << std::endl;
        }
        DeliveryStatus *deliveryStatus = (DeliveryStatus*)message.msg_opaque();
        *deliveryStatus = message.err() == RdKafka::ErrorCode::ERR_NO_ERROR ? DeliveryStatus::SUCCESS : DeliveryStatus::FAILED;
    }
};

class MessageEventCb : public RdKafka::EventCb {
public:
    void event_cb (RdKafka::Event &event) {
        switch (event.type())
        {
            case RdKafka::Event::EVENT_ERROR:
                std::cerr << "ERROR (" << RdKafka::err2str(event.err()) << "): " <<
                event.str() << std::endl;
                break;

            case RdKafka::Event::EVENT_STATS:
                std::cerr << "\"STATS\": " << event.str() << std::endl;
                break;

            case RdKafka::Event::EVENT_LOG:
                fprintf(stderr, "LOG-%i-%s: %s\n",
                        event.severity(), event.fac().c_str(), event.str().c_str());
                break;

            default:
                std::cerr << "EVENT " << event.type() <<
                " (" << RdKafka::err2str(event.err()) << "): " <<
                event.str() << std::endl;
                break;
        }
    }
};

MessageEventCb message_event_cb;
MessageDeliveryReportCb message_dr_cb;

int32_t KafkaExporter::exportData(const std::string &message, const std::string &topicName) {

    int32_t status;

    std::string brokers = "localhost:9092";
    std::string maxMessageSize = "10485760";

    int32_t partition = RdKafka::Topic::PARTITION_UA;

    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    std::string err_str;

    conf->set("message.max.bytes", maxMessageSize, err_str);
    conf->set("compression.codec", "gzip", err_str);

    conf->set("dr_cb", &message_dr_cb, err_str);
    conf->set("event_cb", &message_event_cb, err_str);
    conf->set("metadata.broker.list", brokers, err_str);

    RdKafka::Producer *producer =  RdKafka::Producer::create(conf, err_str);

    if(producer) {

        RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

        RdKafka::Topic *topic = RdKafka::Topic::create(producer, topicName, tconf, err_str);

        DeliveryStatus deliveryStatus = DeliveryStatus::UNKNOWN;
        RdKafka::ErrorCode resp = producer->produce(topic, partition, RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
                                                    const_cast<char *>(message.c_str()), message.size(), NULL, &deliveryStatus);

        if (resp != RdKafka::ERR_NO_ERROR) {
            cout << "% Produce failed: " <<
            RdKafka::err2str(resp) << ", Message size: " << message.length() <<  " bytes" << std::endl;
            status = -1;
        }
        else {
            producer->poll(0);

            long timeElapsed = 0;
            long cutOffInMillis = DELIVERY_TIMEOUT_MS;
            while (deliveryStatus == DeliveryStatus::UNKNOWN && timeElapsed < cutOffInMillis) {
                producer->poll(POLL_TIMEOUT_MS);
                timeElapsed += POLL_TIMEOUT_MS;
            }
            cout << deliveryStatus << endl;
            status = deliveryStatus == DeliveryStatus::SUCCESS ? 0 : -1;
        }

        delete topic;
    }
    else {
        cout << "Failed to create producer: " << err_str << std::endl;
        status = -1;
    }

    delete producer;

    return status;
}



int main() {
    KafkaExporter exporter;
    int32_t status = exporter.exportData("123", "test_lrk");
    cout << status << endl;
    return 0;
}