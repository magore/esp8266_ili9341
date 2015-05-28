all:	test

test:	cordic2c
	./cordic2c >../cordic2c_inc.h
	# Create a stand alone test program called cordic2c
	gcc -DTEST ../cordic.c -o test_cordic -lm
	./test_cordic

cordic2c:	cordic2c.c
	gcc -DTEST cordic2c.c -o cordic2c -lm

clean:
	rm cordic2c test_cordic

  
