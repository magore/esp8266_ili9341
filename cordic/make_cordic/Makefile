all: cordic2c table test

test:	cordic2c table
	# Create a stand alone test program called cordic2c
	gcc -DTEST -g ../cordic.c -o test_cordic -lm
	./test_cordic

table:	cordic2c
	./cordic2c -o ../cordic2c_inc.h

cordic2c:	cordic2c.c
	gcc -g cordic2c.c -o cordic2c -lm

clean:
	-rm -f cordic2c test_cordic

  
