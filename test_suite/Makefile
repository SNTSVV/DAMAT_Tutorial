LIBRARIES=-lsocketcan -lzmq -lpthread
UTILS=utils/csp_server_client_posix.c
LIBCSP=../libcsp/build/libcsp.a
CSP_INCLUDE=../libcsp/include
CSP_AUTOCONFIG=../libcsp/build/
# HEADERS=utils/headers

all: test_01 test_02 test_03 test_04 test_05

test_01: test_01.o
	./test_01/test_01.o
test_01.o:
	$(CC) test_01/test_01.c $(UTILS) $(LIBCSP) $(LIBRARIES) -I$(CSP_AUTOCONFIG) -I$(CSP_INCLUDE) -o test_01/test_01.o

test_02: test_02.o
	./test_02/test_02.o
test_02.o:
	$(CC) test_02/test_02.c $(UTILS) $(LIBCSP) $(LIBRARIES) -I$(CSP_AUTOCONFIG) -I$(CSP_INCLUDE) -o test_02/test_02.o

test_03: test_03.o
	./test_03/test_03.o
test_03.o:
	$(CC) test_03/test_03.c $(UTILS) $(LIBCSP) $(LIBRARIES) -I$(CSP_AUTOCONFIG) -I$(CSP_INCLUDE) -o test_03/test_03.o

test_04: test_04.o
	./test_04/test_04.o
test_04.o:
	$(CC) test_04/test_04.c $(UTILS) $(LIBCSP) $(LIBRARIES) -I$(CSP_AUTOCONFIG) -I$(CSP_INCLUDE) -o test_04/test_04.o

test_05: test_05.o
	./test_05/test_05.o
test_05.o:
	$(CC) test_05/test_05.c $(UTILS) $(LIBCSP) $(LIBRARIES) -I$(CSP_AUTOCONFIG) -I$(CSP_INCLUDE) -o test_05/test_05.o

.PHONY: clean
clean:
	rm -f test_01/test_01.o
	rm -f test_02/test_02.o
	rm -f test_03/test_03.o
	rm -f test_04/test_04.o
	rm -f test_05/test_05.o
