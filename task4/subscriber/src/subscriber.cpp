#include <iostream>
#include <string_view>

#include "general.h"
#include "subscriber.h"

/* Files required for transport initialization. */
#include <coresrv/nk/transport-kos.h>
#include <coresrv/sl/sl_api.h>

#include <mosquitto/Ping.idl.h>

using namespace std::literals;

constexpr auto Topic = "my/awesome/topic"sv;

Subscriber::Subscriber(const char *id, const char *host, int port, struct mosquitto_Ping_proxy *proxy)
    : mosquittopp(id)
{
    m_proxy = proxy;

    std::cout << app::AppTag << "Connecting to MQTT Broker with address "
              << host << " and port " << port << std::endl;

    const int keepAlive = 60;

    connect(host, port, keepAlive);
}

void Subscriber::on_connect(int rc)
{
    std::cout << app::AppTag << "Subscriber connected with code "
              << rc << std::endl;

    if (rc == 0)
    {
        subscribe(NULL, Topic.data());
    }
}

void Subscriber::on_message(const struct mosquitto_message *message)
{
    if (Topic == message->topic)
    {
        // const std::string_view payload{reinterpret_cast<char*>(message->payload),
        //                                static_cast<size_t>(message->payloadlen)};
        // std::cout << app::AppTag << "Got message with topic: " << message->topic
        //           << ", payload: " << payload << std::endl;
        int p = atoi(reinterpret_cast<char *>(message->payload));
        mosquitto_Ping_Ping_req req;
        mosquitto_Ping_Ping_res res;
        req.value = p;
        if (mosquitto_Ping_Ping(&(m_proxy->base), &req, NULL, &res, NULL) == rcOk) {
            std::cout << app::AppTag << "Subscriber sent to Showapp: " << p <<
                std::endl;
        } else {
            std::cout << app::AppTag <<
                "Subscriber failed to communicate with Showapp" << std::endl;
        }
    }
}

void Subscriber::on_subscribe(__rtl_unused int        mid,
                              __rtl_unused int        qos_count,
                              __rtl_unused const int *granted_qos)
{
    std::cout << app::AppTag << "Subscription succeeded." << std::endl;
}
