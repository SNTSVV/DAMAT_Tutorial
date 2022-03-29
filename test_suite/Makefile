LIBRARIES=-lsocketcan -lzmq -lpthread
# UTILS=utils/csp_server_client.c utils/csp_server_client_posix.c
UTILS=utils/csp_server_client_posix.c
LIBCSP=/home/vagrant/libcsp_workspace/libcsp/build/libcsp.a
CSP_INCLUDE=-Iinclude

test_01: test_01.o
	./test_01/test_01.o
test_01.o:
	$(CC) test_01/test_01.c $(UTILS) $(LIBCSP) $(LIBRARIES) $(CSP_INCLUDE) -o test_01/test_01.o
.PHONY: clean
clean:
	rm test_01/test_01.o
