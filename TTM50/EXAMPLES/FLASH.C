/*
 *	flash.c
 *
 *	Transtech SCSI TRAM
 *
 *	Flash EPROM programming utility
 *
 *	Copyright (c) 1993 Transtech Parallel Systems Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <misc.h>
#include <process.h>

/* Debug macros */
#ifdef DEBUG
#define INFO( x )	printf x
#define VERBOSE( x )	(void) 0
#else
#define INFO( x )	(void) ((verbosity > 0) ? printf x : 0 )
#define VERBOSE( x )	(void) ((verbosity > 1) ? printf x : 0 )
#endif

#define TRUE			1
#define FALSE			0
#define OK			0
#define NOT_OK			-1

/* Flash EPROM base addres and size in words */
#define FLASH_BASE_ADDRESS	0x40000000
#define FLASH_WORD_SIZE		0x20000

/* Flash EPROM commands */
#define FLASH_READ_MEMORY	0x00000000
#define FLASH_ERASE_SETUP	0x20202020
#define FLASH_ERASE		0x20202020
#define FLASH_PROGRAM_SETUP	0x40404040
#define FLASH_PROGRAM		0x40404040
#define FLASH_READ_ID_CODES	0x90909090
#define FLASH_ERASE_VERIFY	0xa0a0a0a0
#define FLASH_PROGRAM_VERIFY	0xc0c0c0c0
#define FLASH_RESET		0xffffffff

/* Flash EPROM id codes */
#define MANUFACTURER_CODE	0x89898989
#define DEVICE_CODE		0xb4b4b4b4

void usage( int value );
void high_pr( Process *p, char *filename );
void prog_hf( char *filename );
void check_hf( char *filename );
void reset_fe( void );
void read_i_i( void );
void prog_to_0( void );
void erase( void );
void check_value( long value );
void get_line( FILE *fp, long *file_end, long *hex_data, long *word_ct );
void prog_ep( long *hex_data, long word_ct );          
void check_ep( long *hex_data, long word_ct );          

static int verbosity;

void usage( int value )
{
	printf( "Usage: flash [-h] [-v] <filename>\n" );
	printf( "       -h         Print this message.\n" );
	printf( "       -v         Verbose mode.\n" );
	printf( "       <filename> Name of hex file.\n" );

	exit_terminate( value );
}

int main( int argc, char *argv[] )
{
	char *filename;
	Process *x ;
	int i;

	/* Print message */
	printf( "Transtech flash version %s.\n", "1.8" );
	printf( "Transtech TTM50 flash EPROM programming utility.\n" );
	printf( "Copyright (c) 1993 Transtech Parallel Systems Limited.\n" );

	/* Parse arguments */
	filename = NULL;
	verbosity = 0;
	for (i = 1; i < argc; i++)
	{
		if ((argv[i][0] == '-') || (argv[i][0] == '/'))
		{
			switch (argv[i][1])
			{
				case 'h':
				case 'H':
					usage( 0 );

				case 'v':
				case 'V':
					verbosity++;

					break;

				default:
					usage( 1 );
			}
		}
		else if (filename)
			usage( 1 );
		else
			filename = argv[i];
	}

	if (filename == NULL)
		usage( 1 );

	/* Run process at high priority */
	x = ProcAlloc( high_pr, 0, 1, filename );
	if (x == NULL)
		abort();

	ProcRunHigh(x);
	ProcWait(0x7fffffff);
}

void high_pr( Process *p, char *filename )
{
	p = p;

	/* Reset flash eprom control register */        
	reset_fe();            

	/* Read intelligent identifiers */        
	read_i_i();            

	/* Reset flash eprom control register */        
	reset_fe();            

	/* Read intelligent identifiers */        
	read_i_i();            

	/* Reset flash eprom control register */        
	reset_fe();            

	/* Erase the flash eprom */
	/* First program all words to zero */         
	prog_to_0();  

	/* Check memory is all 0x00000000 */        
	check_value( 0x00000000 );

	/* Reset flash eprom control register */ 
	reset_fe();

	/* Now do the erase algorithm */
	erase();

	/* Check memory is all 0xffffffff */
	check_value( 0xffffffff );

	/* Reset flash eprom control register */
	reset_fe();   

	/* Program with boot-from-rom program values */ 
	prog_hf( filename );

	/* Reset flash eprom control register */         
	reset_fe();           
      
	/* Reset flash eprom control register */         
	reset_fe();  

	/* Check boot-from-rom program values */ 
	check_hf( filename );

	/* Exit */
	printf( "Flash EPROM programmed succesfully.\n" );

	exit_terminate( EXIT_SUCCESS );
}

void prog_hf( char *filename )
{
	long word_ct;
	long file_end;
	long hex_data[5];
	FILE *fp;

	INFO(( "Programming flash eprom ...\n" ));

	/* Open file */
	fp = fopen( filename, "r" );
	if (fp == NULL)
	{
		printf( "Failed to open file.\n" );

		exit_terminate( EXIT_FAILURE );
	}

	file_end = 0;
	while (file_end == 0)
	{
		/* hex_data is an array of 5 longs*/
		get_line(fp,&file_end,hex_data,&word_ct);

		if (word_ct > 0) 
			prog_ep( hex_data, word_ct );          
	}

	fclose( fp );

	INFO(( "... done.\n" ));
}

void check_hf( char *filename )
{
	long word_ct;
	long file_end;
	long hex_data[5];
	FILE *fp;

	INFO(( "Checking flash eprom ...\n" ));

	/* Open file */
	fp = fopen( filename, "r" );
	if (fp == NULL)
	{
		printf( "Failed to open file.\n" );

		exit_terminate( EXIT_FAILURE );
	}

	file_end = 0;
	while (file_end == 0)
	{
		/* hex_data is an array of 5 longs*/
		get_line(fp,&file_end,hex_data,&word_ct);

		if (word_ct > 0) 
			check_ep( hex_data, word_ct );          
	}

	fclose( fp );

	INFO(( "... done.\n" ));
}

void check_value( long value )
{
	long j;
	volatile long ep_data;
	volatile long *fl_eprom;

	fl_eprom = (volatile long *) FLASH_BASE_ADDRESS;
        
	INFO(( "Checking all words are 0x%08lx ...\n", value ));

	for (j = 0; j < FLASH_WORD_SIZE; j++)
	{
		ep_data = fl_eprom[j];
		if (ep_data != value)
		{
			printf( "0x%08p: found 0x%08lx expected 0x%08lx.\n",
			 &fl_eprom[j], ep_data, value );

			exit_terminate( EXIT_FAILURE );
		}
	}

	INFO(( "... done.\n" ));
}

void erase( void )
{
	long i, j, failure;
	volatile long ep_data;
	volatile long *fl_eprom;

	fl_eprom = (volatile long *) FLASH_BASE_ADDRESS;

	INFO(( "Erasing flash EPROM ...\n" ));

	i = 1;
	ep_data = 0x0000000;
	failure = 0x1;
	while ((failure != 0) && (i < 1000))
	{
		VERBOSE(( "*********************************************\n" ));
		VERBOSE(( "erase eprom attempt %lx...", i ));

		fl_eprom[0] = FLASH_ERASE_SETUP;
						/* Erase set-up command */
		fl_eprom[0] = FLASH_ERASE;	/* Erase command */

		ProcWait(10000);		/* Time out 10ms */

		/* Now do erase verify */
		failure = 0;
		for (j = 0; j < FLASH_WORD_SIZE; j++)
		{
			fl_eprom[j] = FLASH_ERASE_VERIFY;
						/* Erase verify command */
			ProcWait( 6 );		/* Time out 6 us */

			if (fl_eprom[j] != 0xffffffff)
			{
				failure++;

				VERBOSE(( "error at 0x%p!\n", &fl_eprom[j] ));

				break;
			}
		}

		i++;
	}

	fl_eprom[0] = FLASH_READ_MEMORY;	/* Go into read mode */

	if (j == FLASH_WORD_SIZE)
	{
		INFO(( "... done.\n"));
	}
	else
	{
		printf( "Failed to erase flash eprom.\n" );

		exit_terminate( EXIT_FAILURE );
	}
}

void reset_fe( void )
{
	volatile long *fl_eprom;

	fl_eprom = (volatile long *) FLASH_BASE_ADDRESS;

	VERBOSE(( "resetting command register on flash eprom\n" ));

	fl_eprom[0] = FLASH_RESET;            
	fl_eprom[0] = FLASH_RESET;            

	ProcWait( 10 );				/* Wait 10 us */
}

void read_i_i( void )
{
	volatile long *fl_eprom;
	long manufacturer;
	long device;

	fl_eprom = (volatile long *) FLASH_BASE_ADDRESS;

	VERBOSE(( "resetting command register on flash eprom\n" ));

	fl_eprom[0] = FLASH_RESET;
	fl_eprom[0] = FLASH_RESET;

	ProcWait(10);				/* Wait 10 us */

	VERBOSE(( "writing command to read intelligent identifier\n" ));   

	fl_eprom[0] = FLASH_READ_ID_CODES;

	ProcWait( 10 );

	VERBOSE(( "reading intelligent identifiers on flash eprom\n" )); 

	manufacturer = fl_eprom[0];
	device = fl_eprom[1];          

	VERBOSE(( "Manufacturer code 0x%08lx, device code 0x%08lx.\n",
	 manufacturer, device ));

	if ((manufacturer != MANUFACTURER_CODE) || (device != DEVICE_CODE))
	{
		VERBOSE(( "Expected 0x%08x and 0x%08x.\n",
		 MANUFACTURER_CODE, DEVICE_CODE ));

		printf( "Failed to read flash EPROM id codes.\n" );
		printf( "Check 12V power supply is connected.\n" );

		exit_terminate( EXIT_FAILURE );
	}
}

void prog_to_0( void )
{
	long i, j;
	long ep_data;
	volatile long *fl_eprom;

	fl_eprom = (volatile long *) FLASH_BASE_ADDRESS;
        
	INFO(( "Programming all words on flash eprom to 0x00000000 ...\n" ));

	for(j = 0; j < FLASH_WORD_SIZE; j++)
	{
		i = 1;
		ep_data = 0xffffffff;

		while ((i < 4) && (ep_data != 0x00000000))
		{
			fl_eprom[0] = FLASH_PROGRAM_SETUP;
						/* Set-up program command */
			fl_eprom[j] = 0x00000000;
						/* Data */
			ProcWait(10);		/* Time out 10 us */
			fl_eprom[0] = FLASH_PROGRAM_VERIFY;
						/* Program verify command */
			ProcWait(6);		/* Time out 6 us */
			ep_data = fl_eprom[j];	/* read data */

			i++;
		}				/* 4 attempts only */
	}

	INFO(( "... done.\n" ));
}

void get_line( FILE *ifp, long *file_end, long *hex_data, long *word_ct )
{
	char line[256];
	unsigned long byte_data[16];
	unsigned char *ptr;
	int i;

	/* Clear byte data */
	for (i = 0; i < 16; i++)
		byte_data[i] = 0;

	/* Read line */
	if (fgets( line, sizeof( line ), ifp ) == NULL)
	{
		*word_ct = 0;
		*file_end = 1;

		VERBOSE(( "Found file end.\n" ));
		return;
	}

	/* Line should contain address and up to sixteen bytes */
	switch (sscanf( line,
	 "%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", &hex_data[0],
	 &byte_data[0], &byte_data[1], &byte_data[2], &byte_data[3],
	 &byte_data[4], &byte_data[5], &byte_data[6], &byte_data[7],
	 &byte_data[8], &byte_data[9], &byte_data[10], &byte_data[11],
	 &byte_data[12], &byte_data[13], &byte_data[14], &byte_data[15] ))
	{
		case 5:
			*word_ct = 1;

			break;

		case 9:
			*word_ct = 2;

			break;

		case 13:
			*word_ct = 3;

			break;

		case 17:
			*word_ct = 4;

			break;

		default:
			/* Check for non white-space characters */
			if (strspn( line, " \t\n\r\f\v" ) != strlen( line ))
			{
				printf( "Invalid line in file.\n" );

				*word_ct = 0;
				*file_end = 1;

				reset_fe();

				exit_terminate( EXIT_FAILURE );
			}

			VERBOSE(( "Found empty line in file.\n" ));

			*word_ct = 0;

			return;
	}

	/* Set address */
	hex_data[0] |= FLASH_BASE_ADDRESS;

	/* Set hex data */
	ptr = (unsigned char *) &hex_data[1];
	for (i = 0; i < 16; i++)
		*(ptr++) = (unsigned char) (byte_data[i] & 255);

	VERBOSE(( "Read %ld words.\n", *word_ct ));
}

void prog_ep( long *hex_data, long word_ct )
{
	long i, j, failure;
	long ep_data;
	volatile long *fl_eprom;

	fl_eprom = (volatile long *) hex_data[0];
        
	for (j = 1; j <= word_ct; j++)
	{
		i = 1;
		failure = TRUE;

		VERBOSE(( "writing value 0x%08lx to address 0x%08lp\n",
		 hex_data[j], &fl_eprom[j-1] ));

		while ((i < 4) && (failure == TRUE))
		{
			fl_eprom[0] = FLASH_PROGRAM_SETUP;
						/* Set-up program command */
			fl_eprom[j-1] = hex_data[j];
						/* Data */
			ProcWait( 10 );		/* Time out 10 us */
			fl_eprom[0] = FLASH_PROGRAM_VERIFY;
						/* Program verify command */
			ProcWait( 6 );		/* Time out 6 us */
			ep_data = fl_eprom[j-1];
						/* Read data */

			/* Check data */
			if (ep_data != hex_data[j])
			{
				failure = TRUE;
			}
			else
				failure = FALSE;

			i++;
		}				/* 4 attempts only */

		if (failure)
		{
			printf( "0x%08p: expected 0x%08lx found 0x%08lx.\n",
			 &fl_eprom[j-1], hex_data[j], ep_data );

			fl_eprom[0] = FLASH_RESET;
			fl_eprom[0] = FLASH_RESET;

			exit_terminate( EXIT_FAILURE );
		}
	}
}

void check_ep( long *hex_data, long word_ct )
{
	long j;
	long ep_data;
	volatile long *fl_eprom;

	fl_eprom = (volatile long *) hex_data[0];
        
	for (j = 1; j <= word_ct; j++)
	{
		VERBOSE(( "checking value 0x%08lx at address 0x%08lp\n",
		 hex_data[j], &fl_eprom[j-1] ));

		ep_data = fl_eprom[j-1];	 /* Read data */

		if (ep_data != hex_data[j])
		{
			printf( "0x%08p: expected 0x%08lx found 0x%08lx.\n",
			 &fl_eprom[j-1], hex_data[j], ep_data );

			exit_terminate( EXIT_FAILURE );
		}
	}
}

