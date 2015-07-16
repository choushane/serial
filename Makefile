
all: libals_uart.so libals_conf.so libals_sock.so libals_tcp_server.so UART con_test

#%.o: %.c
#	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
libals_uart.so: uart.c 
	$(CC) -shared -o $@ $^ $(LDFLAGS)

libals_conf.so: configure.c
	$(CC) -shared -o $@ $^ $(LDFLAGS)

libals_sock.so: socket.c
	$(CC) -shared -o $@ $^ $(LDFLAGS)

libals_tcp_server.so: tcp_server.c
	$(CC) -shared -o $@ $^ $(LDFLAGS)

UART: main.o tcp_client.o udp_mode.o time.o file.o commander.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS) -lpthread -L. -lals_uart -lals_conf -lals_sock -lals_tcp_server

con_test: con_test.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o con_test UART schedule $(all)
