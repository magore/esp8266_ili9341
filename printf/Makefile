all:	test

test:	printf
	./test_printf

# Create a stand alone test program called printf
printf:	printf.c
	gcc -DTEST printf.c -o test_printf -lm

clean:
	rm test_printf

  
