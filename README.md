\mainpage index page
\section README

@par Documentation
 
 * Pleasse use this link for Documentation and this README
   * https://rawgit.com/magore/esp8266_ili9341/master/doxygen/html/index.html

@par Copyright
 * See [COPYRIGHT.md](COPYRIGHT.md) for a full copyright notice for the project

@par Description
 * ESP8266 ILI9341 display demo with multiple window support and simple networking
 * I wrote the following support functions from scratch
 * Font compiler and display code
  * CORDIC C table generator and 3D transformation code support functions
  * Wire frame viewer and C generator for Earth coastline data
  * Small Print with full floating point support along with ftoa/atof and integer conversions
  * BDF fonts and a BDF font to C conversion code and optimized display code
  * Network server client example
  * ILI9341 display driver with multiple custom sized window support 
  * readPixel() and custom sized window scrolling 
     * Not limited by ILI9341 hardware scroll restrictions.
   
 * Updated:
  * I rewrote most of Adafruit code ili9341 graphics library
  * HSPI code now correctly handles memory size and alignment 

 * I created a wireframe viewed and two wireframe demos 
  * Cube wireframe dataset
  * Earth coastline dataset - wireframe view still needs hidden line removal option
 
___

@par Credits
  * Built using ESP Open SDK - or esp8266-devkit by CHERTS
    @see https://github.com/pfalcon/esp-open-sdk
    @see https://github.com/CHERTS/esp8266-devkit
  * Original demo code from CHERTS and Sem (mostly rewritten by me)
    @see https://github.com/CHERTS/esp8266-devkit/tree/master/Espressif/examples/esp8266_ili9341
  * Optimized Line drawing function and Makefile from CHERTS 
    @see https://github.com/CHERTS/esp8266-devkit/tree/master/Espressif/examples/esp8266_ili9341
  * Fonts:
    @see https://www.rockbox.org
    @see http://www.cl.cam.ac.uk/~mgk25/ucs-fonts.html
    @see http://en.wikipedia.org/wiki/Glyph_Bitmap_Distribution_Format
    @see https://partners.adobe.com/public/developer/en/font/5005.BDF_Spec.pdf
___

@par Directories
  * .
    * Makefile from CHERTS modified for the project
    * README.md - Project readme file
    * COPYRIGHT.md - Project Copyright file
    * Doxyfile - Doxygen configuration file
    * get_esp-open-sdk - shell script to download an install ESP OPEN SDK on Ubuntu 14.04LTS
    * term115200 - shell script to launch terminal to 115200 baud

  * wire
    * wire.c
    * wire.h
      * Wireframe viewer code - uses CORDIC
    * cube_data.h    
      * Wireframe CUBE data
    * earth_data.h
      * Wireframe EARTH data

  * docs 
    * ili9431 and esp8266 related documents

  * cordic
    * cordic.c        
      * My 3D transformation functions based on CORDIC and gradians/100
      * 1 = 90degrees so the integer part reflects the quadrant
    * cordic2c_inc.h  
      * CORDIC lookup tables generated with my program cordic2c
      * 1 = 90degrees so the integer part reflects the quadrant
    * make_cordic
      * cordic2c.c
        * My customized CORDIC code, CORDIC table generating tools
        * Creates fixed point tables where 1 = 90degrees 
          * The integer part reflects the quadrant
        * Based on work by P. Knoppers, 13-Apr-1992.
      * Makefile
        * Make and test CORDIC tables

  * display
    * font.c        
    * font.h        
      * My Fixed and proportional font display code 
    * fonts.h        
      * Fonts data structures converted BDF fonts
    * ili9341.c           
    * ili9341.h           
      * My mostly rewritten ili9341 display driver code handles multiple windows
        * Optimized line drawing function is from CHERTS
        * Non optimized version is also supplied that I wrote in 1984
        * multiple window support 
        * readPixel() works on most all 4 wire displays now
        * scrolling window support
    * util.c
    * util.h
      * Flash reading and bittest functions for system requiring specific alignment and access size of flash memory.
      

  * driver 
    * cal_dex.c       
      * Debug exception support by cal (20150421, cal)
    * hspi.c              
    * hspi.h              
      * My rewritten HPSI code that avoids unaligned read and writes
      * Origonal Code from CHERTS and Sem
    * ili9341_adafruit.c  
    * ili9341_adafruit.h
      * Adafruit display - just those functions that I have not rewritten

   * earth   
     * Earth coast line data and display code
     * Still need to add hidden vector removal
       * 00README.txt
         * Article on coordinate transforms from stackoverflow by Daphna Shezaf
       * make_wireframe
         * earth2wireframe.c
           * Create C structure wire-frame coastline data
           * This code could be easily adapted for any kind of wire-frame
         * Makefile
           * earth_data.h from Coastline LAt/LONG pairs
         * data
           * world.dat      
           * world_10m.txt  
           * world_110m.txt 
           * world_50m.txt
             * Coastline data at various resolutions

   * fonts 
     * BDF Font conversion code
       * bdffont2c.c  
         * Convert BDF fonts to C structures main program
       * bdffontutil.c  
       * bdffontutil.h  
         * BDF fonts to C support code
       * bdfview.c  
         * ASCII previewer to test BDF display code 
       * font.h  
         * Font data structure definitions
       * fonts.h  
         * Converted fonts as C structures
       * Makefile
         * Make file to build selected fonts into a C structure
       * docs           
         * BDF font specs
       * fixed          
         * ucs-fonts.tar.gz
         * Unicode versions of the X11 "misc-fixed-*" fonts
         * Markus Kuhn, etc
       * fonts
         * Rockbox Font Collection - www.rockbox.org
         * Reorganized into three directories
           * C - constant size fonts
           * P - Proportional size fonts
           * O - Other

   * include 
     * misc esp8266 include files

   * network
     * Simple network server - displays message from send.c
      * network.c

   * printf  
     * My small printf with floating support, misc I/O functions
        * Makefile       
          * Build and test printf
        * pr.c           
          * Printf interface to display library tft_printf()
        * printf.c       
        * printf.h       
          * My printf code

   * send.c
     * Send message to network server 
     * Example: ./send -i 192.168.200.116 -m '\fscrolling\ntext\n1\n2\n3\n4'
        * These escape characters are processed on the display: \n, \t, \f
  * user    
     * Main user demo task
     * user_main.c
        * Intializes ESP8266 and sets up demo with 4 active windows with independent attributes

@par Demo Images
   * Running demo and sending a message to the network window
   * ./send -i 192.168.200.116 -m 'testing\nTest3\nscrolling\ntext\n1\n2'
     * Diagnostics from send
     ip:192.168.200.116, port:31415, message:
     testing
     Test3
     scrolling
     text
     1
     2
     Host name: 192.168.200.116
     * Result: ![](https://github.com/magore/esp8266_ili9341/blob/master/display.jpg)
