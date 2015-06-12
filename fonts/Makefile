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

    
all:	bdffont2c bigS copyfonts cleanfont

install:	all 
	cp -p bdffont2c /usr/local/bin

.PHONY:	copyfonts
copyfonts: 
	cp -p font.h fonts.h ../display

bdffont2c.o:	bdffont2c.c
	$(CC) $(CFLAGS) -g -c bdffont2c.c 

bdffontutil.o:	bdffontutil.c
	$(CC) $(CFLAGS) -g -c bdffontutil.c 

bdffont2c: bdffont2c.o bdffontutil.o
	$(CC) $(CFLAGS) -g -o bdffont2c bdffont2c.o bdffontutil.o

bdfview:	bdffontutil.o fonts.h
	$(CC) $(CFLAGS) -g -o bdfview bdfview.c bdffontutil.o

# Fixed font set 
big:	cleanfont bdffont2c 
	./bdffont2c -p 2 -l 32 -u 127 -o fonts.h $(FONTS_C)
	$(CC) $(CFLAGS) -g -o bdfview bdfview.c bdffontutil.o
	./bdfview -n 5 -p 2 -t 

# Fixed font set force to fixed mode
bigF:	cleanfont bdffont2c 
	./bdffont2c -f -p 2 -l 32 -u 127 -o fonts.h $(FONTS_C)
	$(CC) $(CFLAGS) -g -o bdfview bdfview.c bdffontutil.o
	./bdfview -n 5 -p 2 -t 

# Proprtional font set
bigP:	cleanfont bdffont2c 
	./bdffont2c -p 3 -l 32 -u 127 -o fonts.h $(FONTS_P)
	$(CC) $(CFLAGS) -g -o bdfview bdfview.c bdffontutil.o
	./bdfview -n 5 -p 3 -t 

bigS:	cleanfont bdffont2c 
	./bdffont2c -s -p 2 -l 32 -u 127 -o fonts.h $(FONTS_C)
	$(CC) $(CFLAGS) -g -o bdfview bdfview.c bdffontutil.o
	./bdfview -n 5 -p 3 -t 

testP:    cleanfont bdffont2c 
	./bdffont2c -o fonts.h -l 32 -u 127 fonts/P/10-Adobe*.bdf 
	$(CC) $(CFLAGS) -g -o bdfview bdfview.c bdffontutil.o
	./bdfview -p 2 -t 

testF:    cleanfont bdffont2c 
	./bdffont2c -p 1 -f -o fonts.h -l 32 -u 127 fonts/P/10-Adobe*.bdf 
	$(CC) $(CFLAGS) -g -o bdfview bdfview.c bdffontutil.o
	#./bdfview -p 2 -t 

testS:    cleanfont bdffont2c 
	./bdffont2c -s -p 2 -o fonts.h -l 32 -u 127 fonts/P/10-Adobe*.bdf
	$(CC) $(CFLAGS) -g -o bdfview bdfview.c bdffontutil.o
	./bdfview -p 2 -t 
	
cleanfont:
	rm -f fonts.h 

clean:
	rm -f bdffont2c *\.o fonts.h bdfview

