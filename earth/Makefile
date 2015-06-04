all:	earthdata

# Earth coastline lat/long data
DATA = data/world.dat
#DATA = data/world_110m.txt

earthdata:	earth2wireframe
	./earth2wireframe -f $(DATA) -o ../wire/earth_data.h

earth2wireframe:	earth2wireframe.c
	gcc earth2wireframe.c -o earth2wireframe -lm

clean:
	rm earth2wireframe 

  
