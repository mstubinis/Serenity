#pragma once

#include "types.h"

namespace discord {

class NetworkManager final {
public:
    ~NetworkManager() = default;

    /**
     * Get the local peer ID for this process.
     */
    void GetPeerId(NetworkPeerId* peerId);
    /**
     * Send pending network messages.
     */
    Result Flush();
    /**
     * Open a connection to a remote peer.
     */
    Result OpenPeer(NetworkPeerId peerId, const char* routeData);
    /**
     * Update the route data for a connected peer.
     */
    Result UpdatePeer(NetworkPeerId peerId, const char* routeData);
    /**
     * Close the connection to a remote peer.
     */
    Result ClosePeer(NetworkPeerId peerId);
    /**
     * Open a message channel to a connected peer.
     */
    Result OpenChannel(NetworkPeerId peerId, NetworkChannelId channelId, bool reliable);
    /**
     * Close a message channel to a connected peer.
     */
    Result CloseChannel(NetworkPeerId peerId, NetworkChannelId channelId);
    /**
     * Send a message to a connected peer over an opened message channel.
     */
    Result SendMessage(NetworkPeerId peerId,
                       NetworkChannelId channelId,
                       uint8_t* data,
                       uint32_t dataLength);

    Event<NetworkPeerId, NetworkChannelId, uint8_t*, uint32_t> OnMessage;
    Event<const char*> OnRouteUpdate;

private:
    friend class Core;

    NetworkManager() = default;
    NetworkManager(NetworkManager const& rhs) = delete;
    NetworkManager& operator=(NetworkManager const& rhs) = delete;
    NetworkManager(NetworkManager&& rhs) = delete;
    NetworkManager& operator=(NetworkManager&& rhs) = delete;

    IDiscordNetworkManager* internal_;
    static IDiscordNetworkEvents events_;
};

} // namespace discord
