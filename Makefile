CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -pthread
INCLUDES = -I./include -I./include/utp_sbe

# UTP Server sources
UTP_SERVER_SOURCES = src/reuters_server_multicast_main.cpp \
                    src/reuters_multicast_publisher.cpp \
                    src/reuters_encoder.cpp \
                    src/udp_multicast_transport.cpp \
                    src/tcp_transport.cpp \
                    src/reuters_protocol_adapter.cpp \
                    src/market_data_generator.cpp \
                    src/order_book.cpp \
                    src/order_book_manager.cpp

# UTP Client sources
UTP_CLIENT_SOURCES = utp_client/utp_client_main.cpp \
                    utp_client/UTPClient.cpp

# Target executables - only 2!
UTP_SERVER = utp_server
UTP_CLIENT = utp_multicast_client

all: $(UTP_SERVER) $(UTP_CLIENT)

# UTP Server build
$(UTP_SERVER): $(UTP_SERVER_SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

# UTP Client build
$(UTP_CLIENT): $(UTP_CLIENT_SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -Iutp_client $^ -o $@

clean:
	rm -f $(UTP_SERVER) $(UTP_CLIENT)

run-server:
	./$(UTP_SERVER)

run-client:
	./$(UTP_CLIENT) 239.100.1.1 15001

.PHONY: all clean run-server run-client