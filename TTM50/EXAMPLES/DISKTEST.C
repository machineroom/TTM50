/*
 *	disktest.c
 *
 *	Transtech SCSI TRAM
 *
 *	SCSI disk test utility
 *
 *	Copyright (c) 1993 Transtech Parallel Systems Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <scsi.h>

#define MAX_BUFFER_SIZE	(1024*1024)		/* 1 MB buffer */
#define MAX_BLOCKS	0x7fff			/* Maximum buffer blocks */
#define VALUE_START	0xdeadbeef		/* Start data value */
#define VALUE_INC	0x12345678		/* Value increment */

#define TRUE		1
#define FALSE		0

/* Debug macros */
#define INFO( x )	(void) ((verbosity > 0) ? printf x : 0)
#define VERBOSE( x )	(void) ((verbosity > 1) ? printf x : 0)

static int verbosity = 0;			/* Verbose level */

static void usage( int value )
{
	printf( "Usage: disktest [-h] [-v] [-c] [-t <id>] [-l <lun>]\n" );
	printf( "       -h         Print this message.\n" );
	printf( "       -v         Verbose mode.\n" );
	printf( "       -c         Continuous mode.\n" );
	printf( "       -i <id>    Target disk id. Default 0.\n" );
	printf( "       -l <lun>   Target disk lun. Default 0.\n" );

	exit( value );
}

static int fill( struct scsi_address *address, unsigned number_blocks,
 void *data, unsigned data_len )
{
	struct scsi_command command;
	unsigned char cdb[10];
	int result;

	/* Setup extended write CDB */
	cdb[0] = SCMD_WRITE_10;
	cdb[1] = ((unsigned) address->lun) << 5;
	cdb[2] = 0;
	cdb[3] = 0;
	cdb[4] = 0;
	cdb[5] = 0;
	cdb[6] = 0;
	cdb[7] = (number_blocks >> 8) & 255;
	cdb[8] = number_blocks & 255;
	cdb[9] = 0;

	/* Setup command */
	command.cdb = cdb;
	command.cdb_len = 10;
	command.data = data;
	command.data_len = data_len;
	command.flags = SFLAG_DATA_OUT;
	command.timeout = 10;

	/* Run command */
	VERBOSE(( "Writing %d bytes.\n", data_len ));

	result = scsi_command( address, &command );
	if (result)
	{
		INFO(( "Write result %d.\n", result ));

		return SCSI_NOT_OK;
	}

	/* Check status and residue */
	if (command.status || command.residue)
	{
		INFO(( "Read status %d. Residue %d.\n", command.status,
		 command.residue ));

		return SCSI_NOT_OK;
	}

	return SCSI_OK;
}

static int dump( struct scsi_address *address, unsigned number_blocks,
 void *data, unsigned data_len )
{
	struct scsi_command command;
	unsigned char cdb[10];
	int result;

	/* Setup extended read CDB */
	cdb[0] = SCMD_READ_10;
	cdb[1] = ((unsigned) address->lun) << 5;
	cdb[2] = 0;
	cdb[3] = 0;
	cdb[4] = 0;
	cdb[5] = 0;
	cdb[6] = 0;
	cdb[7] = (number_blocks >> 8) & 255;
	cdb[8] = number_blocks & 255;
	cdb[9] = 0;

	/* Setup command */
	command.cdb = cdb;
	command.cdb_len = 10;
	command.data = data;
	command.data_len = data_len;
	command.flags = SFLAG_DATA_IN;
	command.timeout = 10;

	/* Run command */
	VERBOSE(( "Reading %d bytes.\n", data_len ));

	result = scsi_command( address, &command );
	if (result)
	{
		INFO(( "Read result %d.\n", result ));

		return SCSI_NOT_OK;
	}

	/* Check status and residue */
	if (command.status || command.residue)
	{
		INFO(( "Read status %d. Residue %d.\n", command.status,
		 command.residue ));

		return SCSI_NOT_OK;
	}

	return SCSI_OK;
}

static void fill_test( struct scsi_address *address, unsigned buffer_blocks,
 void *buffer, unsigned buffer_size )
{
	unsigned long *ptr;			/* Data pointer */
	unsigned long value;			/* Data value */
	int i;					/* Loop counter */

	/* Fill disk with data */
	INFO(( "Write test.\n" ));

	VERBOSE(( "Setting buffer.\n" ));

	ptr = (unsigned long *) buffer;
	value = VALUE_START;
	for (i = 0; i < buffer_size; i += 4)
	{
		*ptr = value;

		ptr++;
		value += VALUE_INC;
	}

	if (fill( address, buffer_blocks, buffer, buffer_size ))
	{
		printf( "Failed to write to disk.\n" );

		exit( 1 );
	}

	/* Test disk data */
	INFO(( "Read test.\n" ));

	VERBOSE(( "Clearing buffer.\n" ));

	memset( buffer, 0x00, buffer_size );

	if (dump( address, buffer_blocks, buffer, buffer_size ))
	{
		printf( "Failed to read from disk.\n" );

		exit( 1 );
	}

	VERBOSE(( "Testing buffer.\n" ));

	ptr = (unsigned long *) buffer;
	value = VALUE_START;
	for (i = 0; i < buffer_size; i += 4)
	{
		if (*ptr != value)
		{
			printf( "0x%08x: Found 0x%08lx expected 0x%08lx.\n",
			 i, *ptr, value );

			exit( 1 );
		}

		ptr++;
		value += VALUE_INC;
	}
}

int main( int argc, char *argv[] )
{
	struct scsi_address address;		/* SCSI target address */
	struct scsi_device *device;		/* SCSI device record */
	struct scsi_unit *unit;			/* SCSI unit record */
	void *buffer;				/* Data buffer */
	int block_size;				/* Disk block size */
	int number_blocks;			/* Number of disk blocks */
	int buffer_size;			/* Buffer size */
	int buffer_blocks;			/* Buffer blocks */
	int continuous;				/* Continuous flag */
	int id;					/* SCSI TRAM id */
	int i;					/* Loop counter */

	/* Print message */
	printf( "Transtech disktest version %s.\n", "1.11" );
	printf( "Transtech TTM50 SCSI disk test utility.\n" );
	printf( "Copyright (c) 1993 Transtech Parallel Systems Limited.\n" );
	printf( "\n" );

	/* Default parameters */
	address.id = 0;
	address.lun = 0;
	verbosity = 0;
	continuous = FALSE;

	/* Parse arguments */
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

				case 'c':
				case 'C':
					continuous = TRUE;

					break;

				case 'i':
				case 'I':
					if (++i == argc)
						usage( 1 );
					else if (sscanf( argv[i], "%d",
					 &address.id ) != 1)
						usage( 1 );
					else if ((address.id < 0) ||
					 (address.id > 7))
					{
						printf( "Invalid SCSI id.\n" );

						exit( 1 );
					}

					break;

				case 'l':
				case 'L':
					if (++i == argc)
						usage( 1 );
					else if (sscanf( argv[i], "%d",
					 &address.lun ) != 1)
						usage( 1 );
					else if ((address.lun < 0) ||
					 (address.lun > 7))
					{
						printf( "Invalid lun.\n" );

						exit( 1 );
					}

					break;

				default:
					usage( 1 );
			}
		}
		else
			usage( 1 );
	}

	/* Initialise SCSI library */
	VERBOSE(( "Initialising SCSI library.\n" ));

	id = scsi_initialise();
	if (id < 0)
	{
		printf( "Failed to initialise SCSI.\n" );

		exit( 1 );
	}

	INFO(( "SCSI TRAM id is %d.\n", id ));

	if (id == address.id)
	{
		printf( "Disk and TRAM SCSI ids cannot be the same.\n" );

		exit( 1 );
	}

	/* Reset SCSI bus */
	VERBOSE(( "Resetting SCSI bus.\n" ));

	scsi_reset_bus();

	/* Set SCSI error function */
	if (verbosity > 1)
		scsi_printf = printf;

	/* Look for disk */
	VERBOSE(( "Looking for direct access device target %d lun %d.\n",
	 address.id, address.lun ));

	device = scsi_find_device( &address );
	if (device == NULL)
	{
		printf( "Failed to find disk.\n" );

		exit( 1 );
	}
	else if (device->type != SDEV_DIRECT_ACCESS)
	{
		INFO(( "Found device of type %d.\n", device->type ));

		printf( "Not a disk device.\n" );

		exit( 1 );
	}

	/* Get block size and number of blocks */
	unit = &device->unit[address.lun];

	block_size = unit->parameter.direct_access.block_size;
	number_blocks = unit->parameter.direct_access.last_block + 1;

	if ((block_size < 4) || (block_size > MAX_BUFFER_SIZE) ||
	 (number_blocks < 1))
	{
		printf( "Invalid block size %d, number %d.\n", block_size,
		 number_blocks );

		exit( 1 );
	}

	VERBOSE(( "Disk has %d blocks of size %d.\n", number_blocks,
	 block_size ));

	/* Allocate buffer with whole number of blocks */
	buffer_blocks = MAX_BUFFER_SIZE/block_size;

	if (buffer_blocks > MAX_BLOCKS)
		buffer_blocks = MAX_BLOCKS;

	if (buffer_blocks > number_blocks)
		buffer_blocks = number_blocks;

	buffer_size = buffer_blocks*block_size;

	VERBOSE(( "Allocating buffer of size %d.\n", buffer_size ));

	buffer = scsi_malloc( buffer_size );
	if (buffer == NULL)
	{
		printf( "Failed to allocate buffer.\n" );

		exit( 1 );
	}

	/* Do tests */
	for (i = 1; TRUE; i++)
	{
		if (continuous)
		{
			printf( "\n" );
			printf( "... Trial %d ...\n", i );

			INFO(( "\n" ));
		}

		/* Do fill test */
		fill_test( &address, buffer_blocks, buffer, buffer_size );

		/* Print success message */
		INFO(( "\n" ));

		printf( "Passed all tests.\n" );

		if (!continuous)
			break;
	}

	/* Free buffer */
	VERBOSE(( "Freeing buffer.\n" ));

	scsi_free( buffer );

	exit( 0 );
}

