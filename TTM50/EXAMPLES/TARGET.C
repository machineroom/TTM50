/*
 *	@(#)target.c	1.2
 *
 *	target.c
 *
 *	Transtech TTM50 SCSI TRAM
 *
 *	Target driver example program
 *
 *	Copyright (c) 1993 Transtech Parallel Systems Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <scsi.h>

/* Debug message macros */
#define INFO( x ) printf x			/* Information message */
#define ERROR( x ) printf x			/* Error message */

#define DATA_SIZE		4096		/* Data buffer size */
#define TRUE			1
#define FALSE			0
#define OK			0
#define NOT_OK			-1

/* Driver state structure */
volatile struct state
{
	int residue;				/* Bytes left to transfer */
	int direction;				/* Data in or out */
	unsigned char *data;			/* Data */
	int busy;				/* Busy flag */
	int working;				/* Working flag */
} *state;

static int target_command( struct scsi_target *target )
{
	struct scsi_inquiry *inquiry;

	/* Check busy flag */
	if (state->busy)
	{
		ERROR(( "Driver is busy.\n" ));

		target->status = SSTAT_BUSY;

		return STARG_COMPLETE;
	}

	/* Act on command opcode */
	switch (target->cdb[0])
	{
		case SCMD_TEST_UNIT_READY:
			INFO(( "Test unit ready.\n" ));

			target->status = SSTAT_GOOD;

			return STARG_COMPLETE;

		case SCMD_INQUIRY:
			INFO(( "Inquiry.\n" ));

			/* Setup inquiry structure */
			inquiry = (struct scsi_inquiry *) state->data;

			memset( inquiry, 0, SCSI_STD_INQUIRY_SIZE );

			inquiry->type = SDEV_PROCESSOR;
			inquiry->qualifier = SQUAL_CONNECTED;
			inquiry->format = 2;
			inquiry->length = SCSI_STD_INQUIRY_SIZE - 4;
			inquiry->sync = 1;

			strcpy( inquiry->vendor,
			 "TRANST  Example Target 0001" );

			/* Setup data transfer */
			state->residue = target->cdb[4];
			if (state->residue == 0)
			{
				target->status = SSTAT_GOOD;

				return STARG_COMPLETE;
			}
			else if (state->residue > SCSI_STD_INQUIRY_SIZE)
				state->residue = SCSI_STD_INQUIRY_SIZE;

			target->data_len = state->residue;
			target->data = state->data;

			/* Set busy flag */
			state->busy = TRUE;

			return STARG_DATA_IN;

		case SCMD_SEND:
			INFO(( "Send.\n" ));

			/* Find requested transfer length */
			state->residue = scsi_get_value( &target->cdb[2], 3 );
			if (state->residue == 0)
			{
				target->status = SSTAT_GOOD;

				return STARG_COMPLETE;
			}

			/* Setup first data transfer */
			if (state->residue > DATA_SIZE)
				target->data_len = DATA_SIZE;
			else
				target->data_len = state->residue;

			target->data = state->data;

			/* Set busy flag */
			state->busy = TRUE;

			return STARG_DATA_OUT;

		case SCMD_RECEIVE:
			INFO(( "Receive.\n" ));

			/* Find requested transfer length */
			state->residue = scsi_get_value( &target->cdb[2], 3 );
			if (state->residue == 0)
			{
				target->status = SSTAT_GOOD;

				return STARG_COMPLETE;
			}

			/* Setup first data transfer */
			if (state->residue > DATA_SIZE)
				target->data_len = DATA_SIZE;
			else
				target->data_len = state->residue;

			target->data = state->data;

			/* Set busy flag */
			state->busy = TRUE;

			return STARG_DATA_IN;

		default:
			ERROR(( "Unrecognised command 0x%02x.\n",
			 target->cdb[0] ));

			target->status = SSTAT_CHECK_CONDITION;

			return STARG_COMPLETE;
	}
}

/* Target driver callback function */
static int target_callback( struct scsi_target *target )
{
	/* Act on callback reason */
	switch (target->reason)
	{
		case STARG_COMMAND:
			return target_command( target );

		case STARG_DATA_IN:
		case STARG_DATA_OUT:
			INFO(( "Data in/out.\n" ));

			/* Update bytes left count */
			state->residue -= target->data_len;
			if (state->residue == 0)
			{
				target->status = SSTAT_GOOD;

				state->busy = FALSE;

				return STARG_COMPLETE;
			}
			else if (state->residue < 0)
			{
				ERROR(( "Invalid residue.\n" ));

				state->busy = FALSE;

				return SCSI_NOT_OK;
			}

			/* Continue if disconnects disabled */
			if (target->flags & SFLAG_NO_DISCONNECTS)
			{
				if (state->residue > DATA_SIZE)
					target->data_len = DATA_SIZE;
				else
					target->data_len = state->residue;

				INFO(( "Continuing.\n" ));

				return target->reason;
			}

			/* Perform save and disconnect */
			state->direction = target->reason;

			return STARG_SAVE_AND_DISCONNECT;

		case STARG_DISCONNECT:
			INFO(( "Disconnected.\n" ));

			/* Set working flag */
			state->working = TRUE;

			/* Release sole use of the SCSI system */
			scsi_unlock();

			/* Get sole use of the SCSI system */
			scsi_lock();

			/* Exit if terminated */
			if (!state->working)
			{
				INFO(( "Restarting.\n" ));

				state->busy = FALSE;

				return STARG_RESTART;
			}

			/* Clear working flag */
			state->working = FALSE;

			/* Do reselect */
			return STARG_RESELECT;

		case STARG_RESELECT:
			INFO(( "Reselected.\n" ));

			/* Setup next data transfer */
			if (state->residue > DATA_SIZE)
				target->data_len = DATA_SIZE;
			else
				target->data_len = state->residue;

			return state->direction;

		case SCSI_RESET:
			ERROR(( "Reset.\n" ));

			/* Reset busy flag if not working */
			if (!state->working)
				state->busy = FALSE;

			/* Reset working flag */
			state->working = FALSE;

			return SCSI_RESET;

		case STARG_ABORT:
			ERROR(( "Abort.\n" ));

			/* Reset busy flag if not working */
			if (!state->working)
				state->busy = FALSE;

			/* Reset working flag */
			state->working = FALSE;

			return STARG_ABORT;

		default:
			ERROR(( "Invalid callback reason %d.\n",
			 target->reason ));

			/* Reset busy flag if not working */
			if (!state->working)
				state->busy = FALSE;

			/* Reset working flag */
			state->working = FALSE;

			return SCSI_NOT_OK;
	}

	/* Should not be here */
	return SCSI_NOT_OK;
}

int main( void )
{
	int id;

	/* Initialise SCSI TRAM */
	id = scsi_initialise();
	if (id < 0)
	{
		printf( "Failed to initialise SCSI.\n" );

		exit( 1 );
	}

	printf( "SCSI target id is %d.\n", id );

	/* Set error message handler function */
	scsi_printf = printf;

	/* Allocate state structure */
	state = (struct state *) scsi_malloc( sizeof( struct state ) );
	if (state == NULL)
	{
		printf( "Failed to allocate state.\n" );

		exit( 1 );
	}

	/* Allocate data buffer */
	state->data = scsi_malloc( DATA_SIZE );
	if (state->data == NULL)
	{
		printf( "Failed to allocate data.\n" );
		return OK;
	}

	/* Reset state flags */
	state->busy = FALSE;
	state->working = FALSE;

	/* Run target process */
	if (scsi_target( target_callback ))
	{
		printf( "Failed to run target routine.\n" );

		exit( 1 );
	}
}

