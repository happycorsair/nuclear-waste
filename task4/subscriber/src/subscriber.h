/* © 2021 AO Kaspersky Lab */
#ifndef _MOSQUITTO_SUBSCRIBER_H
#define _MOSQUITTO_SUBSCRIBER_H

#include <mosquittopp.h>
#include <rtl/compiler.h>

struct mosquitto_Ping_proxy;

class Subscriber : public mosqpp::mosquittopp
{
public:
    Subscriber(const char *id, const char *host, int port, struct mosquitto_Ping_proxy *);
    ~Subscriber() {};

    void on_connect(int rc) override;
    void on_message(const struct mosquitto_message *message) override;
    void on_subscribe(__rtl_unused int        mid,
                      __rtl_unused int        qos_count,
                      __rtl_unused const int *granted_qos) override;
private:
    struct mosquitto_Ping_proxy *m_proxy;
};

#endif // _MOSQUITTO_SUBSCRIBER_H
