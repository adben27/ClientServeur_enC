CC = gcc
CFLAGS = -g -Wall -Iinclude
LDFLAGS = -pthread

all: bin wcp_clt wcp_srv

wcp_srv: bin/wcp_srv.o bin/comptine_utils.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

wcp_clt: bin/wcp_clt.o bin/comptine_utils.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

bin/comptine_utils.o: src/comptine_utils.c include/comptine_utils.h
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

bin/wcp_clt.o: src/wcp_clt.c include/comptine_utils.h
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

bin/wcp_srv.o: src/wcp_srv.c include/comptine_utils.h
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)
bin :
	mkdir $@

clean:
	rm -rf bin wcp_srv wcp_clt
