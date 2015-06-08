CFLAGS=-D_GNU_SOURCE


FONTS_C = fixed/7x13.bdf \
	fixed/7x13B.bdf \
	fonts/C/24-Terminus.bdf \
	fonts/C/24-Terminus-Bold.bdf \
	fonts/C/32-Terminus.bdf \
	fonts/C/32-Terminus-Bold.bdf 

FONTS_P = fonts/P/12-Adobe-Helvetica.bdf \
	fonts/P/12-Adobe-Helvetica-Bold.bdf \
	fonts/P/21-Adobe-Helvetica.bdf \
	fonts/P/21-Adobe-Helvetica-Bold.bdf \
	fonts/P/35-Adobe-Helvetica.bdf \
	fonts/P/35-Adobe-Helvetica-Bold.bdf

    
all:	bdffont2c copyfont bigF 

install:	all 
	cp -p bdffont2c /usr/local/bin

copyfont: bdffont2c
	cp -p font.h ../display

bdffont2c.o:	bdffont2c.c
	$(CC) $(CFLAGS) -g -c bdffont2c.c 

bdffontutil.o:	bdffontutil.c
	$(CC) $(CFLAGS) -g -c bdffontutil.c 

bdffont2c: bdffont2c.o bdffontutil.o
	$(CC) $(CFLAGS) -g -o bdffont2c bdffont2c.o bdffontutil.o

# Fixed font set
bigF:	bdffont2c 
	-rm fonts.h
	./bdffont2c -f -p 2 -l 32 -u 127 -o fonts.h $(FONTS_C)
	$(CC) $(CFLAGS) -g -o bdfview bdfview.c bdffontutil.o
	cp -f fonts.h ../display
	./bdfview -n 5 -p 2 -t 

# Proprtional font set
bigP:	bdffont2c 
	-rm fonts.h
	./bdffont2c -p 2 -l 32 -u 127 -o fonts.h $(FONTS_P)
	$(CC) $(CFLAGS) -g -o bdfview bdfview.c bdffontutil.o
	cp -f fonts.h ../display
	./bdfview -n 5 -p 2 -t 

bigS:	bdffont2c 
	-rm fonts.h
	./bdffont2c -s -p 2 -l 32 -u 127 -o fonts.h $(FONTS_C)
	$(CC) $(CFLAGS) -g -o bdfview bdfview.c bdffontutil.o
	cp -f fonts.h ../display
	./bdfview -n 5 -p 2 -t 

testP:    bdffont2c 
	-rm fonts.h
	./bdffont2c -o fonts.h -l 32 -u 127 fonts/P/10-Adobe*.bdf 
	$(CC) $(CFLAGS) -g -o bdfview bdfview.c bdffontutil.o
	./bdfview -p 2 -t 

testF:    bdffont2c 
	-rm fonts.h
	./bdffont2c -p 1 -f -o fonts.h -l 32 -u 127 fonts/P/10-Adobe*.bdf 
	$(CC) $(CFLAGS) -g -o bdfview bdfview.c bdffontutil.o
	#./bdfview -p 2 -t 

testS:    bdffont2c 
	-rm fonts.h
	./bdffont2c -s -p 2 -o fonts.h -l 32 -u 127 fonts/P/10-Adobe*.bdf
	$(CC) $(CFLAGS) -g -o bdfview bdfview.c bdffontutil.o
	./bdfview -p 2 -t 
	
clean:
	rm -f bdffont2c *\.o fonts.h bdfview


