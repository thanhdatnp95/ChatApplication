CLIENT_DIR = ChatClient
SERVER_DIR = ChatServer

.PHONY: all client server clean

all:	client server

client:
	make -C $(CLIENT_DIR)

server:
	make -C $(SERVER_DIR)

clean:
	make -C $(CLIENT_DIR) clean
	make -C $(SERVER_DIR) clean
	
