See: http://stackoverflow.com/questions/1185408/converting-from-longitude-latitude-to-cartesian-coordinates

Here's the answer I found:

Just to make the definition complete, in the Cartesian coordinate system:

    the x-axis goes through long,lat (0,0), so longitude 0 meets the equator;
    the y-axis goes through (0,90);
    and the z-axis goes through the poles.

The conversion is:

x = R * cos(lat) * cos(lon)
y = R * cos(lat) * sin(lon)
z = R *sin(lat)

Where R is the approximate radius of earth (e.g. 6371KM).

If your trigonometric functions expect radians (which they probably do), you will need to convert your longitude and latitude to radians first. You obviously need a decimal representation, not degrees\minutes\seconds (see e.g. here about conversion).

The formula for back conversion:

lat = asin(z / R)
lon = atan2(y, x)

asin is of course arc sine. read about atan2 in wikipedia. Dont forget to convert back from radians to degrees. 
