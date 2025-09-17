# Reuters UTP Multicast Implementation

This directory contains a complete, self-contained implementation of Reuters market data distribution using UDP multicast with UTP SBE (Simple Binary Encoding).

## Contents

- **`utp_server`** - UTP multicast server that sends market data using correct UTP SBE encoding
- **`utp_multicast_client`** - Client that connects to UTP multicast feed 
- **`include/utp_sbe/`** - Complete UTP SBE message headers from the UTP protocol
- **`src/`** - All source code for the UTP server implementation
- **`utp_client/`** - UTP multicast client source code

## Building

```bash
make clean
make all
```

This builds both executables using the correct UTP SBE headers from `../protocols/utp/include/utp_sbe/utp_sbe/`.

## UTP Messages

The implementation uses the correct UTP SBE messages from UTP_CLIENT_Multicast_MD.xml:

### Message Types:
- **AdminHeartbeat** (ID=10) - Keep-alive messages
- **SecurityDefinition** (ID=18) - Instrument definitions 
- **MDFullRefresh** (ID=20) - Full order book snapshots
- **MDIncrementalRefresh** (ID=21) - Incremental market data updates
- **MDIncrementalRefreshTrades** (ID=111) - Trade updates

## Running the UTP Server

The server sends market data via UDP multicast using proper UTP SBE encoding:

```bash
./utp_server
```

The server will:
1. Create 7 FX instruments (EUR/USD, GBP/USD, etc.)
2. Generate realistic market data
3. Send SecurityDefinition messages via multicast
4. Send MDFullRefresh (full snapshots) 
5. Send MDIncrementalRefresh (live updates)
6. Send AdminHeartbeat messages

## Running the UTP Client

In another terminal, start the client to listen to UTP multicast:

```bash
./utp_multicast_client 239.100.1.1 15001
```

The client will:
1. Connect to UTP multicast feed
2. Receive and parse UTP SBE messages
3. Display market data updates in real-time
4. Show hex dumps for debugging

## Protocol Details

- **Transport**: Pure UDP Multicast (no TCP)
- **Encoding**: UTP SBE (Simple Binary Encoding)
- **Schema**: UTP_CLIENT_Multicast_MD.xml (Schema ID 101, Version 1)
- **Byte Order**: Little Endian
- **Message Format**: Binary SBE with proper headers

## Key Features

✅ **Correct UTP SBE Implementation** - Uses proper UTP headers from UTP protocol  
✅ **UDP Multicast Distribution** - Efficient market data dissemination  
✅ **Real-time Market Data** - 7 FX instruments with live price generation  
✅ **Self-Contained** - Only 2 executables, no external dependencies  
✅ **Production-Ready** - Proper UTP SBE encoding and message structure

## Testing

1. **Start the server:**
```bash
./utp_server
```

2. **In another terminal, start the client:**
```bash
./utp_multicast_client 239.100.1.1 15001
```

3. **You should see:**
   - Server: Creating instruments and generating market data
   - Client: Connecting and receiving UTP multicast messages

## Architecture

```
UTP Client (UDP) ←→ UTP Server ←→ UDP Multicast Feed (239.100.1.1:15001)
                                   ↓
                            [UTP SBE Encoded Messages]
                            - AdminHeartbeat (ID=10)
                            - SecurityDefinition (ID=18)
                            - MDFullRefresh (ID=20)
                            - MDIncrementalRefresh (ID=21)
                            - MDIncrementalRefreshTrades (ID=111)
```

This is a **complete, working UTP implementation** with correct UTP SBE headers from `../protocols/utp/include/utp_sbe/utp_sbe/` and working UDP multicast market data distribution using the UTP_CLIENT_Multicast_MD.xml schema.