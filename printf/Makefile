all:	test_printf

test:	test_printf
	./test_printf | more

# Create a stand alone test program called printf
test_printf:	printf.c test_printf.c
	gcc -DPRINTF_TEST -DFLOAT test_printf.c printf.c -o test_printf -lm


clean:
	rm -f test_printf

  
