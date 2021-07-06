CFLAGS= -Wall -Wpedantic -g

dns_svr: main.o config.o util.o dns.model.o socket.o log.o cache.o
	gcc $(CFLAGS) -o dns_svr main.o config.o util.o dns.model.o socket.o log.o cache.o -lm

config.o: modules/config.c headers/config.h
	gcc $(CFLAGS) -c modules/config.c 

util.o: modules/util.c headers/util.h
	gcc $(CFLAGS) -c modules/util.c 

dns.model.o: modules/dns.model.c headers/dns.model.h
	gcc $(CFLAGS) -c modules/dns.model.c 

socket.o: modules/socket.c headers/socket.h
	gcc $(CFLAGS) -c modules/socket.c 

log.o: modules/log.c headers/log.h
	gcc $(CFLAGS) -c modules/log.c 

cache.o: modules/cache.c headers/cache.h
	gcc $(CFLAGS) -c modules/cache.c

main.o: main.c 
	gcc $(CFLAGS) -c main.c

.PHONY: clean

clean:
	rm -f *.o
	rm -f dns_svr
	rm -f *.log