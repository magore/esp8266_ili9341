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
void ADF4351_update(double freq)
{
    printf("%4.3f\n", freq/1000000.0);
    ADF4351_sync(1);
}

// update every 50 mS
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
int adf4351_cmd(char *line)
{
    char *ptr;
    int len;
    int status;
    double result;
	char tmp[80];

	ptr = skipspaces(line);

	printf("adf4351_cmd:[%s]\n", ptr);

	if((len = token(ptr,"help")) )
	{
		adf4351_help();
		return(1);
	}

	else if((len = token(ptr,"stop")) )
	{
		printf("adf4351 scan stop:  %e\n",frequency.val);
		frequency.scan = 0;
		return(1);
	}

	else if((len = token(ptr,"start")) )
	{
		frequency.scan = 1;
		printf("adf4351 scan start: %e\n",frequency.val);
		return(1);
	}
    else if ((len = token(ptr,"scan")) )
    {
		/* stop scanning while doing an update */
		frequency.scan = 0;

        ptr += len;

        ptr = get_token(ptr, tmp, 79);
        frequency.low = atof(tmp);

        ptr = get_token(ptr, tmp, 79);
        frequency.hi = atof(tmp);

        ptr = get_token(ptr, tmp, 79);
        frequency.spacing = atof(tmp);

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
	ptr = get_token(ptr, tmp, 79);
	frequency.val = atof(tmp);
	printf("frequency: %.2f\n",frequency.val);

	// spacing - if omitted 1000 is used
	ptr = get_token(ptr, tmp, 79);
	if(*tmp)
		frequency.spacing = atof(tmp);
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


