/**
 @file     adf4351_sup.c
 @date     13 Oct 2016
 @brief    ADF4351 user interface

 @par Copyright &copy; 2016 Mike Gore, GPL License
 @par You are free to use this code under the terms of GPL
  please retain a copy of this notice in any code you use it in.

  This is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option)
  any later version.
  
  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "user_config.h"

#include <stdint.h>
#include <stdarg.h>
//#include <math.h>

#include "hspi.h"
#include "stringsup.h"

#include "adf4351.h"

#include "mathio.h"

/* define ADF4351 global data */
typedef struct {
	double low;
	double hi;
	double val;
	double last;
	double spacing;
	int scan;
} freq_t;

freq_t frequency = { 100e6, 100e6, 100e6, 0.0, 0.0, 0 };

// display frequency
MEMSPACE
void ADF4351_update(double freq)
{
    printf("%4.3f\n", freq/1000000.0);
    ADF4351_sync(1);
}

// update every 50 mS
MEMSPACE
void ADF4351_task()
{
    int status;
    double result;

    if(!frequency.scan)
        return;

    if (frequency.val >= frequency.hi || frequency.val <frequency.low)
        frequency.val = frequency.low;

    status = ADF4351_Config(frequency.val, 25000000.0, frequency.spacing, &result);
    if(status && status != ADF4351_RFout_MISMATCH)
    {
        ADF4351_display_error ( status );
        // stop scanning
        printf("adf4351 scan stop:  %e\n",frequency.val);
        frequency.scan = 0;
    }
    else
            ADF4351_update(result);

    frequency.last = frequency.val;

    frequency.val += frequency.spacing;
}
#ifdef ADF4351
// @brief  Help command
MEMSPACE
void adf4351_help()
{
    printf("adf4351 frequency [spacing]\n"
    "adf4351 scan low hi spacing\n"
    "adf4351 start\n"
    "adf4351 stop\n");
}
#endif

// @brief  Command line parser
///
/// @param[in] line - command line
/// @return  void
MEMSPACE
int adf4351_cmd(int argc,char *argv[])
{
    char *ptr;
    int len;
    int status;
    double result;
	char tmp[80];
	int ind;

    if(argc < 2)
        return(0);

    ind = 1;
    ptr = argv[ind++];

	printf("adf4351_cmd:[%s]\n", ptr);
	for(;ind<argc;++ind)
		printf("%s ", argv[ind++]);
	printf("\n");

	if(MATCHARGS(ptr, "help", (ind+0) ,argc))
	{
		adf4351_help();
		return(1);
	}

	if(MATCHARGS(ptr, "stop", (ind+0) ,argc))
	{
		printf("adf4351 scan stop:  %e\n",frequency.val);
		frequency.scan = 0;
		return(1);
	}

	if(MATCHARGS(ptr, "start", (ind+0) ,argc))
	{
		frequency.scan = 1;
		printf("adf4351 scan start: %e\n",frequency.val);
		return(1);
	}
	if(MATCHARGS(ptr, "scan", (ind+3) ,argc))
    {
		/* stop scanning while doing an update */
		frequency.scan = 0;

        frequency.low = atof(argv[ind++]);
        frequency.hi = atof(argv[ind++]);
        frequency.spacing = atof(argv[ind++]);

        frequency.val = frequency.low;
        printf("frequency low:      %10.2f\n",frequency.low);
        printf("frequency hi:       %10.2f\n",frequency.hi);
        printf("frequency spacing:  %10.2f\n",frequency.spacing);
        printf("adf4351 scan start: %10.2f\n",frequency.val);
		/* start scanning */
        frequency.scan = 1;
        return(1);
    }

	/* stop scanning - manual mode */
	frequency.scan = 0;

	// frequency
	if(ind+1 <= argc)
		frequency.val = atof(argv[ind++]);
	else
		return(0);

	printf("frequency: %.2f\n",frequency.val);

	// spacing - if omitted 1000 is used
	if(ind+1 <= argc)
		frequency.spacing = atof(argv[ind++]);
	else
		frequency.spacing = 1000.0;

	frequency.scan = 0;
	status = ADF4351_Config(frequency.val, 25000000.0, frequency.spacing, &result);
	printf("calculated: %.2f\n",result);

	if(status)
		ADF4351_display_error ( status );
	else
		ADF4351_sync(1);

	// FIXME we treat a mismatch as non-fatal
	// we should really be looking for an accuracy value
	if(status == ADF4351_RFout_MISMATCH)
	{
		ADF4351_sync(1);
	}

	return(1);
}


