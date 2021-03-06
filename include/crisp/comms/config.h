#ifndef crisp_comms_config_h
#define crisp_comms_config_h 1

#define MODULE_MAX_INPUTS 15

#define USE_PACKED_FIELDS 1

#define ENABLE_ASSERT 1

#undef MESSAGE_USE_SEQUENCE_ID

#define MESSAGE_CHECKSUM_SIZE 4

#define PROTOCOL_VERSION 0x00
#define PROTOCOL_NAME (('U' << 24) | ('W' << 16) | ('R' << 8) | 'T')

#define PROTOCOL_NODE_DEFAULT_SOCKET_TYPE boost::asio::ip::tcp::socket

#define SYNC_INTERVAL 1

/** How long after a message-handler signal emission we should wait before
    freeing the message-body object. */
#define MESSAGE_HANDLER_SIGNAL_FREE_DELAY 1

#endif	/* crisp_comms_config_h */
