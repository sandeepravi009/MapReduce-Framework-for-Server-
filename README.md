# MapReduce-Framework-for-Server-
This project implements a server client communication model involving some amount of data offloading from the AWS to the backend servers and later performs operations on the results it recieved form the backend servers and sends it to the client  The client sends the sequence of numbers in a input file to AWS over TCP. AWS splits it into non overlapping sets, adds one element on top of the buffer to indicate the process that has to be done by the backend servers and sends it over UDP.  The backend servers extract the first element and figures out the operation that has to done on the set of incoming data. After operating it sends the results baack to the server.  The AWS waits till it has recieved the results from all the three servers, once done it will process those three results and output a the effective result to the client.