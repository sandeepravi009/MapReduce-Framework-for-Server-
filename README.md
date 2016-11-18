# MapReduce-Framework-for-Server-
MapReduce model was developed for clients that have have to process large data using a huge server like Amazon Web Service(AWS).
In the project I designed 
1. Client which gets a lot of data that has to be processed, makes data set with the operation that has to be performed and the actual data. It creates a socket and sends the data to AWS over TCP
2. AWS like server which gets all the data, splits them into non overlapping sets and sends it to the back end servers over UDP and performs the final operation on the outputs from the back-end servers and sends it to client over TCP.
3. Back-end servers - which get the data, unpacks the data, understands what has to be done, performs the operationa nd sends it back to the AWS over UDP
