all:	test_printf

test:	test_printf
	./test_printf | more

CFLAGS = -DPRINTF_TEST -DFLOATIO -g

# Create a stand alone test program called printf
test_printf:	printf.c mathio.c test_printf.c
	gcc $(CFLAGS) test_printf.c printf.c mathio.c -o test_printf -lm

clean:
	-rm -f test_printf

  
