
all: UART con_test

#%.o: %.c
#	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

UART: main.o tcp_client.o socket.o uart.o configure.o tcp_server.o udp_mode.o time.o file.o commander.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS) -lpthread

con_test: con_test.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o con_test UART schedule $(all)
