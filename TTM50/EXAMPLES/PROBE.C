/*
 *	probe.c
 *
 *	Transtech SCSI TRAM
 *
 *	SCSI probe utility
 *
 *	Copyright (c) 1993 Transtech Parallel Systems Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <scsi.h>

#define MAX_DEVICE_TYPE		SDEV_COMMUNICATIONS

static char *device_type[] =
{
	"Direct Access",
	"Sequential Access",
	"Printer",
	"Processor",
	"Write Once",
	"CD-ROM",
	"Scanner",
	"Optical Memory",
	"Medium Changer",
	"Communications"
};

static void usage( int value )
{
	printf( "Usage: probe [-h]\n" );
	printf( "       -h         Print this message.\n" );

	exit( value );
}

static void print_data( char *ptr, int size )
{
	int i;

	for (i = 0; i < size; i++)
	{
		if (isprint( *ptr ))
			putchar( *ptr );
		else
			putchar( ' ' );

		ptr++;
	}
}

static void probe_unit( struct scsi_address *address )
{
	struct scsi_command command;
	struct scsi_inquiry inquiry;
	unsigned char cdb[6];

	/* Clear inquiry data */
	memset( &inquiry, 0, sizeof( struct scsi_inquiry ) );

	/* Setup inquiry CDB */
	cdb[0] = SCMD_INQUIRY;
	cdb[1] = address->lun << 5;
	cdb[2] = 0;
	cdb[3] = 0;
	cdb[4] = sizeof( struct scsi_inquiry );
	cdb[5] = 0;

	/* Setup command */
	command.cdb = cdb;
	command.cdb_len = 6;
	command.data = &inquiry;
	command.data_len = sizeof( struct scsi_inquiry );
	command.flags = SFLAG_DATA_IN;
	command.timeout = 10;

	/* Perform inquiry command */
	if (scsi_command( address, &command ))
		return;

	/* Check we have at least the first byte of inquiry data */
	if (command.residue == sizeof( struct scsi_inquiry ))
		return;

	/* Exit if no device */
	if ((inquiry.type == SDEV_UNKNOWN) ||
	 (inquiry.qualifier == SQUAL_NO_DEVICE))
		return;

	/* Print device address */
	printf( " %d  %d  ", address->id, address->lun );

	/* Print device type */
	if (inquiry.type < MAX_DEVICE_TYPE)
		printf( "%- 17s ", device_type[inquiry.type] );
	else 
		printf( "Type 0x%02x         ", inquiry.type );

	/* Print vendor data */
	print_data( inquiry.vendor, 8 );
	putchar( ' ' );

	/* Print product data */
	print_data( inquiry.product, 16 );
	putchar( ' ' );

	/* Print revision data */
	print_data( inquiry.revision, 4 );
	putchar( '\n' );
}

int main( int argc, char *argv[] )
{
	struct scsi_address address;
	int i;
	int id;

	/* Print message */
	printf( "Transtech probe version %s.\n", "1.5" );
	printf( "Transtech TTM50 SCSI probe utility.\n" );
	printf( "Copyright (c) 1993 Transtech Parallel Systems Limited.\n" );
	printf( "\n" );

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

				default:
					usage( 1 );
			}
		}
		else
			usage( 1 );
	}

	/* Initialise SCSI library */
	id = scsi_initialise();
	if (id < 0)
	{
		printf( "Failed to initialise SCSI.\n" );

		exit( 1 );
	}

	printf( "SCSI TRAM id is %d.\n", id );
	printf( "\n" );

	/* Assert SCSI bus reset */
	scsi_reset_bus();

	/* Print header */
	printf( "% 2s % 3s %- 17s %- 8s %- 16s %- 8s\n", "Id",
	 "LUN", "Device Type", "Vendor", "Product", "Revision" );

	/* Probe each SCSI device and unit in turn */
	for (address.id = 0; address.id < 8; address.id++)
	{
		if (address.id != scsi_id)
		{
			for (address.lun = 0; address.lun < 8; address.lun++)
				probe_unit( &address );
		}
	}

	putchar( '\n' );

	exit( 0 );
}

