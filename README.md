\mainpage index page
\section README
 * Please use this link: https://rawgit.com/magore/esp8266_ili9341/master/doxygen/html/index.html

 @par The sample ESP8266 with ili9341 display project 
 See [COPYRIGHT.md](@ref COPYRIGHT.md) for a full copyrrite notice for the project

This project uses a ESP8266 to drive an ili9341 display
I wrote the following support functions
  CORDIC C table generator and 3D transformation code support functions
  Wireframe viewer and C generator for Earth coastline data
  Small Print with full floating point support - optional
  BDF font to C conversion code and optimized display code
  ILI9341 display driver code 

I Updated:
  HSPI code that correctly handles memory size and agligment 

Two demos are provided
 * CUBE demo 
 * Wireframe Earth viewer - still neads hiden line removal

___

@par Documentation
  * https://rawgit.com/magore/esp8266_ili9341/master/doxygen/html/index.html


___

@par Credits
  * Built using ESP Open SDK - or esp8266-devkit by CERTS
    @see https://github.com/pfalcon/esp-open-sdk
    @see https://github.com/CHERTS/esp8266-devkit
  * Original demo code from CHERTS and Sem (mostly rewritten by me)
    @see https://github.com/CHERTS/esp8266-devkit/tree/master/Espressif/examples/esp8266_ili9341
  * Optimized Line drawing function from CHERTS 
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
    * term115200 - shell script to launch terminal 1t 115200 baud

  * cube 
    * cube.c
    * cube.h
      * Demo by Sem - using my updated fully working CORDIC routines 
      * Orginal code do not work correctly above/below +/-90 degrees

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
      * My mostly rewritten ili9341 display driver code
        * Optimized line drawing function is from CERTS
        * Non optimized verion is also supplied that I wrote in 1984
    * util.c
    * util.h
      * Flash reading and bittest functions for system requiring specific alignment and access size of flash memory.
      

  * driver 
    * cal_dex.c       
      * Debug exception support by cal (20150421, cal)
    * hspi.c              
    * hspi.h              
      * My rewritten HPSI code that avoids unaligned read and writes
      * Origonal Code from CERTS and Sem
    * ili9341_adafruit.c  
    * ili9341_adafruit.h
      * Adafruit display - just those functions that I have not rewritten

   * earth   
     * Earth coast line data and display code
     * Still need to add hidden vector removal
       * 00README.txt
         * Article on coordinate transforms from stackoverflow by Daphna Shezaf
       * earth.c
         * wireframe viewer with earth coastlines
       * earth.h
       * earth_inc.h
         * Earth wireframe coastline data as C structure
       * make_wireframe
         * earth2wireframe.c
           * Create C structure wireframe coastline data
           * This code could be easily adapted for any kind of wireframe
         * Makefile
           * Create earth_inc.h from Coastline LAt/LONG pairs
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

   * printf  
     * My small printf with floating support, misc I/O functions
        * Makefile       
          * Build and test printf
        * pr.c           
          * Printf interface to display library tft_printf()
        * printf.c       
        * printf.h       
          * My printf code

   * user    
     * Main user demo task

