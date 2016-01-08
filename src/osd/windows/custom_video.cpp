/**************************************************************

   custom_video.cpp - Custom video library

   ---------------------------------------------------------

   SwitchRes   Modeline generation engine for emulation
               (C) 2010 Chris Kennedy
               (C) 2015 Antonio Giner

   GroovyMAME  Integration of SwitchRes into the MAME project
               Some reworked patches from SailorSat's CabMAME

 **************************************************************/

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "emu.h"
#include "custom_video.h"
#include "custom_video_ati.h"
#include "custom_video_adl.h"
#include "custom_video_pstrip.h"

extern bool ati_is_legacy(int vendor, int device);

//============================================================
//  LOCAL VARIABLES
//============================================================

static int custom_method;
static modeline m_user_mode;
static modeline m_backup_mode;
static char m_device_name[32];
static char m_device_key[128];
static char ps_timing[256];

//============================================================
//  custom_video_init
//============================================================

bool custom_video_init(char *device_name, char *device_id, modeline *desktop_mode, modeline *user_mode, int method, char *s_param)
{
	memset(&m_backup_mode, 0, sizeof(struct modeline));
	memcpy(&m_user_mode, user_mode, sizeof(modeline));
	memcpy(m_device_name, device_name, sizeof(m_device_name));

	if ((method == CUSTOM_VIDEO_TIMING_POWERSTRIP) && ps_init(ps_monitor_index(m_device_name), &m_backup_mode))
	{
		custom_method = CUSTOM_VIDEO_TIMING_POWERSTRIP;
		m_backup_mode.type |= CUSTOM_VIDEO_TIMING_POWERSTRIP;

		// If we have a -ps_timing string defined, use it as user defined modeline
		memcpy(ps_timing, s_param, sizeof(ps_timing));
		custom_video_parse_timing(ps_timing, &m_user_mode);
		return true;
	}
	else
	{
		int vendor, device;
		custom_video_parse_pci_id(device_id, &vendor, &device);

		if (vendor == 0x1002) // ATI/AMD
		{
			if (ati_is_legacy(vendor, device))
			{
				memcpy(m_device_key, s_param, sizeof(m_device_key));
				if (ati_init(m_device_name, m_device_key, device_id))
				{
					custom_method = CUSTOM_VIDEO_TIMING_ATI_LEGACY;
					return true;
				}
			}
			else
			{
				if (adl_init())
				{
					custom_method = CUSTOM_VIDEO_TIMING_ATI_ADL;
					return true;
				}
			}
		}
		else
			osd_printf_info("Video chipset is not compatible.\n");
	}

	return false;
}

//============================================================
//  custom_video_close
//============================================================

void custom_video_close()
{
	switch (custom_method)
	{
		case CUSTOM_VIDEO_TIMING_ATI_LEGACY:
			break;
		
		case CUSTOM_VIDEO_TIMING_ATI_ADL:
			adl_close();
			break;

		case CUSTOM_VIDEO_TIMING_POWERSTRIP:
			break;
	}
}

//============================================================
//  custom_video_get_timing
//============================================================

bool custom_video_get_timing(modeline *mode)
{
	char modeline_txt[256]={'\x00'};

	switch (custom_method)
	{
		case CUSTOM_VIDEO_TIMING_ATI_LEGACY:
			if (ati_get_modeline(mode))
			{
				osd_printf_verbose("ATI legacy timing %s\n", modeline_print(mode, modeline_txt, MS_FULL));
				mode->type |= CUSTOM_VIDEO_TIMING_ATI_LEGACY | (!(mode->type & MODE_DESKTOP)? V_FREQ_EDITABLE | (mode->width == DUMMY_WIDTH? X_RES_EDITABLE:0):0);
				return true;
			}
			break;
		
		case CUSTOM_VIDEO_TIMING_ATI_ADL:
			if (adl_get_modeline(m_device_name, mode))
			{
				osd_printf_verbose("ATI ADL timing %s\n", modeline_print(mode, modeline_txt, MS_FULL));
				mode->type |= CUSTOM_VIDEO_TIMING_ATI_ADL | (!(mode->type & MODE_DESKTOP)? V_FREQ_EDITABLE :0);
				return true;
			}
			break;

		case CUSTOM_VIDEO_TIMING_POWERSTRIP:
			if (ps_get_modeline(ps_monitor_index(m_device_name), mode))
			{
				osd_printf_verbose("Powerstrip timing %s\n", modeline_print(mode, modeline_txt, MS_FULL));
				mode->type |= CUSTOM_VIDEO_TIMING_POWERSTRIP | V_FREQ_EDITABLE;
				return true;
			}
			break;

		default:
			osd_printf_verbose("system mode\n");
			mode->type |= CUSTOM_VIDEO_TIMING_SYSTEM;
			break;
	}

	return false;
}

//============================================================
//  custom_video_set_timing
//============================================================

bool custom_video_set_timing(modeline *mode)
{
	char modeline_txt[256]={'\x00'};
	
	switch (custom_method)
	{
		case CUSTOM_VIDEO_TIMING_ATI_LEGACY:
			if (ati_set_modeline(mode))
			{
				osd_printf_verbose("ATI legacy timing %s\n", modeline_print(mode, modeline_txt, MS_FULL));
				return true;
			}
			break;
		
		case CUSTOM_VIDEO_TIMING_ATI_ADL:
			if (adl_set_modeline(m_device_name, mode, MODELINE_UPDATE))
			{
				osd_printf_verbose("ATI ADL timing %s\n", modeline_print(mode, modeline_txt, MS_FULL));
				return true;
			}
			break;

		case CUSTOM_VIDEO_TIMING_POWERSTRIP:
			// In case -ps_timing is provided, pass it as raw string
			if (m_user_mode.type & CUSTOM_VIDEO_TIMING_POWERSTRIP)
				ps_set_monitor_timing_string(ps_monitor_index(m_device_name), (char*)ps_timing);
			// Otherwise pass it as modeline
			else
				ps_set_modeline(ps_monitor_index(m_device_name), mode);
			
			osd_printf_verbose("Powerstrip timing %s\n", modeline_print(mode, modeline_txt, MS_FULL));
			Sleep(100);
			return true;
			break;

		default:
			break;
	}
	return false;
}

//============================================================
//  custom_video_restore_timing
//============================================================

bool custom_video_restore_timing()
{
	if (!m_backup_mode.hactive)
		return false;

	// Restore backup mode
	return custom_video_update_timing(0);
}

//============================================================
//  custom_video_refresh_timing
//============================================================

void custom_video_refresh_timing()
{
	switch (custom_method)
	{
		case CUSTOM_VIDEO_TIMING_ATI_LEGACY:
			ati_refresh_timings();
			break;
		
		case CUSTOM_VIDEO_TIMING_ATI_ADL:
			break;

		case CUSTOM_VIDEO_TIMING_POWERSTRIP:
			break;
	}
}

//============================================================
//  custom_video_update_timing
//============================================================

bool custom_video_update_timing(modeline *mode)
{
	// Restore old video timing
	if (m_backup_mode.hactive)
	{
		osd_printf_verbose("Switchres: restoring ");
		custom_video_set_timing(&m_backup_mode);
	}

	// Update with new video timing
	if (mode)
	{
		// Backup current timing
		m_backup_mode.width = mode->width;
		m_backup_mode.height = mode->height;
		m_backup_mode.refresh = mode->refresh;
		m_backup_mode.interlace = mode->interlace;
		osd_printf_verbose("Switchres: saving    ");
		custom_video_get_timing(&m_backup_mode);

		// Apply new timing now
		osd_printf_verbose("Switchres: updating  ");
		custom_video_set_timing(mode);
	}
	custom_video_refresh_timing();
	return false;
}

//============================================================
//  custom_video_parse_timing
//============================================================

bool custom_video_parse_timing(char *timing_string, modeline *user_mode)
{
	char modeline_txt[256]={'\x00'};

	if (!strcmp(timing_string, "auto"))
		return false;

	if (strstr(timing_string, "="))
	{
		// Powerstrip timing string
		MonitorTiming timing;
		ps_read_timing_string(timing_string, &timing);
		ps_pstiming_to_modeline(&timing, user_mode);
		user_mode->type |= CUSTOM_VIDEO_TIMING_POWERSTRIP;
		osd_printf_verbose("SwitchRes: ps_string: %s (%s)\n", timing_string, modeline_print(user_mode, modeline_txt, MS_PARAMS));
	}
	else
	{
		// Normal modeline
		modeline_parse(timing_string, user_mode);
		osd_printf_verbose("SwitchRes: modeline: %s \n", modeline_print(user_mode, modeline_txt, MS_PARAMS));
	}

	return true;
}

//============================================================
//  custom_video_parse_pci_id
//============================================================

int custom_video_parse_pci_id(char *device_id, int *vendor, int *device)
{
	return sscanf(device_id, "PCI\\VEN_%x&DEV_%x", vendor, device);
}

//============================================================
//  custom_get_backup_mode
//============================================================

modeline *custom_video_get_backup_mode()
{
	return &m_backup_mode;
}
