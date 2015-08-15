all:	test

test:	printf
	./test_printf | more

# Create a stand alone test program called printf
printf:	printf.c
	gcc -DPRINTF_TEST test_printf.c sup.c printf.c -o test_printf -lm

clean:
	rm test_printf

  
