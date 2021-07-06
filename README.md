# DNS Proxy

## Table of contents
* [About](#about)
* [Project Structure](#project-structure)
* [Running Project](#running-project)
* [DNS TCP Overview](#dns-tcp-overview)

<hr>

## About
Application can recieve and respond to TCP [IPv6 DNS][1] queries. Contents of queries are recorded and a log file is generated providing a history of all requests through proxy. Application caches recent queries, queries are expired in accordance to [TLL (time-to-live)][2].

[1]: https://datatracker.ietf.org/doc/html/rfc3596
[2]: https://datatracker.ietf.org/doc/html/rfc1035#section-3.2

### What is DNS?
Computers on the internet use IP addresses to communicate with each other. These addresses act as a unique identifier allowing traffic to be delivered to the correct destination. IP addresses like `142.250.70.206` (IPv4) or `2404:6800:4015:802::200e` (IPv6) are difficult to remember for humans as such we use hostnames such as `google` or `youtube` instead. However, our system still needs a way to find the IP from the hostname in order to communicate. This is where DNS comes in. DNS (Domain Name System) returns an IP address given a hostname, done by DNS servers communicating with nameservers which hold records containing hostnames and there IP address, if a nameserver contains the records it will return the IP address, if not the DNS server will then check another nameserver, the process repeats until the address is found. Nameservers are usually set up in a hierarchical structure as shown below.

<br/>
<p align="center">
<img src="./images/nameserver.png" alt="DNS Nameserver Hierarchy" width="500">
<p style="text-align:center">DNS Nameserver Hierarchy</p>
</p>
<br/><br/>
<p align="center">
<img src="./images/DNS example.png" alt="DNS Nameserver Hierarchy" width="500">
<p style="text-align:center">DNS Process Example</p>
</p>

## Project Structure
| Directory | Description |
| :-- | :-- |
| main.c | Entry point of program |
| headers/| Folder containing all header files for program |
| modules/| Folder containing all c files for subsequent header files |
| modules/dns.model.c | Models DNS query structure into struct |
| modules/socket.c | Provides various socket related functions such as creating server and client sockets |
| modules/cache.c | Provides caching functionality. Utilised linked list |
| modules/log.c | Provides logging functions to create dns_svr.log. Provides time functions to concat and generate timestamps |
| modules/util.c | Provides a series of utility functions such as Concatenating/splitting N-bits and printing hex representation of binary data |
| images/ | Folder containing images used in README |
