all:	test_printf n2a

test:	test_printf
	./test_printf

CFLAGS = -DPRINTF_TEST -DFLOATIO -g

# Create a stand alone test program called printf
test_printf:	printf.c mathio.c test_printf.c
	gcc $(CFLAGS) test_printf.c printf.c mathio.c -o test_printf -lm

n2a:	printf.c mathio.c n2a.c
	gcc $(CFLAGS) n2a.c printf.c mathio.c -o n2a -lm
	

clean:
	-rm -f test_printf n2a

  
