#include <cassert>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>

#include <kos_net.h>

/* Files required for transport initialization. */
#include <coresrv/nk/transport-kos.h>
#include <coresrv/sl/sl_api.h>

/* Description of the server interface used by the `client` entity. */
#include <mosquitto/Ping.idl.h>

#include "general.h"
#include "subscriber.h"

#define EXAMPLE_VALUE_TO_SEND 777

namespace consts {
constexpr const char *DefaultMqttAddress = "10.0.2.2";
constexpr int DefaultMqttUnencryptedPort = 1883;
constexpr int PublicationIntervalInSec = 5;
} // namespace consts

static std::string GetBrokerAddress()
{
    const char *brokerAddress = getenv("MQTT_BROKER_ADDRESS");
    if (!brokerAddress)
    {
        std::cerr << app::AppTag
                  << "Failed to get MQTT broker address. Using default MQTT "
                     "broker address ("
                  << consts::DefaultMqttAddress << ")" << std::endl;
        return consts::DefaultMqttAddress;
    }
    return brokerAddress;
}

static int GetBrokerPort()
{
    const char *brokerPortEnvVariable = getenv("MQTT_BROKER_PORT");
    if (!brokerPortEnvVariable)
    {
        std::cerr << app::AppTag
                  << "Failed to get MQTT broker port. Using default MQTT "
                     "broker port ("
                  << consts::DefaultMqttUnencryptedPort << ")" << std::endl;
        return consts::DefaultMqttUnencryptedPort;
    }

    try
    {
        return std::stoi(brokerPortEnvVariable);
    }
    catch (const std::invalid_argument &ex)
    {
        std::cerr << app::AppTag
                  << "Failed to get MQTT broker port: " << ex.what()
                  << "Using default port ("
                  << consts::DefaultMqttUnencryptedPort << ")" << std::endl;
        return consts::DefaultMqttUnencryptedPort;
    }
}

int main(void)
{
    NkKosTransport transport;
    struct mosquitto_Ping_proxy proxy;
    int i;

    fprintf(stderr, "Hello I'm client\n");

    /**
     * Get the client IPC handle of the connection named
     * "server_connection".
     */
    Handle handle = ServiceLocatorConnect("showapp_connection");
    assert(handle != INVALID_HANDLE);

    /* Initialize IPC transport for interaction with the server entity. */
    NkKosTransport_Init(&transport, handle, NK_NULL, 0);

    /**
     * Get Runtime Interface ID (RIID) for interface echo.Ping.ping.
     * Here ping is the name of the echo.Ping component instance,
     * echo.Ping.ping is the name of the Ping interface implementation.
     */
    nk_iid_t riid = ServiceLocatorGetRiid(handle, "mosquitto.Ping.ping");
    assert(riid != INVALID_RIID);

    /**
     * Initialize proxy object by specifying transport (&transport)
     * and server interface identifier (riid). Each method of the
     * proxy object will be implemented by sending a request to the server.
     */
    mosquitto_Ping_proxy_init(&proxy, &transport.base, riid);

    /* Request and response structures */
    mosquitto_Ping_Ping_req req;
    mosquitto_Ping_Ping_res res;

    /* Test loop. */
    // req.value = EXAMPLE_VALUE_TO_SEND;
    // for (i = 0; i < 10; ++i)
    // {
    //     /**
    //      * Call Ping interface method.
    //      * Server will be sent a request for calling Ping interface method
    //      * ping_comp.ping_impl with the value argument. Calling thread is locked
    //      * until a response is received from the server.
    //      */
    //     if (mosquitto_Ping_Ping(&proxy.base, &req, NULL, &res, NULL) == rcOk)

    //     {
    //         /**
    //          * Print result value from response
    //          * (result is the output argument of the Ping method).
    //          */
    //         fprintf(stderr, "result = %d\n", (int) res.result);
    //         /**
    //          * Include received result value into value argument
    //          * to resend to server in next iteration.
    //          */
    //         req.value = res.result;

    //     }
    //     else
    //         fprintf(stderr, "Failed to call mosquitto.Ping.Ping()\n");
    // }

    if (!wait_for_network())
    {
        std::cerr << app::AppTag << "Error: Wait for network failed!"
                  << std::endl;
        return EXIT_FAILURE;
    }

    mosqpp::lib_init();

    auto sub = std::make_unique<Subscriber>(
        "subscriber", GetBrokerAddress().c_str(), GetBrokerPort(), &proxy);
    if (sub)
    {
        sub->loop_forever();
    }

    mosqpp::lib_cleanup();
    return EXIT_SUCCESS;
}
