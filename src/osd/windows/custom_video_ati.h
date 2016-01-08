/**************************************************************

	custom_video_ati.h - ATI legacy library header

	---------------------------------------------------------

	SwitchRes	Modeline generation engine for emulation
				(C) 2010 Chris Kennedy
				(C) 2015 Antonio Giner

	GroovyMAME	Integration of SwitchRes into the MAME project
				Some reworked patches from SailorSat's CabMAME

 **************************************************************/

bool ati_init(char *device_name, char *device_key, char *device_id);
bool ati_get_modeline(modeline *mode);
bool ati_set_modeline(modeline *mode);
void ati_refresh_timings(void);
