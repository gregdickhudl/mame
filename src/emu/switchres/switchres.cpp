/**************************************************************

   switchres.cpp - SwichRes core routines

   ---------------------------------------------------------

   SwitchRes   Modeline generation engine for emulation
               (C) 2010 Chris Kennedy
               (C) 2013 Antonio Giner

   GroovyMAME  Integration of SwitchRes into the MAME project
               Some reworked patches from SailorSat's CabMAME

 **************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "config.h"
#include "rendutil.h"

#define CUSTOM_VIDEO_TIMING_SYSTEM      0x00000010

//============================================================
//  PROTOTYPES
//============================================================

void set_option(running_machine &machine, const char *option_ID, bool state);
static void switchres_load(running_machine &machine, int config_type, xml_data_node *parentnode);
static void switchres_save(running_machine &machine, int config_type, xml_data_node *parentnode);

//============================================================
//  switchres_get_video_mode
//============================================================

bool switchres_get_video_mode(running_machine &machine)
{
	switchres_manager *switchres = &machine.switchres;
	config_settings *cs = &switchres->cs;
	game_info *game = &switchres->game;
	monitor_range *range = switchres->range;
	modeline *mode;
	modeline *mode_table = switchres->video_modes;
	modeline *best_mode = &switchres->best_mode;
	modeline *user_mode = &switchres->user_mode;
	modeline source_mode, *s_mode = &source_mode;
	modeline target_mode, *t_mode = &target_mode;
	char modeline[256]={'\x00'};
	char result[256]={'\x00'};
	int i = 0, j = 0, table_size = 0;

	cs->effective_orientation = effective_orientation(machine);

	osd_printf_verbose("SwitchRes: v%s:[%s] Calculating best video mode for %dx%d@%.6f orientation: %s\n",
						SWITCHRES_VERSION, game->name, game->width, game->height, game->refresh,
						cs->effective_orientation?"rotated":"normal");

	memset(best_mode, 0, sizeof(struct modeline));
	best_mode->result.weight |= R_OUT_OF_RANGE;
	s_mode->hactive = game->vector?1:normalize(game->width, 8);
	s_mode->vactive = game->vector?1:game->height;
	s_mode->vfreq = game->refresh;

	if (user_mode->hactive)
	{
		table_size = 1;
		mode = user_mode;
	}
	else
	{
		i = 1;
		table_size = MAX_MODELINES;
		mode = &mode_table[i];
	}

	while (mode->width && i < table_size)
	{
		// apply options to mode type
		if (!cs->modeline_generation)
			mode->type &= ~XYV_EDITABLE;

		if (cs->refresh_dont_care)
			mode->type |= V_FREQ_EDITABLE;
		
		if (cs->lock_system_modes && (mode->type & CUSTOM_VIDEO_TIMING_SYSTEM) && !(mode->type & MODE_DESKTOP) && !(mode->type & MODE_USER_DEF))
			mode->type |= MODE_DISABLED;

		osd_printf_verbose("\nSwitchRes: %s%4d%sx%s%4d%s_%s%d=%.4fHz%s%s\n",
			mode->type & X_RES_EDITABLE?"(":"[", mode->width, mode->type & X_RES_EDITABLE?")":"]",
			mode->type & Y_RES_EDITABLE?"(":"[", mode->height, mode->type & Y_RES_EDITABLE?")":"]",
			mode->type & V_FREQ_EDITABLE?"(":"[", mode->refresh, mode->vfreq, mode->type & V_FREQ_EDITABLE?")":"]",
			mode->type & MODE_DISABLED?" - locked":"");

		// now get the mode if allowed
		if (!(mode->type & MODE_DISABLED))
		{
			for (j = 0 ; j < MAX_RANGES ; j++)
			{
				if (range[j].hfreq_min)
				{
					memcpy(t_mode, mode, sizeof(struct modeline));
					modeline_create(s_mode, t_mode, &range[j], cs);
					t_mode->range = j;

					osd_printf_verbose("%s\n", modeline_result(t_mode, result));

					if (modeline_compare(t_mode, best_mode))
						memcpy(best_mode, t_mode, sizeof(struct modeline));
				}
			}
		}
		mode++;
		i++;
	}

	if (best_mode->result.weight & R_OUT_OF_RANGE)
	{
		osd_printf_error("SwitchRes: could not find a video mode that meets your specs\n");
		return false;
	}

	osd_printf_info("\nSwitchRes: [%s] (%d) %s (%dx%d@%.2f)->(%dx%d@%.2f)\n", game->name, game->screens, game->orientation?"vertical":"horizontal",
		game->width, game->height, game->refresh, best_mode->hactive, best_mode->vactive, best_mode->vfreq);

	osd_printf_verbose("%s\n", modeline_result(best_mode, result));
	if (cs->modeline_generation)
		osd_printf_verbose("SwitchRes: Modeline %s\n", modeline_print(best_mode, modeline, MS_FULL));

	return true;
}

//============================================================
//  switchres_get_monitor_specs
//============================================================

int switchres_get_monitor_specs(running_machine &machine)
{
	switchres_manager *switchres = &machine.switchres;
	char default_monitor[] = "generic_15";

	memset(&switchres->range[0], 0, sizeof(struct monitor_range) * MAX_RANGES);

	if (!strcmp(switchres->cs.monitor, "custom"))
	{
		monitor_fill_range(&switchres->range[0],machine.options().crt_range0());
		monitor_fill_range(&switchres->range[1],machine.options().crt_range1());
		monitor_fill_range(&switchres->range[2],machine.options().crt_range2());
		monitor_fill_range(&switchres->range[3],machine.options().crt_range3());
		monitor_fill_range(&switchres->range[4],machine.options().crt_range4());
		monitor_fill_range(&switchres->range[5],machine.options().crt_range5());
		monitor_fill_range(&switchres->range[6],machine.options().crt_range6());
		monitor_fill_range(&switchres->range[7],machine.options().crt_range7());
		monitor_fill_range(&switchres->range[8],machine.options().crt_range8());
		monitor_fill_range(&switchres->range[9],machine.options().crt_range9());
	}
	else if (!strcmp(switchres->cs.monitor, "lcd"))
		monitor_fill_lcd_range(&switchres->range[0],machine.options().lcd_range());

	else if (monitor_set_preset(switchres->cs.monitor, switchres->range) == 0)
		monitor_set_preset(default_monitor, switchres->range);

	return 0;
}

//============================================================
//  switchres_init
//============================================================

void switchres_init(running_machine &machine)
{
	emu_options &options = machine.options();
	config_settings *cs = &machine.switchres.cs;
	modeline *user_mode = &machine.switchres.user_mode;

	osd_printf_verbose("SwitchRes: v%s, Monitor: %s, Orientation: %s, Modeline generation: %s\n",
		SWITCHRES_VERSION, options.monitor(), options.orientation(), options.modeline_generation()?"enabled":"disabled");

	// Get user defined modeline
	if (options.modeline_generation())
	{
		modeline_parse(options.modeline(), user_mode);
		user_mode->type |= MODE_USER_DEF;
	}

	// Register for configuration
	config_register(machine, "switchres", config_saveload_delegate(FUNC(switchres_load), &machine), config_saveload_delegate(FUNC(switchres_save), &machine));

	// Get monitor specs
	sprintf(cs->monitor, "%s", options.monitor());
	sprintf(cs->connector, "%s", options.connector());
	for (int i = 0; cs->monitor[i]; i++) cs->monitor[i] = tolower(cs->monitor[i]);
	if (user_mode->hactive)
	{
		modeline_to_monitor_range(machine.switchres.range, user_mode);
		monitor_show_range(machine.switchres.range);
	}
	else
		switchres_get_monitor_specs(machine);

	// Get rest of config options
	cs->modeline_generation = options.modeline_generation();
	cs->doublescan = options.doublescan();
	cs->interlace = options.interlace();
	cs->lock_system_modes = options.lock_system_modes();
	cs->lock_unsupported_modes = options.lock_unsupported_modes();
	cs->refresh_dont_care = options.refresh_dont_care();
	cs->cleanstretch = options.cleanstretch();
	sscanf(options.sync_refresh_tolerance(), "%f", &cs->sync_refresh_tolerance);
	float pclock_min;
	sscanf(options.dotclock_min(), "%f", &pclock_min);
	cs->pclock_min = pclock_min * 1000000;
}

//============================================================
//  switchres_get_game_info
//============================================================

void switchres_get_game_info(running_machine &machine)
{
	emu_options &options = machine.options();
	game_info *game = &machine.switchres.game;
	const game_driver *game_drv = &machine.system();
	const screen_device *devconfig;

	// Get game information
	sprintf(game->name, "%s", options.system_name());
	if (game->name[0] == 0) sprintf(game->name, "empty");

	machine_config config(*game_drv, options);
	devconfig = config.first_screen();

	// Fill in current video mode settings
	game->orientation = effective_orientation(machine);

	if (devconfig->screen_type() == SCREEN_TYPE_VECTOR)
	{
		game->vector = 1;
		game->width = 640;
		game->height = 480;
	}

	// Output width and height only for games that are not vector
	else
	{
		const rectangle &visarea = devconfig->visible_area();
		int w = visarea.max_x - visarea.min_x + 1;
		int h = visarea.max_y - visarea.min_y + 1;
		game->width = game->orientation?h:w;
		game->height = game->orientation?w:h;
	}

	game->refresh = ATTOSECONDS_TO_HZ(devconfig->refresh_attoseconds());

	// Check for multiple screens
	screen_device_iterator iter(config.root_device());
	for (devconfig = iter.first(); devconfig != NULL; devconfig = iter.next())
		game->screens++;
}

//============================================================
//  effective_orientation
//============================================================

bool effective_orientation(running_machine &machine)
{
	config_settings *cs = &machine.switchres.cs;
	const game_driver *game = &machine.system();
	emu_options &options = machine.options();
	render_target *target = machine.render().first_target();
	bool game_orientation = ((game->flags & ORIENTATION_MASK) & ORIENTATION_SWAP_XY);

	if (target)
		cs->monitor_orientation = ((target->orientation() & ORIENTATION_MASK) & ORIENTATION_SWAP_XY? 1:0) ^ cs->desktop_rotated;
	else if (!strcmp(options.orientation(), "horizontal"))
		cs->monitor_orientation = 0;
	else if (!strcmp(options.orientation(), "vertical"))
		cs->monitor_orientation = 1;
	else if (!strcmp(options.orientation(), "rotate") || !strcmp(options.orientation(), "rotate_r"))
	{
		cs->monitor_orientation = game_orientation;
		cs->monitor_rotates_cw = 0;
	}
	else if (!strcmp(options.orientation(), "rotate_l"))
	{
		cs->monitor_orientation = game_orientation;
		cs->monitor_rotates_cw = 1;
	}

	return game_orientation ^ cs->monitor_orientation;
}

//============================================================
//  switchres_get_scale
//============================================================

bool switchres_get_scale(render_target *target, INT32 target_width, INT32 target_height, float &width, float &height, float &xscale, float &yscale)
{
	INT32 iwidth, iheight;
	config_settings *cs = &target->manager().machine().switchres.cs;
	game_info *game = &target->manager().machine().switchres.game;
	modeline *mode = &target->manager().machine().switchres.best_mode;

	target->compute_minimum_size(iwidth, iheight);
	xscale = yscale = 1.0f;

	// We only apply cleanstretch to window #0
	if (target->index() == 0 && cs->cleanstretch && mode->hactive && !(mode->result.weight & R_RES_STRETCH) && !game->vector)
	{
		switch(cs->cleanstretch)
		{
			case 1:
			default:
			{
				width = iwidth;
				height = iheight;

				xscale = cs->desktop_rotated? mode->result.y_scale : mode->result.x_scale;
				yscale = cs->desktop_rotated? mode->result.x_scale : mode->result.y_scale;
				break;
			}

			case 2:
			{
				width = cs->desktop_rotated? iwidth: target_width;
				height = cs->desktop_rotated? target_height : iheight;

				float xscale_frac = (cs->effective_orientation? (1.0 / (STANDARD_CRT_ASPECT)) : (STANDARD_CRT_ASPECT)) / cs->monitor_aspect;

				xscale = cs->desktop_rotated? mode->result.y_scale : xscale_frac;
				yscale = cs->desktop_rotated? xscale_frac : mode->result.y_scale;
				break;
			}
		}
		return true;
	}
	return false;
}

//============================================================
//  switchres_check_resolution_change
//============================================================

void switchres_check_resolution_change(running_machine &machine, int curr_width, int curr_height)
{
	game_info *game = &machine.switchres.game;
	config_settings *cs = &machine.switchres.cs;
	bool curr_orientation = effective_orientation(machine);
	float curr_vfreq = game->refresh;

	screen_device_iterator scriter(machine.root_device());
	if (scriter.count())
	{
		screen_device *screen = scriter.first();
		if (screen-> frame_number())
			curr_vfreq = ATTOSECONDS_TO_HZ(screen->frame_period().m_attoseconds);
	}

	if (cs->desktop_rotated)
	{
		int temp = curr_width;
		curr_width = curr_height;
		curr_height = temp;
	}

	if (game->width != curr_width || game->height != curr_height || curr_vfreq != game->refresh || cs->effective_orientation != curr_orientation)
	{
		osd_printf_verbose("SwitchRes: Resolution change from %dx%d@%f %s to %dx%d@%f %s\n",
			game->width, game->height, game->refresh, cs->effective_orientation?"rotated":"normal", curr_width, curr_height, curr_vfreq, curr_orientation?"rotated":"normal");

		game->width = curr_width;
		game->height = curr_height;
		game->refresh = curr_vfreq;
		game->changeres = 1;
	}
}

//============================================================
//  switchres_set_options
//============================================================

void switchres_set_options(running_machine &machine)
{
	config_settings *cs = &machine.switchres.cs;
	bool native_orientation = ((machine.system().flags & ORIENTATION_MASK) & ORIENTATION_SWAP_XY);
	bool must_rotate = effective_orientation(machine) ^ cs->desktop_rotated;

	// Set rotation options
	set_option(machine, OPTION_ROTATE, true);
	if (cs->monitor_rotates_cw)
	{
		set_option(machine, OPTION_ROL, (!native_orientation & must_rotate));
		set_option(machine, OPTION_AUTOROL, !must_rotate);
		set_option(machine, OPTION_ROR, false);
		set_option(machine, OPTION_AUTOROR, false);
	}
	else
	{
		set_option(machine, OPTION_ROR, (!native_orientation & must_rotate));
		set_option(machine, OPTION_AUTOROR, !must_rotate);
		set_option(machine, OPTION_ROL, false);
		set_option(machine, OPTION_AUTOROL, false);
	}
}

//============================================================
//  switchres_reset_options
//============================================================

void switchres_reset_options(running_machine &machine)
{
	// Reset MAME common options
	machine.options().revert(OPTION_PRIORITY_SWITCHRES);
}

//============================================================
// set_option - option setting wrapper
//============================================================

void set_option(running_machine &machine, const char *option_ID, bool state)
{
	emu_options &options = machine.options();
	std::string error_string;

	options.set_value(option_ID, state, OPTION_PRIORITY_SWITCHRES, error_string);
	osd_printf_verbose("SwitchRes: Setting option -%s%s\n", state?"":"no", option_ID);
}

//============================================================
// switchres_load
//============================================================

static void switchres_load(running_machine &machine, int config_type, xml_data_node *parentnode)
{
	// We don't do anything here yet.
}

//============================================================
// switchres_save
//============================================================

static void switchres_save(running_machine &machine, int config_type, xml_data_node *parentnode)
{
	modeline *mode = &machine.switchres.best_mode;
	char modeline_txt[256]={'\x00'};

	// only care about game-specific data
	if (config_type != CONFIG_TYPE_GAME)
		return;

	xml_data_node *switchresnode = xml_add_child(parentnode, "switchres", NULL);
	if (switchresnode != NULL)
	{
		if (mode && mode->htotal)
		{
			modeline_print(mode, modeline_txt, MS_PARAMS);
			xml_set_attribute(switchresnode, "modeline", modeline_txt);
		}
	}
}
