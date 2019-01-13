randmain: randmain.o randcpuid.o
	$(CC) $(CFLAGS) -ldl -Wl,-rpath=$(PWD) randmain.o randcpuid.o -o $@

randlibhw.so: randlibhw.c
	$(CC) $(CFLAGS) -fPIC -shared randlibhw.c -o $@

randlibsw.so: randlibsw.c
	$(CC) $(CFLAGS) -fPIC -shared randlibsw.c -o $@
