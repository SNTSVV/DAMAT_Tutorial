LIBRARIES=-lsocketcan -lzmq -lpthread
UTILS=utils/csp_server_client_posix.c
LIBCSP=../libcsp/build/libcsp.a
CSP_INCLUDE=../libcsp/include
CSP_AUTOCONFIG=../libcsp/build/

test_01: test_01.o
	./test_01/test_01.o
test_01.o:
	$(CC) test_01/test_01.c $(UTILS) $(LIBCSP) $(LIBRARIES) -I$(CSP_AUTOCONFIG) -I$(CSP_INCLUDE) -o test_01/test_01.o

test_02: test_02.o
	./test_02/test_02.o
test_02.o:
	$(CC) test_02/test_02.c $(UTILS) $(LIBCSP) $(LIBRARIES) -I$(CSP_AUTOCONFIG) -I$(CSP_INCLUDE) -o test_02/test_02.o

.PHONY: clean
clean:
	rm test_01/test_01.o
	rm test_02/test_02.o
