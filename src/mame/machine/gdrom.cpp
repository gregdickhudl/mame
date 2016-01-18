// license:BSD-3-Clause
// copyright-holders:smf, Angelo Salese
/***************************************************************************

 gdrom.c - Implementation of the Sega GD-ROM device

***************************************************************************/

#include "gdrom.h"

#define GDROM_BUSY_STATE    0x00
#define GDROM_PAUSE_STATE   0x01
#define GDROM_STANDBY_STATE 0x02
#define GDROM_PLAY_STATE    0x03
#define GDROM_SEEK_STATE    0x04
#define GDROM_SCAN_STATE    0x05
#define GDROM_OPEN_STATE    0x06
#define GDROM_NODISC_STATE  0x07
#define GDROM_RETRY_STATE   0x08
#define GDROM_ERROR_STATE   0x09


/*
  Following data contain real command reply obfuscated, it is extracted by such code:

   for (UINT32 i = 0, offset = 0; i < length; i++)
   {
     offset++;
     offset += GDROM_Cmd71_Reply[offset] - 2;
     result[i] = GDROM_Cmd71_Reply[offset++];
   }

 in the case of cmd 0x71 only 1 byte extracted, and later verified for &0x10.

 SysCalls/BIOS uses same extract routine for command 0x72 (14 bytes), but it seems not used on practice
*/

static const UINT8 GDROM_Cmd71_Reply[] =
{
	0x96, 0x0B, 0x45, 0xF0, 0x7E, 0xFF, 0x3D, 0x06, 0x4D, 0x7D, 0x10, 0xBF, 0x07, 0x00, 0x73, 0xCF,
	0x9C, 0x00, 0xBC, 0x0C, 0x1C, 0xAF, 0x1C, 0x30, 0xE7, 0xA7, 0x03, 0xA8, 0x98, 0x00, 0xBD, 0x0F,
	0xBD, 0x5B, 0xAA, 0x50, 0x23, 0x39, 0x31, 0x10, 0x0E, 0x69, 0x13, 0xE5, 0x00, 0xD2, 0x0D, 0x66,
	0x54, 0xBF, 0x5F, 0xFD, 0x37, 0x74, 0xF4, 0x5B, 0x22, 0x00, 0xC6, 0x09, 0x0F, 0xCA, 0x93, 0xE8,
	0xA4, 0xAB, 0x00, 0x61, 0x0E, 0x2E, 0xE1, 0x4B, 0x76, 0x8B, 0x6A, 0xA5, 0x9C, 0xE6, 0x23, 0xC4,
	0x00, 0x4B, 0x06, 0x1B, 0x91, 0x01, 0x00, 0xE2, 0x0D, 0xCF, 0xCA, 0x38, 0x3A, 0xB9, 0xE7, 0x91,
	0xE5, 0xEF, 0x4B, 0x00, 0xD6, 0x09, 0xD3, 0x68, 0x3E, 0xC4, 0xAF, 0x2D, 0x00, 0x2A, 0x0D, 0xF9,
	0xFC, 0x78, 0xED, 0xAE, 0x99, 0xB3, 0x32, 0x5A, 0xE7, 0x00, 0x4C, 0x0A, 0x22, 0x97, 0x5B, 0x82,
	0x06, 0x7A, 0x4C, 0x00, 0x42, 0x0E, 0x57, 0x78, 0x46, 0xF5, 0x20, 0xFC, 0x6B, 0xCB, 0x01, 0x5B,
	0x86, 0x00, 0xE4, 0x0E, 0xB2, 0x26, 0xCD, 0x71, 0xE3, 0xA5, 0x33, 0x06, 0x8E, 0x9A, 0x50, 0x00,
	0x07, 0x07, 0xF5, 0x34, 0xEF, 0xE6, 0x00, 0x32, 0x0F, 0x13, 0x41, 0x59, 0x56, 0x0F, 0x02, 0x38,
	0x2A, 0x64, 0x2A, 0x07, 0x3E, 0x00, 0x52, 0x11, 0x2A, 0x1D, 0x5F, 0x76, 0x66, 0xA0, 0xB2, 0x2F,
	0x97, 0xC7, 0x5E, 0x6E, 0x52, 0xE2, 0x00, 0x58, 0x09, 0xCA, 0x89, 0xA5, 0xDF, 0x0A, 0xDE, 0x00,
	0x50, 0x06, 0x49, 0xB8, 0xB4, 0x00, 0x77, 0x05, 0x24, 0xE8, 0x00, 0xBB, 0x0C, 0x91, 0x89, 0xA2,
	0x8B, 0x62, 0xDE, 0x6A, 0xC6, 0x60, 0x00, 0xE7, 0x0F, 0x0F, 0x11, 0x96, 0x55, 0xD2, 0xBF, 0xE6,
	0x48, 0x0B, 0x5C, 0xAB, 0xDC, 0x00, 0xBA, 0x0A, 0x30, 0xD7, 0x48, 0x0E, 0x78, 0x63, 0x0C, 0x00,
	0xD2, 0x0D, 0xFB, 0x8A, 0xA3, 0xFE, 0xF8, 0x3A, 0xDD, 0x88, 0xA9, 0x4B, 0x00, 0xA2, 0x0A, 0x75,
	0x5D, 0x0D, 0x37, 0x24, 0xC5, 0x9D, 0x00, 0xF7, 0x0B, 0x25, 0xEF, 0xDB, 0x41, 0xE0, 0x52, 0x3E,
	0x4E, 0x00, 0xB7, 0x03, 0x00, 0xE5, 0x11, 0xB9, 0xDE, 0x5A, 0x57, 0xCF, 0xB9, 0x1A, 0xFC, 0x7F,
	0x26, 0xEE, 0x7B, 0xCD, 0x2B, 0x00, 0x4B, 0x08, 0xB8, 0x09, 0x70, 0x6A, 0x9F, 0x00, 0x4B, 0x11,
	0x8C, 0x15, 0x87, 0xA3, 0x05, 0x4F, 0x37, 0x8E, 0x63, 0xDE, 0xEF, 0x39, 0xFC, 0x4B, 0x00, 0xAB,
	0x10, 0x0B, 0x91, 0xAA, 0x0F, 0xE1, 0xE9, 0xAE, 0x69, 0x3A, 0xF8, 0x03, 0x69, 0xD2, 0x00, 0xE2,
	0x07, 0xC1, 0x5C, 0x3D, 0x82, 0x00, 0xA9, 0x08, 0x68, 0xC4, 0xAD, 0x2E, 0xD1, 0x00, 0xF7, 0x0E,
	0xC6, 0x47, 0xC8, 0xCD, 0x8E, 0x7C, 0x00, 0x5C, 0x95, 0xB9, 0xF4, 0x00, 0xE3, 0x04, 0x5B, 0x00,
	0x74, 0x07, 0x65, 0xC7, 0x84, 0x8E, 0x00, 0xC6, 0x07, 0x61, 0x80, 0x44, 0x3F, 0x00, 0xC8, 0x0E,
	0x72, 0x78, 0x47, 0xD3, 0xC2, 0x4D, 0xAF, 0xC0, 0x54, 0x13, 0x31, 0x00, 0xF7, 0x0D, 0x48, 0xD8,
	0xE2, 0x92, 0x9F, 0x7F, 0x2F, 0x44, 0x68, 0x33, 0x00, 0x0D, 0x10, 0xAB, 0xFE, 0xEA, 0x8E, 0x19,
	0x81, 0xF8, 0x6F, 0x7C, 0xDE, 0xE1, 0xB3, 0x06, 0x00, 0x4D, 0x11, 0x66, 0xAE, 0x4C, 0xF9, 0xB7,
	0x2F, 0xEE, 0xB0, 0x8E, 0x7E, 0xE1, 0x8D, 0x95, 0x6F, 0x00, 0xF4, 0x0D, 0x88, 0x9D, 0xCA, 0xE3,
	0xC4, 0xB2, 0x47, 0xBB, 0xA0, 0x69, 0x00, 0xF3, 0x0B, 0x48, 0x17, 0x41, 0x64, 0xA0, 0x0E, 0x71,
	0x82, 0x00, 0x34, 0x1E, 0x18, 0x4D, 0x85, 0x80, 0x4C, 0xA9, 0x0B, 0x66, 0x9B, 0x75, 0x13, 0x61,
	0x70, 0x27, 0x81, 0x7A, 0x02, 0xCD, 0x57, 0xAB, 0xDF, 0x02, 0x93, 0x52, 0x83, 0xDF, 0x48, 0xA8,
	0xA6, 0x9E, 0x74, 0x6F, 0x89, 0x03, 0x28, 0x25, 0x52, 0x96, 0xFF, 0x67, 0x7A, 0xD8, 0x3C, 0xB1,
	0x2C, 0x46, 0x84, 0xEF, 0xE1, 0xC1, 0xC6, 0xC9, 0xDC, 0x96, 0xAA, 0xA9, 0xC4, 0x82, 0x58, 0x27,
	0x57, 0x75, 0x67, 0x34, 0xFB, 0x3B, 0x25, 0xBF, 0xFB, 0x3B, 0xF6, 0x13, 0xEC, 0x96, 0xE5, 0x16,
	0x26, 0xFD, 0xA8, 0xDA, 0x1B, 0xC6, 0x50, 0x7F, 0x47, 0xFF, 0x08, 0x55, 0x08, 0xED, 0x00, 0x93,
	0x9B, 0xC4, 0x71, 0x67, 0xEC, 0xA6, 0xCC, 0x16, 0x20, 0x87, 0x47, 0x07, 0xA6, 0x00, 0x79, 0x5D,
	0x4F, 0xAB, 0xA1, 0x6F, 0x7A, 0x6B, 0x27, 0xC4, 0xDA, 0xA3, 0xC3, 0x94, 0x4F, 0x7F, 0xF3, 0xE5,
	0x1B, 0x6F, 0xCC, 0xE5, 0xF0, 0xE5, 0x9D, 0xC9, 0xAE, 0xFD, 0x39, 0xAC, 0x4C, 0xE5, 0x58, 0x83,
	0x25, 0x65, 0x92, 0x74, 0x9E, 0x81, 0xA0, 0xB6, 0xA9, 0x02, 0x9B, 0x07, 0xB6, 0xE7, 0x79, 0x57,
	0xD9, 0x4A, 0xCE, 0xFA, 0xB4, 0x94, 0x05, 0xCC, 0x86, 0x3C, 0xDD, 0x06, 0xCD, 0xA6, 0x24, 0x24,
	0xFA, 0xC1, 0xF9, 0x48, 0xC9, 0x0C, 0x6C, 0xC4, 0x96, 0x82, 0x17, 0xF6, 0x31, 0x09, 0xC4, 0xE2,
	0x77, 0xFD, 0xCF, 0x46, 0x18, 0xB2, 0x5F, 0x01, 0x6B, 0xD1, 0x7B, 0x56, 0xB8, 0x94, 0x4A, 0xE5,
	0x6C, 0x19, 0xF0, 0xC0, 0xB6, 0x70, 0x93, 0xF7, 0xD3, 0xD1, 0x2B, 0x6E, 0x7C, 0x53, 0x6D, 0x85,
	0xD1, 0x0C, 0x8B, 0x77, 0xEE, 0x90, 0xDA, 0x15, 0x55, 0xE0, 0x58, 0x09, 0x56, 0xFC, 0x31, 0x9F,
	0xAF, 0x46, 0xCB, 0xC3, 0x8D, 0x71, 0x75, 0xF2, 0x2C, 0xC3, 0xBB, 0xA1, 0xC4, 0xCF, 0x27, 0x56,
	0x7C, 0x9B, 0xFE, 0xAF, 0x3E, 0x4E, 0xB4, 0xCD, 0x6A, 0xAA, 0xF5, 0xF3, 0xE3, 0x22, 0x82, 0xE1,
	0xA5, 0x68, 0xB3, 0xDB, 0x8F, 0x9E, 0x5E, 0x7B, 0x90, 0xF0, 0x79, 0x3F, 0x52, 0x8C, 0x61, 0x88,
	0x76, 0xAE, 0x14, 0x63, 0x19, 0x0F, 0x1D, 0xCE, 0xA1, 0x63, 0x10, 0xB2, 0xE2, 0xD7, 0x94, 0xB1,
	0x33, 0xCB, 0x28, 0x85, 0x7D, 0x9B, 0xF5, 0xF4, 0x25, 0x50, 0x9B, 0xDB, 0x35, 0xA5, 0xB0, 0x9C,
	0x09, 0x92, 0xE3, 0x31, 0x40, 0xAB, 0x4D, 0xF4, 0x35, 0xE8, 0xB3, 0x0A, 0x21, 0xC3, 0x86, 0x9C,
	0xCB, 0x29, 0xA4, 0x77, 0x57, 0xBC, 0xD8, 0xDA, 0xA5, 0x82, 0x80, 0xE8, 0xCF, 0x72, 0x81, 0xAD,
	0x2E, 0x28, 0xFF, 0xD8, 0xB6, 0xD1, 0x2B, 0x97, 0x00, 0xFF, 0xE1, 0x06, 0x44, 0x39, 0x1C, 0x4B,
	0xAB, 0x19, 0x5B, 0x4D, 0xD6, 0x3E, 0x1B, 0x5C, 0x64, 0xBB, 0x32, 0x68, 0xF5, 0x7C, 0xC9, 0x9E,
	0xE8, 0xB4, 0x29, 0x1B, 0x7F, 0x4D, 0x80, 0x80, 0x7E, 0x8B, 0x1C, 0x0A, 0xE6, 0x9A, 0xBF, 0x49,
	0x1E, 0xC5, 0xB6, 0x67, 0x7D, 0x05, 0xE4, 0x90, 0x40, 0x4B, 0xAF, 0x9B, 0x52, 0xDE, 0x17, 0x80,
	0x81, 0x56, 0xEA, 0x3A, 0x53, 0x82, 0x8C, 0x62, 0xFB, 0x96, 0x97, 0x6F, 0xC1, 0x16, 0x78, 0xD4,
	0x7B, 0xE7, 0xB9, 0x5A, 0x2A, 0xEB, 0x87, 0x68, 0x33, 0xD3, 0x31, 0x45, 0xFA, 0xFE, 0xF4, 0x1C,
	0x90, 0x86, 0x73, 0x77, 0xD9, 0xA9, 0xD1, 0x4A, 0x4A, 0xCF, 0xAE, 0x23, 0xDB, 0xF9, 0x09, 0xD8,
	0x18, 0xDC, 0x6A, 0x0D, 0xE4, 0x19, 0x8C, 0x65, 0xC6, 0x64, 0xC7, 0xDC, 0xA9, 0xE3, 0x91, 0xB1,
	0x4C, 0xC8, 0xC1, 0x9E, 0x3B, 0x7F, 0xCB, 0xA3, 0xCF, 0xDD, 0xF0, 0x1D, 0x07, 0x6E, 0xDC, 0xCE,
	0x0D, 0xCD, 0x7E, 0x1E, 0x55, 0x11, 0x8B, 0xDF, 0x3A, 0xAB, 0xB6, 0x3B, 0x6E, 0x52, 0x7F, 0xA7,
	0x00, 0xD1, 0x33, 0xBE, 0xF2, 0x9B, 0xFC, 0x4A, 0xCF, 0x9D, 0x8F, 0xC6, 0xC4, 0x7B, 0xDA, 0xE7,
	0x2A, 0x1C, 0x26, 0x6E
};


void gdrom_device::device_reset()
{
	static const UINT8 GDROM_Def_Cmd11_Reply[32] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0xB4, 0x19, 0x00, 0x00, 0x08, 0x53, 0x45, 0x20, 0x20, 0x20, 0x20,
		0x20, 0x20, 0x52, 0x65, 0x76, 0x20, 0x36, 0x2E, 0x34, 0x32, 0x39, 0x39, 0x30, 0x33, 0x31, 0x36
	};

	for(int i = 0;i<32;i++)
		GDROM_Cmd11_Reply[i] = GDROM_Def_Cmd11_Reply[i];

	atapi_cdrom_device::device_reset();
}

// scsicd_exec_command
//
// Execute a SCSI command.

void gdrom_device::ExecCommand()
{
	switch ( command[0] )
	{
		case 0x11: // REQ_MODE
			m_phase = SCSI_PHASE_DATAIN;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			printf("REQ_MODE %02x %02x %02x %02x %02x %02x\n",
				command[0], command[1],
				command[2], command[3],
				command[4], command[5]);
//          if (SCSILengthFromUINT8( &command[ 4 ] ) < 32) return -1;
			transferOffset = command[2];
			m_transfer_length = SCSILengthFromUINT8( &command[ 4 ] );
			break;

		case 0x12: // SET_MODE
			logerror("GDROM: SET_MODE\n");
			m_phase = SCSI_PHASE_DATAOUT;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			//transferOffset = command[2];
			m_transfer_length = SCSILengthFromUINT8( &command[ 4 ] );
			printf("SET_MODE %02x %02x\n",command[2],command[4]);
			break;

		case 0x30: // CD_READ
			if (command[1] & 1)
			{
				fatalerror("GDROM: MSF mode used for CD_READ, unsupported\n");
				m_transfer_length = 0;
			}
			else
			{
				m_lba = (command[2]<<16 | command[3]<<8 | command[4]) - 150;
				m_blocks = command[8]<<16 | command[9]<<8 | command[10];

				read_type = (command[1] >> 1) & 7;
				data_select = (command[1]>>4) & 0xf;

				if (read_type != 2) // mode 1
				{
					fatalerror("GDROM: Unhandled read_type %d\n", read_type);
				}

				if (data_select != 2)   // just sector data
				{
					fatalerror("GDROM: Unhandled data_select %d\n", data_select);
				}

				printf("GDROM: CD_READ at LBA %x for %d blocks (%d bytes, read type %d, data select %d)\n", m_lba, m_blocks, m_blocks * m_sector_bytes, read_type, data_select);

				if (m_num_subblocks > 1)
				{
					m_cur_subblock = m_lba % m_num_subblocks;
					m_lba /= m_num_subblocks;
				}
				else
				{
					m_cur_subblock = 0;
				}

				if (m_cdda != nullptr)
				{
					m_cdda->stop_audio();
				}

				m_phase = SCSI_PHASE_DATAIN;
				m_status_code = SCSI_STATUS_CODE_GOOD;
				m_transfer_length = m_blocks * m_sector_bytes;
			}
			break;

		// READ TOC (GD-ROM ver.)
		case 0x14:
		{
			int start_trk = command[2];// ok?
			int end_trk = cdrom_get_last_track(m_cdrom);
			int length;
			int allocation_length = SCSILengthFromUINT16( &command[ 3 ] );

			if( start_trk == 0 )
			{
				start_trk = 1;
			}
			if( start_trk == 0xaa )
			{
				end_trk = start_trk;
			}

			length = 4 + ( 8 * ( ( end_trk - start_trk ) + 1 ) );
			if( length > allocation_length )
			{
				length = allocation_length;
			}
			else if( length < 4 )
			{
				length = 4;
			}

			if (m_cdda != nullptr)
			{
				m_cdda->stop_audio();
			}

			m_phase = SCSI_PHASE_DATAIN;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			m_transfer_length = length;
			break;
		}

		case 0x70:  // unknown, return no data, always followed by cmd 0x71, command[1] parameter can be 0x1f or 0x9f
			m_phase = SCSI_PHASE_STATUS;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			m_transfer_length = 0;
			break;

		case 0x71:
			m_phase = SCSI_PHASE_DATAIN;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			m_transfer_length = sizeof(GDROM_Cmd71_Reply);
			break;

		case 0x40:
			if(command[1] & 0xf)
			{
				m_phase = SCSI_PHASE_DATAIN;
				m_status_code = SCSI_STATUS_CODE_GOOD;
				m_transfer_length = 0xe;
			}
			else
			{
				printf("command 0x40: unhandled subchannel request\n");
			}
			break;

		default:
			t10mmc::ExecCommand();
			break;
	}
}

// scsicd_read_data
//
// Read data from the device resulting from the execution of a command

void gdrom_device::ReadData( UINT8 *data, int dataLength )
{
	int i;
	UINT8 tmp_buffer[2048];

	switch ( command[0] )
	{
		case 0x11: // REQ_MODE
			printf("REQ_MODE: dataLength %d\n", dataLength);
			memcpy(data, &GDROM_Cmd11_Reply[transferOffset], (dataLength >= 32-transferOffset) ? 32-transferOffset : dataLength);
			break;

		case 0x30: // CD_READ
			logerror("GDROM: read %x dataLength, \n", dataLength);
			if ((m_cdrom) && (m_blocks))
			{
				while (dataLength > 0)
				{
					if (!cdrom_read_data(m_cdrom, m_lba, tmp_buffer, CD_TRACK_MODE1))
					{
						logerror("GDROM: CD read error!\n");
					}

					logerror("True LBA: %d, buffer half: %d\n", m_lba, m_cur_subblock * m_sector_bytes);

					memcpy(data, &tmp_buffer[m_cur_subblock * m_sector_bytes], m_sector_bytes);

					m_cur_subblock++;
					if (m_cur_subblock >= m_num_subblocks)
					{
						m_cur_subblock = 0;

						m_lba++;
						m_blocks--;
					}

					m_last_lba = m_lba;
					dataLength -= m_sector_bytes;
					data += m_sector_bytes;
				}
			}
			break;

		case 0x14: // READ TOC (GD-ROM ver.)
			/*
			    Track numbers are problematic here: 0 = lead-in, 0xaa = lead-out.
			    That makes sense in terms of how real-world CDs are referred to, but
			    our internal routines for tracks use "0" as track 1.  That probably
			    should be fixed...
			*/
			printf("GDROM: READ TOC, format = %d time=%d\n", command[2]&0xf,(command[1]>>1)&1);
			switch (command[2] & 0x0f)
			{
				case 0:     // normal
					{
						int start_trk;
						int end_trk;
						int len;
						int in_len;
						int dptr;
						UINT32 tstart;

						start_trk = command[2];
						if( start_trk == 0 )
						{
							start_trk = 1;
						}

						end_trk = cdrom_get_last_track(m_cdrom);
						len = (end_trk * 8) + 2;

						// the returned TOC DATA LENGTH must be the full amount,
						// regardless of how much we're able to pass back due to in_len
						dptr = 0;
						data[dptr++] = (len>>8) & 0xff;
						data[dptr++] = (len & 0xff);
						data[dptr++] = 1;
						data[dptr++] = end_trk;

						if( start_trk == 0xaa )
						{
							end_trk = 0xaa;
						}

						in_len = command[3]<<8 | command[4];

						for (i = start_trk; i <= end_trk; i++)
						{
							int cdrom_track = i;
							if( cdrom_track != 0xaa )
							{
								cdrom_track--;
							}

							if( dptr >= in_len )
							{
								break;
							}

							data[dptr++] = 0;
							data[dptr++] = cdrom_get_adr_control(m_cdrom, cdrom_track);
							data[dptr++] = i;
							data[dptr++] = 0;

							tstart = cdrom_get_track_start(m_cdrom, cdrom_track);
							if ((command[1]&2)>>1)
								tstart = lba_to_msf(tstart);
							data[dptr++] = (tstart>>24) & 0xff;
							data[dptr++] = (tstart>>16) & 0xff;
							data[dptr++] = (tstart>>8) & 0xff;
							data[dptr++] = (tstart & 0xff);
						}
					}
					break;
				default:
					logerror("GDROM: Unhandled READ TOC format %d\n", command[2]&0xf);
					break;
			}
			break;

		case 0x71:
			memcpy(data, &GDROM_Cmd71_Reply[0], sizeof(GDROM_Cmd71_Reply));
			break;

		case 0x40: // Get Subchannel status
			data[0] = 0; // Reserved
			data[1] = 0x15; // Audio Playback status (todo)
			data[2] = 0;
			data[3] = 0x0e; // header size
			data[4] = 0; // ?
			data[5] = 1; // Track Number
			data[6] = 1; // gap #1
			data[7] = 0; // ?
			data[8] = 0; // ?
			data[9] = 0; // ?
			data[0xa] = 0; // ?
			data[0xb] = 0; // FAD >> 16
			data[0xc] = 0; // FAD >> 8
			data[0xd] = 0x96; // FAD >> 0
			break;

		default:
			t10mmc::ReadData( data, dataLength );
			break;
	}
}

// scsicd_write_data
//
// Write data to the CD-ROM device as part of the execution of a command

void gdrom_device::WriteData( UINT8 *data, int dataLength )
{
	switch (command[ 0 ])
	{
		case 0x12: // SET_MODE
			memcpy(&GDROM_Cmd11_Reply[transferOffset], data, (dataLength >= 32-transferOffset) ? 32-transferOffset : dataLength);
			break;

		default:
			t10mmc::WriteData( data, dataLength );
			break;
	}
}

// device type definition
const device_type GDROM = &device_creator<gdrom_device>;

gdrom_device::gdrom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	atapi_cdrom_device(mconfig, GDROM, "GDROM", tag, owner, clock, "gdrom", __FILE__)
{
}

void gdrom_device::device_start()
{
	save_item(NAME(read_type));
	save_item(NAME(data_select));
	save_item(NAME(transferOffset));

	/// TODO: split identify buffer into another method as device_start() should be called after it's filled in, but the atapi_cdrom_device has it's own.
	atapi_cdrom_device::device_start();

	memset(m_identify_buffer, 0, sizeof(m_identify_buffer));

	m_identify_buffer[0] = 0x8600; // ATAPI device, cmd set 6 compliant, DRQ within 3 ms of PACKET command

	m_identify_buffer[23] = ('S' << 8) | 'E';
	m_identify_buffer[24] = (' ' << 8) | ' ';
	m_identify_buffer[25] = (' ' << 8) | ' ';
	m_identify_buffer[26] = (' ' << 8) | ' ';

	m_identify_buffer[27] = ('C' << 8) | 'D';
	m_identify_buffer[28] = ('-' << 8) | 'R';
	m_identify_buffer[29] = ('O' << 8) | 'M';
	m_identify_buffer[30] = (' ' << 8) | 'D';
	m_identify_buffer[31] = ('R' << 8) | 'I';
	m_identify_buffer[32] = ('V' << 8) | 'E';
	m_identify_buffer[33] = (' ' << 8) | ' ';
	m_identify_buffer[34] = (' ' << 8) | ' ';
	m_identify_buffer[35] = ('6' << 8) | '.';
	m_identify_buffer[36] = ('4' << 8) | '2';
	m_identify_buffer[37] = (' ' << 8) | ' ';
	m_identify_buffer[38] = (' ' << 8) | ' ';
	m_identify_buffer[39] = (' ' << 8) | ' ';
	m_identify_buffer[40] = (' ' << 8) | ' ';
	m_identify_buffer[41] = (' ' << 8) | ' ';
	m_identify_buffer[42] = (' ' << 8) | ' ';
	m_identify_buffer[43] = (' ' << 8) | ' ';
	m_identify_buffer[44] = (' ' << 8) | ' ';
	m_identify_buffer[45] = (' ' << 8) | ' ';
	m_identify_buffer[46] = (' ' << 8) | ' ';

	m_identify_buffer[49] = 0x0400; // IORDY may be disabled

	m_identify_buffer[63]=7; // multi word dma mode 0-2 supported
	m_identify_buffer[64]=1; // PIO mode 3 supported
}

void gdrom_device::process_buffer()
{
	atapi_hle_device::process_buffer();
	m_sector_number = 0x80 | GDROM_PAUSE_STATE; /// HACK: find out when this should be updated
}
