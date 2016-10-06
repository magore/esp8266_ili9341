\mainpage index page
\section README

@par Documentation
  * Pleasse use this link for Documentation and this README
   @see https://rawgit.com/magore/esp8266_ili9341/master/doxygen/html/index.html
@par Copyright
  * See [COPYRIGHT.md](COPYRIGHT.md) for a full copyright notice for the project
@par Description
  * ESP8266 support libraries
    * All of the code in this project, except where noted is written by me
    * Analog Devices ADF4351 Wideband Synthesizer driver and test code
      * Control is via serial terminal that works while the other demos run
      * adf4351 123456000 10000
        * frequency, and channel spacing used in calculations
      * adf4351_scan 144e6 145e6 10000
        * start frequency, end frequency, channel spacing
    * ILI9341 display driver 
      * Mutiple custom size window support
      * Window scrolling 
      * readPixel() function suported
      * The specific display this was tested on is a TM022HDH26 display I got from ebay.
      * Not limited by ILI9341 hardware scroll restrictions.
   * BDF compiler and fonts - creates C code used in the display drivers
   * Wireframe Viewer 
     * Code generator for Earth coastline data
     * I created two wireframe demos 
       * Cube wireframe dataset
       * Earth coastline dataset - wireframe view still needs hidden line removal option
   * CORDIC C code generator and 3D transformation code support functions use by wireframe viewer code
   * Small PRINTF with full floating point support - much smaller then GNU full verson along 
   * ATOF and various integer conversions
   * WEB server using SD CARD with CGI processing - files and CGI results can be ANY SIZE!
    * Example web site for testing
    * WEB server can update TFT display
      * Simple door sign status update using web page - see html/msg.cgi and web/web.c for code
   * Network server client example for display updates
   * Uart network server client for serial uart to network bridge.
   * Generic queue handling code
   * Uart driver.
   * POSIX wrappers for FatFS - provides unix file I/O operations
    * Includes FatFS SD card support by ChaN
   * POSIX time functions and RTC set with NTP
   * Multiple timers used by rtc and time functions
   * HSPI code that can handle multiple devices each with varying clock frequencies

@par Credits
  * Project compiles using ESP Open SDK - or esp8266-devkit by CHERTS
    @see https://github.com/pfalcon/esp-open-sdk
    @see https://github.com/CHERTS/esp8266-devkit
  * Display drivers from third parties
    A few of the display driver functions are from the Adafruit GFX Library
      @see https://github.com/adafruit/Adafruit-GFX-Library
    Optimized Line drawing function and Makefile from CHERTS 
      @see https://github.com/CHERTS/esp8266-devkit/tree/master/Espressif/examples/esp8266_ili9341
  * Font sources:
    @see https://www.gnu.org/software/freefont/
    @see https://www.rockbox.org
    @see http://www.cl.cam.ac.uk/~mgk25/ucs-fonts.html
    @see http://en.wikipedia.org/wiki/Glyph_Bitmap_Distribution_Format
    @see https://partners.adobe.com/public/developer/en/font/5005.BDF_Spec.pdf
  * FatFS code by CHaN:
    @see http://elm-chan.org/fsw/ff/00index_e.html
  * Yield Code extracted from from ESP8266 Arduino Project
	@see https://github.com/esp8266/Arduino
___

@par Directories
  * Files
    * Makefile from CHERTS modified for the project
      * Features controlled by variables in Makefile
    * README.md - Project readme file
    * COPYRIGHT.md - Project Copyright file
    * Doxyfile - Doxygen configuration file
    * get_esp-open-sdk - shell script to download an install ESP OPEN SDK on Ubuntu 14.04LTS
      * Tested with esp_iot_sdk_v1.5.2 as of 21 Jun 2016
    * term115200 - shell script to launch terminal to 115200 baud

  * Eclipse Project files and Directories
    * Change .settings/language.settings.xml
    * .settings
       * .settings/language.settings.xml
         * Contains path to cross compiler for indexing - matches assumptions in SDK installer script: get_esp-open-sdk
    * .cproject
    * .project

  * Directories
    * adf4351
      * Analog Devices ADF4351 Wideband Synthesizer with Integrated VCO driver
        * adf4351.c
        * adf4351.h
      * Hardware Abstarction layer
      * adf4351_hal.c
      * adf4351_hal.h
    * bridge
      * bridge.c
      * bridge.h
        * Serial bridge code - send and receive serial data via network
        * Opens a port on port 23 so you can use telnet to test
          * Note: at the moment no telnet command processing is done.
    * cordic
      * CORDIC - very fast 2D/3D trig conversion
      * cordic.c        
      * cordic.h        
        * My 3D transformation functions are based on CORDIC and gradians/100
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
        * My mostly rewritten ili9341 display driver code handles multiple windows
          * Depends on a few modified Adifruit functions under directory driver
      * font.c        
      * font.h        
        * My Fixed and proportional font display code 
      * fonts.h        
        * Fonts data structures converted BDF fonts
      * ili9341.c           
      * ili9341.h           
          * multiple window support 
          * readPixel() works on most all 4 wire displays now
          * scrolling window support
          * Optimized line drawing function is from CHERTS
            * Non optimized version is also supplied that I wrote in 1984
        * ili9341_hal.c           
        * ili9341_hal.h           
          * DIsplay hardware abstraction layer
  	* tft_printf.c
  	* tft_printf.h
  	  * Printf interface to display library tft_printf()
  
    * docs 
      * ili9431 and esp8266 related documents
  
    * driver - only third party code
      * cal_dex.c       
        * Debug exception support by cal (20150421, cal)
      * ili9341_adafruit.c  
      * ili9341_adafruit.h
        * Adafruit display - just those remaining functions that I have not rewritten
  
    * earth   
      * Earth coast line data and display code
      * Still need to add hidden vector removal
        * 00README.txt
           * Article on coordinate transforms from stackoverflow by Daphna Shezaf
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
  
    * fatfs
      * FatFs core is by (C)ChaN, 2013
        * disk.c
        * disk.h
        * diskio.h
        * ff.c
        * ffconf.h
        * ff.h
        * integer.h
        * mmc.c
        * mmc.h
        * option
        * syscall.c
        * unicode.c
     * fatfs SD card HAL and POSIX wrappers - provides unix file I/O operations
        * modified to use hardware abstraction
        * fatfs_utils.c
        * fatfs_utils.h
          * user tests and commands
           * fatfs_alloc_finfo fatfs_cat fatfs_cd fatfs_copy fatfs_create fatfs_filinfo_list 
           * fatfs_help fatfs_ls fatfs_mkdir fatfs_pwd fatfs_rename fatfs_rm fatfs_rmdir fatfs_stat fatfs_status fatfs_tests
        * mmc_hal.c
        * mmc_hal.h
          * MMC Hardware abstraction layer
     * My POSIX wrappers for fatfs
        * posix.c - provides a POSIX interface for FatFS - unix file I/O wrappers 
        * posix.h
  
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
        * freefont
          * Freefont fonts  www.gnu.org/software/freefont
  
    * html
      * A working web site with files that can be put on an SD card
        * Used for testing web.c
  
    * include 
       * user_config.h
         * Master include and config header
       * spi_register.h 
         * From Espressif
       * uart_register.h
         * From Espressif
  
    * lib
      * time, rtc and timer functions
        * time.h
        * time.c - POSIX time functions
        * timer.h
        * timer.c - simple timers (only intended for quick functions)
        * timer_hal.h
        * timer_hal.c - timer hardware abstraction layer
      * implementation of some posix stdlib functions
        * std.c 
        * std.h 
      * implementation of some posix ctype, string and strings functions
        * str.c    
        * str.h
      * Generic ring buffer support code
        * queue.c
        * queue.h
  
    * network
      * Simple network server - displays message sent by send.c 
        * network.c
        * network.h
  
    * printf  
      * My small printf with floating support, misc I/O functions
        * printf.c       
        * printf.h       
      * Build and test printf
        * Makefile       
      * Test functions for printf.c that can be run under linux to verify results
        * test_printf.c
  
    * scanf
      * Simple scanf
        * scanf.c
  
    * send.c
      * Send message to network server 
      * Example: ./send -i 192.168.200.116 -m '\fscrolling\ntext\n1\n2\n3\n4'
        * These escape characters are processed on the display: \n, \t, \f
  
    * user    
     * Main user demo task
       * user_main.c
     * Intializes ESP8266 and sets up demo with 4 active windows with independent attributes
  
    * utils
     * Debugging printf wrapper
       * debug.c              
       * debug.h              
     * My rewritten HPSI code that avoids unaligned read and writes (based on code ideas from CHERTS and Sem)
       * hspi.c              
       * hspi.h              
     * Serial I/O Interrup driven receive and transmit code
       * uart.c
       * uart.h
       * uart_register.h
     * Flash read, Bittest functions, For systems requiring specific memory alignment access methods
       * flash.c
       * flash.h
     * POSIX malloc,calloc and free wrappers
       * sys.c
       * sys.h
  
    * web
     * Web server with FAT filesystem SD CARD support 
       * web.c
       * web.h
     * Features
  	 * Served files can be ANY SIZE!
       * CGI files can have an extension of: html,htm,text,txt,cgi
  	   * CGI results can be ANY SIZE
  	 * Only tokens of the form @_ token _@ are replaced by the rewrite function
         * @see rewrite_cgi_token() in web.c
  	 * Uses yield function to continue background tasks while serving requests
     * Applications
       * I created a door sign status display that can be updated via a web page web page running on the esp8266
         * Copy the file html/msg.cgi to the root folder of a fat32 SD card and modify to suit your needs.
         * Use: open a web browser to the web server runing on the esp8266
           * See the Video
           * For example: http://192.168.200.116/msg.cgi
           * You can update status and return time information on the TFT display by entering information on the page.
    * wire
      * Wireframe viewer code - uses CORDIC
        * wire.c
        * wire.h
      * Wireframe CUBE data
        * cube_data.h    
      * Wireframe EARTH data
        * earth_data.h
  
    * yield
      * Yield code from Arduino yield code
      * README.txt     
      * cont.S         
      * cont.h         
      * cont_util.c    
        * Context switch code
      * ets_sys.h
      * Main user_init and task replacement with yield support
        * user_task.c    
        * user_task.h
  

@par Demo Video
  * See web.c for details.
    * I created a door status display that can be updated via a web page web page 
    * I did a status update while recording the video
    * Result: https://github.com/magore/esp8266_ili9341/blob/master/video.mp4?raw=true

@par Demo Images
  * Running demo and sending a message to the network window
  * "./send -i 192.168.200.116 -m 'testing\nTest3\nscrolling\ntext\n1\n2'"
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
