# collaborators
Roberto Ayuso
Princewill Okorie
Woo Seok Yang

# chat
chat - Network Chat Client

# Usage:

server *IP address* *port number*

client *IP address* *port number*

#Description

The goal of this assignment is to write a networked chat server and client. Multiple users will connect concurrently to a single server process via individual chat clients. The client program will send messages from users to the server, and will print messages from the server to the user's terminal screen. The server will keep track of connected clients, and when it recieves a message from any client it will pass that message along to all other connected clients. Each user will also have a name that is used to identify their statements.

Each user, when they first connect, should get a default name consisting of the string "User" followed by a unique set of digits. When a user connects to the server any other connected users should recieve a message stating " *user name* has connected".

Additionally, the client will support two special commands name and quit.

name command: When the client enters the phrase "name" followed by a space and at least one character, everything after the space will become the user's name. The system will then send this name in front of any subsequent chat messages. If the user specifies another name their current name is replaced. Lastly, when any user changes their name then all other users should recieve the message " *old name* has changed their name to *new name* "

quit command: When the client enters the phrase "quit" the client program will terminate. All other connected users should recieve the message " *user name* has quit".

Messages should always be printed to all terminals in less than a second, regardless of the current state of the system. (This means that your client should be able to display text even while waiting for input, and your server should be able to respond to an incoming request by any client at any time.)

If a client suddenly finds its socket to be closed then it should quit and print a message saying the server shut down unexpectedly. If the server finds a socket unable to be used it should print the same message as though the user quit normally.
