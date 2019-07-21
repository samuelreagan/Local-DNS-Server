# Local DNS Server
- This is an implementation of a Local DNS Server utilizing a UDP client/server architecture using C. The client enters a hostname and the server returns the IP address of the hostname. The servers maintains a cache of the 5 most recent hostnames with their corresponding IP address. Each record in the cache has a TTL of 10 seconds. On each request, the server first checks if the hostname is in the local DNS cache, and returns the stored IP address if the record is present. If the hostname hasn't been cached or the record has expired, then the server retrieves the IP address from the DNS. This project was created for my Computer Networking couse in the Spring of 2019. 

# Compile
```
cd src
./run.scrt
```

# Execute
- First, start the server at a specified port number.
```
cd src
./udpserver port-number
```

- Second, run the client with the hostname that you want to look up and the port number of the running server.
```
cd src
./udpserver github.com port-number
```
