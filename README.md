# tcp-ip-library
In progress tcp-ip library
- server listens on port 13, localhost
- multiple clients possible


Server
- server starts with one open connection listening for a client
- when a client connects to the server, the server creates a another connection and starts to listen for another client
- server sends a heart beat message to all connections that are connected to clients
- during the heartbeat the server will check if a client has disconnected and will cleanup and close the connection
- each connection has one socket

IO
- a looping timer is used to ensure the io context has a task to work on, boost asio io context requires something to do otherwise it will shutdown, 

TCPConnection
- TCPConnection is used in the server and client to receive and send
- TCPConnection has a write thread for sending messages and a read thread for receiving messages
- the write thread and read thread share the same socket 
- the read thread will read some data then sleep for a while to allow the write thread to send data
- each message is made up of a header, payload and a footer
- the header and footer help the receiver know when they have received the entire message
- when a header is detected all subsequent bytes received are accumulated in a buffer until the footer is detected.
- the header and footer are then stripped and the payload forwarded to the message handler


//  questions
what is the upper limit of clients/connections/sockets?
