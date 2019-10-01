/*
 *	@(#)test.c	1.1
 *
 *	test.c
 *
 *	Transtech TTM50 SCSI TRAM
 *
 *	Test unit ready example program
 *
 *	Copyright (c) 1993 Transtech Parallel Systems Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <scsi.h>

#define ID		0		/* Target SCSI id */
#define LUN		0		/* Target logical unit number */

int main( void )
{
	int id;				/* SCSI TRAM id */
	struct scsi_address address;	/* SCSI target address */
	struct scsi_command command;	/* SCSI command */
	unsigned char cdb[6];		/* Command descriptor block */
	int result;			/* Result */

	/* Initialise SCSI TRAM */
	id = scsi_initialise();
	if (id < 0)
	{
		printf( "Failed to initialise TRAM.\n" );

		exit( 1 );
	}
	else if (id == ID)
	{
		printf( "Can't test this device.\n" );

		exit( 1 );
	}
	else
		printf( "SCSI TRAM id is %d.\n", id );

	/* Setup target address */
	address.id = ID;
	address.lun = LUN;

	/* Setup command descriptor block */
	cdb[0] = SCMD_TEST_UNIT_READY;
	cdb[1] = LUN << 5;
	cdb[2] = 0;
	cdb[3] = 0;
	cdb[4] = 0;
	cdb[5] = 0;

	/* Setup command structure */
	command.cdb = cdb;		/* Command descriptor block */
	command.cdb_len = 6;		/* CDB length */
	command.data = NULL;		/* Data */
	command.data_len = 0;		/* Data length */
	command.flags = 0;		/* Flags */
	command.timeout = 10;		/* Timeout in seconds */

	/* Run command */
	result = scsi_command( &address, &command );
	if (result)
	{
		printf( "Failed to run command.\n" );
		printf( "Result = %d.\n", result );

		exit( 1 );
	}

	printf( "Completed command.\n" );
	printf( "Status = %d.\n", command.status );
}

