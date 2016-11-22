all:

	gcc -o server chat_server.c -lpthread
	gcc -o client chat_client.c -lpthread
