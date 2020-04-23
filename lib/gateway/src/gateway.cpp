#include "gateway/gateway.h"

#include <variant>

namespace gateway
{

void Gateway::on_device_input(const net_utils::AddressWrapper &source,
    const commons::device_messages::GenericDeviceMessage &generic_msg)
{
  using commons::subscriber_messages::DeviceNotification;

  auto &subscriptions = subscribers_.subscriptions();

  std::visit(
      [&](auto &&msg) {
        DeviceNotification notif{source.str(), msg};

        for (auto it = subscriptions.cbegin(); it != subscriptions.cend(); ++it)
        {
          auto &[client_id, s] = *it;
          auto client = subscribers_.named(client_id);

          if (!client)
          {
            // TODO Queue messages. This client must receive something at some point.
            continue;
          }

          if (s.topic != msg.topic)
          {
            continue;
          }

          client->raw_conn->send(notif.serialize());
        }
      },
      generic_msg);
}

}  // namespace gateway
