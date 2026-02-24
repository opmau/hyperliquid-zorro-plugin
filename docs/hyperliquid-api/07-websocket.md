<!-- Source: https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/websocket -->

WebSocket endpoints are available for real-time data streaming and as an alternative to HTTP request sending on the Hyperliquid exchange. The WebSocket URLs by network are:

* Mainnet: `wss://api.hyperliquid.xyz/ws`
* Testnet: `wss://api.hyperliquid-testnet.xyz/ws`.

### [hashtag](#connecting) Connecting

To connect to the WebSocket API, establish a WebSocket connection to the respective URL based on the desired network. Once connected, you can start sending subscription messages to receive real-time data updates.

Example from command line:

Copy

```
$ wscat -c  wss://api.hyperliquid.xyz/ws
Connected (press CTRL+C to quit)
>  { "method": "subscribe", "subscription": { "type": "trades", "coin": "SOL" } }
< {"channel":"subscriptionResponse","data":{"method":"subscribe","subscription":{"type":"trades","coin":"SOL"}}}
```

Important: all automated users should handle disconnects from the server side and gracefully reconnect. Disconnection from API servers may happen periodically and without announcement. Missed data during the reconnect will be present in the snapshot ack on reconnect. Users can also manually query any missed data using the corresponding info request.

Note: this doc uses Typescript for defining many of the message types. The python SDK also has examples [herearrow-up-right](https://github.com/hyperliquid-dex/hyperliquid-python-sdk/blob/master/hyperliquid/utils/types.py) and example connection code [herearrow-up-right](https://github.com/hyperliquid-dex/hyperliquid-python-sdk/blob/master/hyperliquid/websocket_manager.py).

[PreviousExchange endpointchevron-left](/hyperliquid-docs/for-developers/api/exchange-endpoint)[NextSubscriptionschevron-right](/hyperliquid-docs/for-developers/api/websocket/subscriptions)

Last updated 1 month ago