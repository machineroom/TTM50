/*
 *	@(#)scsi.h	1.56
 *
 *	scsi.h
 *
 *	Transtech SCSI TRAM
 *
 *	Library header file
 *
 *	Copyright (c) 1992 Transtech Parallel Systems Limited
 */

#ifndef SCSI_H
#define SCSI_H

#include <stddef.h>

/* Function return codes */
#define SCSI_OK				0	/* No error */
#define SCSI_NOT_OK			-1	/* General failure */
#define SCSI_NOMEM			-2	/* No memory */
#define SCSI_TIMEOUT			-3	/* Timeout */
#define SCSI_NODEV			-4	/* No device */
#define SCSI_RESET			-5	/* SCSI bus or other reset */
#define SCSI_PHASE_MISMATCH		-6	/* Phase mismatch */
#define SCSI_DATA_PHASE_MISMATCH	-7	/* Data phase mismatch */
#define SCSI_BUSY			-8	/* Device busy */
#define SCSI_UNEXPECTED_DISCONNECT	-9	/* Unexpected disconnect */
#define SCSI_ATN_MESSAGE		-10	/* Initiator asserts ATN at
						   end of message in phase */
#define SCSI_C012_INTERRUPT		-11	/* C012 generated interrupt */
#define SCSI_ABORT			-12	/* Aborted */
#define SCSI_BAD_MESSAGE		-13	/* Bad or unrecognised
						   message */
#define SCSI_SELECTION_TIMEOUT		-14	/* Device selection timed
						   out */

/* Function flags */
#define SFLAG_DATA_IN			0x001	/* Data from target flag */
#define SFLAG_DATA_OUT			0x002	/* Data to target flag */
#define SFLAG_NO_DISCONNECTS		0x004	/* Disable disconnects */
#define SFLAG_NEGOTIATE_SYNCHRONOUS	0x008	/* Negotiate synchronous */
#define SFLAG_NO_ATN			0x010	/* No ATN on selection */
#define SFLAG_SDTR_SENT			0x020	/* Synchronous message sent */
#define SFLAG_SDTR_RECEIVED		0x040	/* Synchronous message
						   received */
#define SFLAG_CDB_RECEIVED		0x080	/* CDB received */
#define SFLAG_STARTED			0x100	/* Command started */

/* SCSI commands */
#define SCMD_TEST_UNIT_READY		0x00	/* Test unit ready */
#define SCMD_REWIND			0x01	/* Rewind */
#define SCMD_REZERO_UNIT		0x01	/* Re-zero unit */
#define SCMD_REQUEST_SENSE		0x03	/* Request sense */
#define SCMD_FORMAT			0x04	/* Format */
#define SCMD_FORMAT_UNIT		0x04	/* Format unit */
#define SCMD_READ_BLOCK_LIMITS		0x05	/* Read block limits */
#define SCMD_INITIALIZE_ELEMENT_STATUS	0x07	/* Initialize element status */
#define SCMD_REASSIGN_BLOCKS		0x07	/* Reassign blocks */
#define SCMD_GET_MESSAGE_6		0x08	/* Get message */
#define SCMD_READ_6			0x08	/* Read */
#define SCMD_RECEIVE			0x08	/* Receive */
#define SCMD_PRINT			0x0a	/* Print */
#define SCMD_SEND_MESSAGE_6		0x0a	/* Send message */
#define SCMD_SEND			0x0a	/* Send */
#define SCMD_WRITE_6			0x0a	/* Write */
#define SCMD_SEEK_6			0x0b	/* Seek */
#define SCMD_SLEW_AND_PRINT		0x0b	/* Slew and print */
#define SCMD_READ_REVERSE		0x0f	/* Read reverse */
#define SCMD_SYNCHRONIZE_BUFFER		0x10	/* Synchronize buffer */
#define SCMD_WRITE_FILEMARKS		0x10	/* Write filemarks */
#define SCMD_SPACE			0x11	/* Space */
#define SCMD_INQUIRY			0x12	/* Inquiry */
#define SCMD_VERIFY_6			0x13	/* Verify */
#define SCMD_RECOVER_BUFFERED_DATA	0x14	/* Recovered buffered data */
#define SCMD_MODE_SELECT_6		0x15	/* Mode select */
#define SCMD_RESERVE			0x16	/* Reserve */
#define SCMD_RESERVE_UNIT		0x16	/* Reserve unit */
#define SCMD_RELEASE			0x17	/* Release */
#define SCMD_RELEASE_UNIT		0x17	/* Release unit */
#define SCMD_COPY			0x18	/* Copy */
#define SCMD_ERASE			0x19	/* Erase */
#define SCMD_MODE_SENSE_6		0x1a	/* Mode sense */
#define SCMD_LOAD_UNLOAD		0x1b	/* Load or unload */
#define SCMD_SCAN			0x1b	/* Scan */
#define SCMD_START_STOP_UNIT		0x1b	/* Start or stop unit */
#define SCMD_STOP_PRINT			0x1b	/* Stop print */
#define SCMD_STOP_START_UNIT		0x1b	/* Stop or start unit */
#define SCMD_RECEIVE_DIAGNOSTIC_RESULTS	0x1c	/* Receive diagnosic results */
#define SCMD_SEND_DIAGNOSTIC		0x1d	/* Send diagnostic */
#define SCMD_PREVENT_ALLOW_MEDIUM_REMOVAL \
					0x1e	/* Prevent or allow medium
						   removal */
#define SCMD_SET_WINDOW			0x24	/* Set window */
#define SCMD_GET_WINDOW			0x25	/* Get window */
#define SCMD_READ_CAPACITY		0x25	/* Read capacity */
#define SCMD_READ_CD_ROM_CAPACITY	0x25	/* Read CD ROM capacity */
#define SCMD_GET_MESSAGE_10		0x28	/* Extended get message */
#define SCMD_READ_10			0x28	/* Extended read */
#define SCMD_READ_GENERATION		0x29	/* Read generation */
#define SCMD_SEND_MESSAGE_10		0x2a	/* Extended send message */
#define SCMD_SEND_10			0x2a	/* Extended send */
#define SCMD_WRITE_10			0x2a	/* Extended write */
#define SCMD_LOCATE			0x2b	/* Locate */
#define SCMD_POSITION_TO_ELEMENT	0x2b	/* Position to element */
#define SCMD_SEEK_10			0x2b	/* Seek */
#define SCMD_ERASE_10			0x2c	/* Erase */
#define SCMD_READ_UPDATED_BLOCK		0x2d	/* Read updated block */
#define SCMD_WRITE_AND_VERIFY		0x2e	/* Write and verify data */
#define SCMD_WRITE_AND_VERIFY_10	0x2e	/* Write and verify data */
#define SCMD_VERIFY_10			0x2f	/* Verify */
#define SCMD_SEARCH_DATA_HIGH		0x30	/* Search for higher data */
#define SCMD_SEARCH_DATA_HIGH_10	0x30	/* Search for higher data */
#define SCMD_OBJECT_POSITION		0x31	/* Object position */
#define SCMD_SEARCH_DATA_EQUAL		0x31	/* Search for equal data */
#define SCMD_SEARCH_DATA_EQUAL_10	0x31	/* Search for equal data */
#define SCMD_SEARCH_DATA_LOW		0x32	/* Search for lower data */
#define SCMD_SEARCH_DATA_LOW_10		0x32	/* Search for lower data */
#define SCMD_SET_LIMITS			0x33	/* Set limits */
#define SCMD_SET_LIMITS_10		0x33	/* Set limits */
#define SCMD_GET_DATA_BUFFER_STATUS	0x34	/* Get data buffer status */
#define SCMD_PRE_FETCH			0x34	/* Pre-fetch */
#define SCMD_READ_POSITION		0x34	/* Read position */
#define SCMD_SYNCHRONIZE_CACHE		0x35	/* Sync cache */
#define SCMD_LOCK_UNLOCK_CACHE		0x36	/* Lock or unlock cache */
#define SCMD_READ_DEFECT_DATA		0x37	/* Read defect data */
#define SCMD_READ_DEFECT_DATA_10	0x37	/* Read defect data */
#define SCMD_MEDIUM_SCAN		0x38	/* Medium scan */
#define SCMD_COMPARE			0x39	/* Compare */
#define SCMD_COPY_AND_VERIFY		0x3a	/* Copy and verify */
#define SCMD_WRITE_BUFFER		0x3b	/* Write buffer */
#define SCMD_READ_BUFFER		0x3c	/* Read buffer */
#define SCMD_UPDATE_BLOCK		0x3d	/* Update block */
#define SCMD_READ_LONG			0x3e	/* Read long */
#define SCMD_WRITE_LONG			0x3f	/* Write long */
#define SCMD_CHANGE_DEFINITION		0x40	/* Change definition */
#define SCMD_WRITE_SAME			0x41	/* Write same */
#define SCMD_READ_SUB_CHANNEL		0x42	/* Write sub-channel */
#define SCMD_READ_TOC			0x43	/* Read table of contents */
#define SCMD_READ_HEADER		0x44	/* Read header */
#define SCMD_PLAY_AUDIO_10		0x45	/* Play audio */
#define SCMD_PLAY_AUDIO_MSF		0x47	/* Play audio MSF */
#define SCMD_PLAY_AUDIO_TRACK_INDEX	0x48	/* Play audio track index */
#define SCMD_PLAY_TRACK_RELATIVE_10	0x49	/* Play track relative */
#define SCMD_PAUSE_RESUME		0x4b	/* Pause or resume */
#define SCMD_LOG_SELECT			0x4c	/* Log select */
#define SCMD_LOG_SENSE			0x4d	/* Log sense */
#define SCMD_MODE_SELECT_10		0x55	/* Mode select */
#define SCMD_MODE_SENSE_10		0x5a	/* Mode sense */
#define SCMD_MOVE_MEDIUM		0xa5	/* Move medium */
#define SCMD_PLAY_AUDIO_12		0xa5	/* Play audio */
#define SCMD_EXCHANGE_MEDIUM		0xa6	/* Exchange medium */
#define SCMD_GET_MESSAGE_12		0xa8	/* Get message */
#define SCMD_READ_12			0xa8	/* Extended read */
#define SCMD_PLAY_TRACK_RELATIVE_12	0xa9	/* Play track relative */
#define SCMD_SEND_MESSAGE_12		0xaa	/* Send message */
#define SCMD_WRITE_12			0xaa	/* Extended write */
#define SCMD_ERASE_12			0xac	/* Extended erase */
#define SCMD_WRITE_AND_VERIFY_12	0xae	/* Write and verify */
#define SCMD_VERIFY_12			0xaf	/* Verify */
#define SCMD_SEARCH_DATA_HIGH_12	0xb0	/* Search for higher data */
#define SCMD_SEARCH_DATA_EQUAL_12	0xb1	/* Search for equal data */
#define SCMD_SEARCH_DATA_LOW_12		0xb2	/* Search for lower data */
#define SCMD_SET_LIMITS_12		0xb3	/* Set limits */
#define SCMD_REQUEST_VOLUME_ELEMENT_ADDRESS \
					0xb5	/* Request volume element
						   address */
#define SCMD_SEND_VOLUME_TAG		0xb6	/* Send volume tag */
#define SCMD_READ_DEFECT_DATA_12	0xb7	/* Read defect data */
#define SCMD_READ_ELEMENT_STATUS	0xb8	/* Read element status */

/* SCSI command sizes */
#define SCSI_STD_INQUIRY_SIZE		36	/* Standard inquiry data */
#define SCSI_ID_SIZE			28	/* Inquiry id size */
#define SCSI_CAPACITY_SIZE		8	/* Capacity data size */
#define SCSI_BLOCK_LIMITS_SIZE		6	/* Block limits data size */
#define SCSI_CDB_SIZE			12	/* Maximum CDB size */
#define SCSI_SENSE_SIZE			18	/* Standard sense data size */

/* SCSI device types */
#define SDEV_DIRECT_ACCESS		0x00	/* Direct access device */
#define SDEV_SEQUENTIAL_ACCESS		0x01	/* Sequential access device */
#define SDEV_PRINTER			0x02	/* Printer device */
#define SDEV_PROCESSOR			0x03	/* Processor device */
#define SDEV_WRITE_ONCE			0x04	/* Write once device */
#define SDEV_CD_ROM			0x05	/* CD ROM device */
#define SDEV_SCANNER			0x06	/* Scanner device */
#define SDEV_OPTICAL_MEMORY		0x07	/* Optical memory device */
#define SDEV_MEDIUM_CHANGER		0x08	/* Medium changer device */
#define SDEV_COMMUNICATIONS		0x09	/* Communications device */
#define SDEV_UNKNOWN			0x1f	/* Unknown or no device type */

/* SCSI device qualifiers */
#define SQUAL_CONNECTED			0x0	/* Device connected */
#define SQUAL_NOT_CONNECTED		0x1	/* Device not connected */
#define SQUAL_NO_DEVICE			0x3	/* No device */

/* SCSI status */
#define SSTAT_GOOD			0x00	/* Good status */
#define SSTAT_CHECK_CONDITION		0x02	/* Check condition */
#define SSTAT_BUSY			0x08	/* Target busy */

/* SCSI sense error codes */
#define SSCODE_CURRENT_ERROR		0x70	/* Error on current command */
#define SSCODE_DEFERRED_ERROR		0x71	/* Error on deferred command */

/* SCSI sense keys */
#define SSKEY_NO_SENSE			0x0	/* No specific sense
						   information */
#define SSKEY_RECOVERED_ERROR		0x1	/* Command completed after
						   error recovery */
#define SSKEY_NOT_READY			0x2	/* Logical unit not ready */
#define SSKEY_MEDIUM_ERROR		0x3	/* Non recoverable medium
						   error */
#define SSKEY_HARDWARE_ERROR		0x4	/* Non recoverable hardware
						   error */
#define SSKEY_ILLEGAL_REQUEST		0x5	/* Invalid parameter */
#define SSKEY_UNIT_ATTENTION		0x6	/* Reset, medium changed etc */
#define SSKEY_DATA_PROTECT		0x7	/* Read or write protection
						   error */
#define SSKEY_BLANK_CHECK		0x8	/* Read or written on invalid
						   medium eg write once */
#define SSKEY_COPY_ABORTED		0xa	/* Copy, compare or copy and
						   verify aborted on error */
#define SSKEY_ABORTED_COMMAND		0xb	/* Target aborted the
						   command */
#define SSKEY_EQUAL			0xc	/* Search data command has
						   satisfied an equal
						   comparison */
#define SSKEY_VOLUME_OVERFLOW		0xd	/* Media overflow */
#define SSKEY_MISCOMPARE		0xe	/* Source data did not match
						   data read from medium */

/* SCSI target routine values */
#define STARG_COMPLETE			0	/* Command completed */
#define STARG_COMMAND			1	/* Command received */
#define STARG_DATA_IN			2	/* Data to initiator */
#define STARG_DATA_OUT			3	/* Data from initiator */
#define STARG_SAVE_AND_DISCONNECT	4	/* Save pointers and
						   disconnect */
#define STARG_DISCONNECT		5	/* Disconnect */
#define STARG_RESELECT			6	/* Reselect */
#define STARG_RESTART			7	/* Restart */
#define STARG_ABORT			8	/* Abort */

/* Miscellaneous constants */
#define SCSI_UNKNOWN_LUN		8	/* No lun specified on
						   selection */

/* SCSI I_T_L nexus */
struct scsi_itl_nexus
{
	int initiator;				/* Initiator device id */
	int target;				/* Target device id */
	int lun;				/* Target LUN */
};

/* SCSI device address */
struct scsi_address
{
	int id;
	int lun;
};

/* SCSI 2 inquiry structure */
struct scsi_inquiry
{
	unsigned	type:5,			/* Peripheral device type */
			qualifier:3,		/* Peripheral qualifier */
			modifier:7,		/* Device type modifier */
			rmb:1,			/* Removable medium bit */
			ansi:3,			/* ANSI approved version */
			ecma:3,			/* ECMA version */
			iso:2,			/* ISO version */
			format:4,		/* Response data format */
			:2,			/* Reserved */
			trmiop:1,		/* Terminate I/O process */
			aenc:1;			/* Asynchronous event
						   notification capability */
	unsigned	length:8,		/* Additional length */
			:8,			/* Reserved */
			:8,			/* Reserved */
			sftre:1,		/* Soft reset */
			cmdque:1,		/* Command queue */
			:1,			/* Reserved */
			linked:1,		/* Linked commands */
			sync:1,			/* Synchronous data
						   transfers */
			wbus16:1,		/* 16 bit wide transfers */
			wbus32:1,		/* 32 bit wide transfers */
			reladr:1;		/* Relative addressing */
	char		vendor[8];		/* Vendor identification */
	char		product[16];		/* Product identification */
	char		revision[4];		/* Product revision level */
};

/* SCSI 2 sense structure */
struct scsi_sense
{
	unsigned	code:7,			/* Error code */
			valid:1,		/* Information valid */
			segment:8,		/* Segment number */
			key:4,			/* Sense key */
			:1,			/* Reserved */
			ili:1,			/* Incorrect length
						   indicator */
			eom:1,			/* End of medium */
			filemark:1,		/* Filemark */
			info_1:8;		/* Information byte 1 (MSB) */
	unsigned	info_2:8,		/* Information byte 2 */
			info_3:8,		/* Information byte 3 */
			info_4:8,		/* Information byte 4 (LSB) */
			length:8;		/* Additional sense length */
	unsigned	specific_1:8,		/* Command specific
						   information byte 1 */
			specific_2:8,		/* Command specific
						   information byte 2 */
			specific_3:8,		/* Command specific
						   information byte 3 */
			specific_4:8;		/* Command specific
						   information byte 4 */
	unsigned	asc:8,			/* Additional sense code */
			ascq:8,			/* Additional sense code
						   qualifier */
			fruc:8,			/* Field replaceable unit
						   code */
			sks_1:8;		/* Sense key specific byte 1 */
	unsigned	sks_2:8,		/* Sense key specific byte 2 */
			sks_3:8,		/* Sense key specific byte 3 */
			asb_1:8,		/* Additional sense byte 1 */
			asb_2:8;		/* Additional sense byte 2 */
};

/* SCSI logical unit record */
struct scsi_unit
{
	int valid;				/* Data is valid */
	int lun;				/* Logical unit number */
	int inquiry_length;			/* Length of inquiry data */
	struct scsi_inquiry inquiry;		/* Inquiry data */
	union
	{
		struct
		{
			int last_block;		/* Last block address */
			int block_size;		/* Last block size */
		} direct_access;
		struct
		{
			int max_block_size;	/* Maximum block size */
			int min_block_size;	/* Minimum block size */
		} sequential_access;
		struct
		{
			int last_block;		/* Last seek block address */
			int block_size;		/* Last block size */
		} cd_rom;
	} parameter;
};

/* SCSI device record */
struct scsi_device
{
	int id;					/* SCSI id */
	struct scsi_unit unit[8];		/* Logical unit records */
	int type;				/* Device type */
	int cycles;				/* Synchronous transfer
						   cycles */
	unsigned char period;			/* Min synchronous transfer
						   period (ns) */
	unsigned char offset;			/* Max synchronous REQ/ACK
						   offset */
	int flags;				/* Flags */
};

/* SCSI target structure */
struct scsi_target
{
	struct scsi_itl_nexus nexus;		/* I_T_L nexus */
	struct scsi_device *initiator_device;	/* Initiator device */
	int lun;				/* Actual lun */
	unsigned char cdb[SCSI_CDB_SIZE];	/* CDB */
	void *data;				/* Data */
	size_t data_len;			/* Length of data */
	unsigned char status;			/* Command status */
	int sense_pending;			/* Sense is pending */
	struct scsi_sense sense;		/* Nexus sense record */
	int reason;				/* Reason for callback */
	int (*callback)( struct scsi_target *target );
						/* Callback function */
	int flags;				/* Flags */
};

/* SCSI command structure */
struct scsi_command
{
	void *cdb;				/* Command block */
	size_t cdb_len;				/* Command block length */
	void *data;				/* Data */
	size_t data_len;			/* Data length */
	size_t residue;				/* Data left */
	unsigned char status;			/* SCSI status */
	int flags;				/* Flags */
	int timeout;				/* Timeout in seconds */
};

/* SCSI configuration link structure */
struct scsi_config
{
	void *data;				/* Data */
	size_t data_len;			/* Data length */
	size_t residue;				/* Data left */
	int timeout;				/* Timeout in seconds */
};

/* Global variables */
extern int scsi_id;				/* SCSI id */
extern int (*scsi_printf)( const char *fmt, ... );
						/* Debug printf function */

int scsi_initialise( void );
struct scsi_device *scsi_device( int id );
struct scsi_device *scsi_find_device( struct scsi_address *address );
int scsi_command( struct scsi_address *address, struct scsi_command *command );
int scsi_target( int (*callback)( struct scsi_target *target ) );
void scsi_reset_bus( void );
int scsi_reset_device( struct scsi_address *address );
void scsi_lock( void );
void scsi_unlock( void );
void *scsi_malloc( size_t size );
void scsi_free( void *ptr );
unsigned scsi_get_value( void *source, int length );
void scsi_set_value( void *dest, unsigned value, int length );
int scsi_config_read( struct scsi_config *config, int lock );
int scsi_config_write( struct scsi_config *config, int lock );
void scsi_config_reset( int lock );
void scsi_config_abort( struct scsi_config *config, int lock );

#endif

