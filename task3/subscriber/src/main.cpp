#include <cstring>
#include <exception>
#include <iostream>
#include <memory>

#include <kos_net.h>

#include "general.h"
#include "subscriber.h"

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
    if (!wait_for_network())
    {
        std::cerr << app::AppTag << "Error: Wait for network failed!"
                  << std::endl;
        return EXIT_FAILURE;
    }

    mosqpp::lib_init();

    auto sub = std::make_unique<Subscriber>(
        "subscriber", GetBrokerAddress().c_str(), GetBrokerPort());
    if (sub)
    {
        sub->loop_forever();
    }

    mosqpp::lib_cleanup();
    return EXIT_SUCCESS;
}