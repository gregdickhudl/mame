/**************************************************************

   switchres_proto.h - SwichRes prototypes header

   ---------------------------------------------------------

   SwitchRes   Modeline generation engine for emulation
               (C) 2010 Chris Kennedy
               (C) 2015 Antonio Giner

   GroovyMAME  Integration of SwitchRes into the MAME project
               Some reworked patches from SailorSat's CabMAME

 **************************************************************/

#ifndef __SWITCHRES_H_PROTO__
#define __SWITCHRES_H_PROTO__

//============================================================
//  PROTOTYPES
//============================================================

// util.cpp
int normalize(int a, int b);
int real_res(int x);

// switchres.cpp
bool switchres_get_video_mode(running_machine &machine);
int switchres_get_monitor_specs(running_machine &machine);
void switchres_init(running_machine &machine);
void switchres_get_game_info(running_machine &machine);
bool switchres_get_scale(render_target *target, INT32 target_width, INT32 target_height, float &width, float &height, float &xscale, float &yscale);
void switchres_check_resolution_change(running_machine &machine, int width, int height);
void switchres_set_options(running_machine &machine);
void switchres_reset_options(running_machine &machine);
bool effective_orientation(running_machine &machine);

// OSD - switchres.cpp
bool switchres_init_osd(running_machine &machine);
bool switchres_modeline_setup(running_machine &machine);
bool switchres_modeline_remove(running_machine &machine);

#endif
