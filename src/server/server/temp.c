/***************************************************************
  Real game_load function.
 ***************************************************************/
static void game_load_internal(struct section_file *file)
{
	int i, k;
	enum server_states tmp_server_state;
	RANDOM_STATE rstate;
	const char *string;
	int improvement_order_size = 0;
	int technology_order_size = 0;
	int civstyle = 0;
	char *scen_text;
	char **improvement_order = NULL;
	char **technology_order = NULL;
	enum tile_special_type *special_order = NULL;
	struct base_type **base_order = NULL;
	int num_base_types = 0;
	char *savefile_options = secfile_lookup_str(file, "savefile.options");

	/* [savefile] */

	if (has_capability("improvement_order", savefile_options)) 
	{
		improvement_order = secfile_lookup_str_vec(file, &improvement_order_size,
				"savefile.improvement_order");
	}
	if (has_capability("technology_order", savefile_options)) 
	{
		technology_order = secfile_lookup_str_vec(file, &technology_order_size,
				"savefile.technology_order");
	}
	if (has_capability("resources", savefile_options)) 
	{
		char **modname;
		int nmod;
		enum tile_special_type j;

		modname = secfile_lookup_str_vec(file, &nmod, "savefile.specials");
		/* make sure that the size of the array is divisible by 4 */
		special_order = fc_calloc(nmod + (4 - (nmod % 4)), sizeof(*special_order));
		for (j = 0; j < nmod; j++) 
		{
			special_order[j] = find_special_by_rule_name(modname[j]);
		}
		fc_free(modname);
		for (; j < S_LAST + (4 - (S_LAST % 4)); j++) 
		{
			special_order[j] = S_LAST;
		}
	}

	/* [scenario] */
	scen_text = secfile_lookup_str_default(file, "", "scenario.name");
	if (scen_text[0] != '\0') 
	{
		game.scenario.is_scenario = TRUE;
		sz_strlcpy(game.scenario.name, scen_text);
		scen_text = secfile_lookup_str_default(file, "",
				"scenarion.description");
		if (scen_text[0] != '\0') 
		{
			sz_strlcpy(game.scenario.description, scen_text);
		}
		else 
		{
			game.scenario.description[0] = '\0';
		}
		game.scenario.players
			= secfile_lookup_bool_default(file, TRUE, "scenario.save_players");
	}
	else 
	{
		game.scenario.is_scenario = FALSE;
	}


	tmp_server_state = server_states_invalid();
	if (section_file_lookup(file, "game.server_state")) 
	{
		string = secfile_lookup_str_int(file, (int *) &tmp_server_state,
				"game.server_state");
		if (NULL != string) 
		{
			/* new in 2.2: server_state as string; see srv_main.h */
			tmp_server_state = server_states_by_name(string, strcmp);
		}
	}
	if (!server_states_is_valid(tmp_server_state)) 
	{
		tmp_server_state = S_S_RUNNING;
	}

	///////////////////////////////////////////////

	{
		set_meta_patches_string(secfile_lookup_str_default(file, 
					default_meta_patches_string(),
					"game.metapatches"));
		game.server.meta_info.user_message_set =
			secfile_lookup_bool_default(file, FALSE, "game.user_metamessage");
		if (game.server.meta_info.user_message_set) 
		{
			set_user_meta_message_string(secfile_lookup_str_default(file, 
						default_meta_message_string(),
						"game.metamessage"));
		}
		else 
		{
			/* To avoid warnings when loading pre-2.1 savegames */
			secfile_lookup_str_default(file, "", "game.metamessage");
		}

		sz_strlcpy(srvarg.metaserver_addr,
				secfile_lookup_str_default(file, DEFAULT_META_SERVER_ADDR,
					"game.metaserver"));
		sz_strlcpy(srvarg.serverid,
				secfile_lookup_str_default(file, "", "game.serverid"));

	///////////////////////////////////////////////

		game.info.gold          = secfile_lookup_int(file, "game.gold");
		game.info.tech          = secfile_lookup_int(file, "game.tech");
		game.info.skill_level   = secfile_lookup_int(file, "game.skill_level");
		if (game.info.skill_level==0)
			game.info.skill_level = GAME_OLD_DEFAULT_SKILL_LEVEL;

		game.info.timeout       = secfile_lookup_int(file, "game.timeout");
		game.server.timeoutint =
			secfile_lookup_int_default(file, GAME_DEFAULT_TIMEOUTINT,
					"game.timeoutint");
		game.server.timeoutintinc =
			secfile_lookup_int_default(file, GAME_DEFAULT_TIMEOUTINTINC,
					"game.timeoutintinc");
		game.server.timeoutinc =
			secfile_lookup_int_default(file, GAME_DEFAULT_TIMEOUTINC,
					"game.timeoutinc");
		game.server.timeoutincmult =
			secfile_lookup_int_default(file, GAME_DEFAULT_TIMEOUTINCMULT,
					"game.timeoutincmult");
		game.server.timeoutcounter =
			secfile_lookup_int_default(file, 1, "game.timeoutcounter");

		game.server.timeoutaddenemymove
			= secfile_lookup_int_default(file, game.server.timeoutaddenemymove,
					"game.timeoutaddenemymove");

		game.info.end_turn      = secfile_lookup_int_default(file, 5000,
				"game.end_turn");
		game.info.shieldbox
			= secfile_lookup_int_default(file, GAME_DEFAULT_SHIELDBOX,
					"game.box_shield");
		game.info.sciencebox
			= secfile_lookup_int_default(file, 0, "game.box_science");
		if (game.info.sciencebox == 0) 
		{
			/* Researchcost was used for 2.0 and earlier servers. */
			game.info.sciencebox
				= 5 * secfile_lookup_int_default(file, 0, "game.researchcost");
			if (game.info.sciencebox == 0) 
			{
				/* With even earlier servers (?) techlevel was used for this info. */
				game.info.sciencebox = 5 * secfile_lookup_int(file, "game.techlevel");
			}
		}

		game.info.year          = secfile_lookup_int(file, "game.year");
		game.info.year_0_hack   = secfile_lookup_bool_default(file, FALSE,
				"game.year_0_hack");

		if (has_capability("turn", savefile_options)) 
		{
			game.info.turn = secfile_lookup_int(file, "game.turn");
		}
		else 
		{
			game.info.turn = 0;
		}

		if (section_file_lookup(file, "game.simultaneous_phases_now")) 
		{
			bool sp_now;

			sp_now = secfile_lookup_bool(file, "game.simultaneous_phases_now");
			game.info.phase_mode = (sp_now ? PMT_CONCURRENT
					: PMT_PLAYERS_ALTERNATE);

		}
		else 
		{
			game.info.phase_mode = GAME_DEFAULT_PHASE_MODE;
		}

		if (section_file_lookup(file, "game.simultaneous_phases_stored")) 
		{
			bool sp_stored;

			sp_stored
				= secfile_lookup_bool(file, "game.simultaneous_phases_stored");
			game.server.phase_mode_stored = (sp_stored ? PMT_CONCURRENT
					: PMT_PLAYERS_ALTERNATE);
		}
		else 
		{
			game.server.phase_mode_stored = game.info.phase_mode;
		}

		game.info.phase_mode
			= secfile_lookup_int_default(file, game.info.phase_mode,
					"game.phase_mode");
		game.server.phase_mode_stored
			= secfile_lookup_int_default(file, game.server.phase_mode_stored,
					"game.phase_mode_stored");

		game.info.min_players   = secfile_lookup_int(file, "game.min_players");
		game.info.max_players   = secfile_lookup_int(file, "game.max_players");

		game.info.heating = secfile_lookup_int_default(file, 0, "game.heating");
		game.info.globalwarming = secfile_lookup_int(file, "game.globalwarming");
		game.info.warminglevel  = secfile_lookup_int(file, "game.warminglevel");
		game.info.nuclearwinter = secfile_lookup_int_default(file, 0, "game.nuclearwinter");
		game.info.cooling = secfile_lookup_int_default(file, 0, "game.cooling");
		game.info.coolinglevel = secfile_lookup_int_default(file, 8, "game.coolinglevel");
		game.info.notradesize =
			secfile_lookup_int_default(file, GAME_DEFAULT_NOTRADESIZE, "game.notradesize");
		game.info.fulltradesize =
			secfile_lookup_int_default(file, GAME_DEFAULT_FULLTRADESIZE, "game.fulltradesize");
		game.info.trademindist =
			secfile_lookup_int_default(file, GAME_DEFAULT_TRADEMINDIST, "game.trademindist");
		game.info.angrycitizen =
			secfile_lookup_bool_default(file, GAME_DEFAULT_ANGRYCITIZEN, "game.angrycitizen");
		game.info.citymindist =
			secfile_lookup_int_default(file, GAME_DEFAULT_CITYMINDIST, "game.citymindist");
		game.info.rapturedelay =
			secfile_lookup_int_default(file, GAME_DEFAULT_RAPTUREDELAY, "game.rapturedelay");
		game.info.diplcost =
			secfile_lookup_int_default(file, GAME_DEFAULT_DIPLCOST, "game.diplcost");
		game.info.freecost =
			secfile_lookup_int_default(file, GAME_DEFAULT_FREECOST, "game.freecost");
		game.info.conquercost =
			secfile_lookup_int_default(file, GAME_DEFAULT_CONQUERCOST, "game.conquercost");

		game.info.foodbox = secfile_lookup_int_default(file, 0, "game.box_food");
		if (game.info.foodbox == 0) 
		{
			/* foodbox was used for 2.0 and earlier servers. */
			game.info.foodbox = 10 * secfile_lookup_int_default(file, 100, "game.foodbox");
		}
		game.info.techpenalty =
			secfile_lookup_int_default(file, GAME_DEFAULT_TECHPENALTY, "game.techpenalty");
		game.info.razechance =
			secfile_lookup_int_default(file, GAME_DEFAULT_RAZECHANCE, "game.razechance");

		civstyle = secfile_lookup_int_default(file, 2, "game.civstyle");
		game.info.save_nturns =
			secfile_lookup_int_default(file, GAME_DEFAULT_SAVETURNS, "game.save_nturns");

		/* suppress warnings about unused entries in old savegames: */
		(void) section_file_lookup(file, "game.rail_food");
		(void) section_file_lookup(file, "game.rail_prod");
		(void) section_file_lookup(file, "game.rail_trade");
		(void) section_file_lookup(file, "game.farmfood");

		/* National borders setting. */
		game.info.borders = secfile_lookup_int_default(file, 0, "game.borders");
		if (game.info.borders > GAME_MAX_BORDERS) 
		{
			game.info.borders = 1;
		}
		game.info.happyborders = secfile_lookup_bool_default(file, FALSE, 
				"game.happyborders");

		/* Diplomacy. */
		game.info.diplomacy = secfile_lookup_int_default(file, GAME_DEFAULT_DIPLOMACY, 
				"game.diplomacy");

		sz_strlcpy(game.server.save_name,
				secfile_lookup_str_default(file, GAME_DEFAULT_SAVE_NAME,
					"game.save_name"));
		game.info.save_compress_level
			= secfile_lookup_int_default(file, GAME_DEFAULT_COMPRESS_LEVEL,
					"game.save_compress_level");
		game.info.save_compress_type
			= secfile_lookup_int_default(file, GAME_DEFAULT_COMPRESS_TYPE,
					"game.save_compress_type");

		game.info.aifill = secfile_lookup_int_default(file, 0, "game.aifill");

		game.server.scorelog = secfile_lookup_bool_default(file, FALSE,
				"game.scorelog");
		game.server.scoreturn =
			secfile_lookup_int_default(file,
					game.info.turn + GAME_DEFAULT_SCORETURN,
					"game.scoreturn");
		sz_strlcpy(server.game_identifier,
				secfile_lookup_str_default(file, "", "game.id"));
		/* We are not checking game_identifier legality just yet.
		 * That's done when we are sure that rand seed has been initialized,
		 * so that we can generate new game_identifier, if needed. */

		game.info.fogofwar = secfile_lookup_bool_default(file, FALSE, "game.fogofwar");
		game.server.fogofwar_old = game.info.fogofwar;

		game.server.foggedborders
			= secfile_lookup_bool_default(file, GAME_DEFAULT_FOGGEDBORDERS,
					"game.foggedborders");

		game.info.civilwarsize =
			secfile_lookup_int_default(file, GAME_DEFAULT_CIVILWARSIZE,
					"game.civilwarsize");
		game.info.contactturns =
			secfile_lookup_int_default(file, GAME_DEFAULT_CONTACTTURNS,
					"game.contactturns");

		if(has_capability("diplchance_percent", savefile_options)) 
		{
			game.info.diplchance = secfile_lookup_int_default(file, game.info.diplchance,
					"game.diplchance");
		}
		else 
		{
			game.info.diplchance = secfile_lookup_int_default(file, 3, /* old default */
					"game.diplchance");
			if (game.info.diplchance < 2) 
			{
				game.info.diplchance = GAME_MAX_DIPLCHANCE;
			}
			else if (game.info.diplchance > 10) 
			{
				game.info.diplchance = GAME_MIN_DIPLCHANCE;
			}
			else 
			{
				game.info.diplchance = 100 - (10 * (game.info.diplchance - 1));
			}
		}

		game.info.aqueductloss =
			secfile_lookup_int_default(file, game.info.aqueductloss,
					"game.aqueductloss");
		game.info.killcitizen =
			secfile_lookup_int_default(file, game.info.killcitizen,
					"game.killcitizen");
		game.info.savepalace =
			secfile_lookup_bool_default(file, game.info.savepalace,
					"game.savepalace");
		game.info.turnblock =
			secfile_lookup_bool_default(file, game.info.turnblock,
					"game.turnblock");
		game.info.fixedlength =
			secfile_lookup_bool_default(file, game.info.fixedlength,
					"game.fixedlength");
		game.info.barbarianrate =
			secfile_lookup_int_default(file, game.info.barbarianrate,
					"game.barbarians");
		game.info.onsetbarbarian =
			secfile_lookup_int_default(file, game.info.onsetbarbarian,
					"game.onsetbarbs");
		game.info.revolution_length =
			secfile_lookup_int_default(file, game.info.revolution_length,
					"game.revolen");
		game.info.occupychance =
			secfile_lookup_int_default(file, game.info.occupychance,
					"game.occupychance");
		game.info.autoattack =
			secfile_lookup_bool_default(file, GAME_DEFAULT_AUTOATTACK,
					"game.autoattack");
		game.server.seed =
			secfile_lookup_int_default(file, game.server.seed,
					"game.randseed");
		game.info.allowed_city_names =
			secfile_lookup_int_default(file, game.info.allowed_city_names,
					"game.allowed_city_names"); 
		game.info.migration =
			secfile_lookup_int_default(file, game.info.migration,
					"game.migration");
		game.info.mgr_turninterval =
			secfile_lookup_int_default(file, game.info.mgr_turninterval,
					"game.mgr_turninterval");
		game.info.mgr_foodneeded =
			secfile_lookup_bool_default(file, game.info.mgr_foodneeded,
					"game.mgr_foodneeded");
		game.info.mgr_distance =
			secfile_lookup_int_default(file, game.info.mgr_distance,
					"game.mgr_distance");
		game.info.mgr_nationchance =
			secfile_lookup_int_default(file, game.info.mgr_nationchance,
					"game.mgr_nationchance");
		game.info.mgr_worldchance =
			secfile_lookup_int_default(file, game.info.mgr_worldchance,
					"game.mgr_worldchance");

		if(civstyle == 1) 
		{
			string = "civ1";
		}
		else 
		{
			string = "default";
		}

		if (!has_capability("rulesetdir", savefile_options)) 
		{
			char *str2, *str =
				secfile_lookup_str_default(file, "default", "game.info.t.techs");

			if (strcmp("classic",
						secfile_lookup_str_default(file, "default",
							"game.info.t.terrain")) == 0) 
			{
				/* TRANS: Fatal error message. */
				freelog(LOG_FATAL, _("Saved game uses the \"classic\" terrain"
							" ruleset, and is no longer supported."));
				exit(EXIT_FAILURE);
			}


#define T(x) \
			str2 = secfile_lookup_str_default(file, "default", x); \
			if (strcmp(str, str2) != 0) { \
				freelog(LOG_NORMAL, _("Warning: Different rulesetdirs " \
							"('%s' and '%s') are no longer supported. " \
							"Using '%s'."), \
						str, str2, str); \
			}

			T("game.info.t.units");
			T("game.info.t.buildings");
			T("game.info.t.terrain");
			T("game.info.t.governments");
			T("game.info.t.nations");
			T("game.info.t.cities");
			T("game.info.t.game");
#undef T

			sz_strlcpy(game.server.rulesetdir, str);
		}
		else 
		{
			sz_strlcpy(game.server.rulesetdir, 
					secfile_lookup_str_default(file, string,
						"game.rulesetdir"));
		}

		sz_strlcpy(game.server.demography,
				secfile_lookup_str_default(file, GAME_DEFAULT_DEMOGRAPHY,
					"game.demography"));
		sz_strlcpy(game.server.allow_take,
				secfile_lookup_str_default(file, GAME_DEFAULT_ALLOW_TAKE,
					"game.allow_take"));

		game.info.spacerace = secfile_lookup_bool_default(file, game.info.spacerace,
				"game.spacerace");
		game.info.endspaceship =
			secfile_lookup_bool_default(file, game.info.endspaceship,
					"game.endspaceship");

		game.info.auto_ai_toggle = 
			secfile_lookup_bool_default(file, game.info.auto_ai_toggle, 
					"game.auto_ai_toggle");

		game.server.event_cache.turns =
			secfile_lookup_int_default(file, game.server.event_cache.turns,
					"game.event_cache.turns");
		game.server.event_cache.max_size =
			secfile_lookup_int_default(file, game.server.event_cache.max_size,
					"game.event_cache.max_size");
		game.server.event_cache.chat =
			secfile_lookup_bool_default(file, game.server.event_cache.chat,
					"game.event_cache.chat");
		game.server.event_cache.info =
			secfile_lookup_bool_default(file, game.server.event_cache.info,
					"game.event_cache.info");

		load_rulesets();
	}

	if (has_capability("bases", savefile_options)) 
	{
		char **modname = NULL;
		int j;

		num_base_types = secfile_lookup_int_default(file, 0,
				"savefile.num_bases");

		if (num_base_types > 0) 
		{
			modname = secfile_lookup_str_vec(file, &num_base_types,
					"savefile.bases");
		}

		/* make sure that the size of the array is divisible by 4 */
		base_order = fc_calloc(4 * ((num_base_types + 3) / 4),
				sizeof(*base_order));
		for (j = 0; j < num_base_types; j++) 
		{
			base_order[j] = find_base_type_by_rule_name(modname[j]);
		}
		fc_free(modname);
	}

	/* Free all players from teams, and teams from players
	 * This must be done while players_iterate() still iterates
	 * to the previous number of players. */
	players_iterate(pplayer) 
	{
		team_remove_player(pplayer);
	} players_iterate_end;

	set_player_count(secfile_lookup_int_default(file, 0, "game.nplayers"));
	player_slots_iterate(pplayer) 
	{
		server_player_init(pplayer, FALSE, FALSE);
	} player_slots_iterate_end;

	script_state_load(file);


	{

		{

			{
				if (!has_capability("startunits", savefile_options)) 
				{
					int settlers = secfile_lookup_int(file, "game.settlers");
					int explorer = secfile_lookup_int(file, "game.explorer");
					int i;
					for (i = 0; settlers > 0 && i < (MAX_LEN_STARTUNIT - 1) ; i++, settlers--) 
					{
						game.info.start_units[i] = 'c';
					}
					for (; explorer > 0 && i < (MAX_LEN_STARTUNIT - 1) ; i++, explorer--) 
					{
						game.info.start_units[i] = 'x';
					}
					game.info.start_units[i] = '\0';
				}
				else 
				{
					sz_strlcpy(game.info.start_units,
							secfile_lookup_str_default(file,
								GAME_DEFAULT_START_UNITS,
								"game.start_units"));
				}
				game.info.dispersion =
					secfile_lookup_int_default(file, GAME_DEFAULT_DISPERSION,
							"game.dispersion");
			}

			map.topology_id = secfile_lookup_int_default(file, MAP_ORIGINAL_TOPO,
					"map.topology_id");
			map.server.size = secfile_lookup_int_default(file, MAP_DEFAULT_SIZE,
					"map.size");
			map.server.riches = secfile_lookup_int(file, "map.riches");
			map.server.huts = secfile_lookup_int(file, "map.huts");
			map.server.generator = secfile_lookup_int(file, "map.generator");
			map.server.startpos = secfile_lookup_int_default(file,
					MAP_DEFAULT_STARTPOS,
					"map.startpos");
			map.server.seed = secfile_lookup_int(file, "map.seed");
			map.server.landpercent = secfile_lookup_int(file, "map.landpercent");
			map.server.wetness =
				secfile_lookup_int_default(file, MAP_DEFAULT_WETNESS, "map.wetness");
			map.server.steepness =
				secfile_lookup_int_default(file, MAP_DEFAULT_STEEPNESS,
						"map.steepness");
			map.server.have_huts = secfile_lookup_bool_default(file, TRUE,
					"map.have_huts");
			map.server.temperature =
				secfile_lookup_int_default(file, MAP_DEFAULT_TEMPERATURE,
						"map.temperature");
			map.server.alltemperate
				= secfile_lookup_bool_default(file, MAP_DEFAULT_ALLTEMPERATE,
						"map.alltemperate");
			map.server.tinyisles
				= secfile_lookup_bool_default(file, MAP_DEFAULT_TINYISLES,
						"map.tinyisles");
			map.server.separatepoles
				= secfile_lookup_bool_default(file, MAP_DEFAULT_SEPARATE_POLES,
						"map.separatepoles");

			if (has_capability("startoptions", savefile_options)) 
			{
				map.xsize = secfile_lookup_int(file, "map.width");
				map.ysize = secfile_lookup_int(file, "map.height");
			}
			else 
			{
				/* old versions saved with these names in S_S_INITIAL: */
				map.xsize = secfile_lookup_int(file, "map.xsize");
				map.ysize = secfile_lookup_int(file, "map.ysize");
			}

	///////////////////////////////////////////////
			if (S_S_INITIAL == tmp_server_state && 0 == map.server.generator) 
			{
				/* generator 0 = map done with a map editor aka a "scenario" */
				if (has_capability("specials",savefile_options)) 
				{
					map_load(file, savefile_options, special_order,
							base_order, num_base_types);
					// return;
					goto cleanup_and_exit;
				}
				map_load_tiles(file);
				if (has_capability("riversoverlay",savefile_options)) 
				{
					map_load_rivers_overlay(file, special_order);
				}
				if (has_capability("startpos",savefile_options)) 
				{
					map_load_startpos(file);
					// return;
					goto cleanup_and_exit;
				}
				// return;
				goto cleanup_and_exit;
			}
		}
		if (S_S_INITIAL == tmp_server_state) 
		{
			// return;
			goto cleanup_and_exit;
		}
	}

	/* We check
	   1) if the block exists at all.
	   2) if it is saved. */
	if (section_file_lookup(file, "random.index_J")
			&& secfile_lookup_bool_default(file, TRUE, "game.save_random")) 
	{
		rstate.j = secfile_lookup_int(file,"random.index_J");
		rstate.k = secfile_lookup_int(file,"random.index_K");
		rstate.x = secfile_lookup_int(file,"random.index_X");
		for(i = 0; i < 8; i++) 
		{
			string = secfile_lookup_str(file, "random.table%d",i);
			sscanf(string,"%8x %8x %8x %8x %8x %8x %8x", &rstate.v[7*i],
					&rstate.v[7*i+1], &rstate.v[7*i+2], &rstate.v[7*i+3],
					&rstate.v[7*i+4], &rstate.v[7*i+5], &rstate.v[7*i+6]);
		}
		rstate.is_init = TRUE;
		set_myrand_state(rstate);
	}
	else 
	{
		/* mark it */
		(void) secfile_lookup_bool_default(file, TRUE, "game.save_random");

		/* We're loading a running game without a seed (which is okay, if it's
		 * a scenario).  We need to generate the game seed now because it will
		 * be needed later during the load. */
		if (S_S_GENERATING_WAITING < tmp_server_state) 
		{
			init_game_seed();
			rstate = get_myrand_state();
		}
	}

	if (0 == strlen(server.game_identifier)
			|| !is_base64url(server.game_identifier)) 
	{
		/* This uses myrand(), so random state has to be initialized before this. */
		randomize_base64url_string(server.game_identifier,
				sizeof(server.game_identifier));
	}

	//////////////////////////////////////////
	game.info.is_new_game = !secfile_lookup_bool_default(file, TRUE, "game.save_players");

	map_load(file, savefile_options, special_order, base_order, num_base_types);

	if (game.info.is_new_game) 
	{
		/* override previous load */
		set_player_count(0);
	}
	else 
	{
		int loaded_players = 0;

		/* destroyed wonders: */
		string = secfile_lookup_str_default(file, NULL,
				"game.destroyed_wonders_new");
		if (!string) 
		{
			/* old savegames */
			string = secfile_lookup_str_default(file, "",
					"game.destroyed_wonders");
			for (k = 0; string[k]; k++) 
			{
				const char *name = old_impr_type_name(k);
				if (!name) 
				{
					freelog(LOG_FATAL,
							"game.destroyed_wonders: unknown building (%d)",
							k);
					exit(EXIT_FAILURE);
				}
				if (string[k] == '1') 
				{
					struct impr_type *pimprove = find_improvement_by_rule_name(name);
					if (pimprove) 
					{
						game.info.great_wonder_owners[improvement_index(pimprove)] =
							WONDER_DESTROYED;
					}
				}
			}
		}
		else 
		{
			for (k = 0; k < improvement_order_size && string[k]; k++) 
			{
				if (string[k] == '1') 
				{
					struct impr_type *pimprove = 
						find_improvement_by_rule_name(improvement_order[k]);
					if (pimprove) 
					{
						game.info.great_wonder_owners[improvement_index(pimprove)] =
							WONDER_DESTROYED;
					}
				}
			}
		}

		server.identity_number =
			secfile_lookup_int_default(file, server.identity_number,
					"game.identity_number_used");

		/* Initialize nations we loaded from rulesets. This has to be after
		 * map loading and before we seek nations for players */
		init_available_nations();

		/* Now, load the players. */
		player_slots_iterate(pplayer) 
		{
			int plrno = player_number(pplayer);
			if (!secfile_has_section(file, "player%d", plrno)) 
			{
				player_slot_set_used(pplayer, FALSE);
				continue;
			}
			player_slot_set_used(pplayer, TRUE);
			player_load_main(pplayer, plrno, file, savefile_options,
					technology_order, technology_order_size);
			player_load_cities(pplayer, plrno, file, savefile_options,
					improvement_order, improvement_order_size);
			player_load_units(pplayer, plrno, file, savefile_options,
					base_order, num_base_types);
			player_load_attributes(pplayer, plrno, file);
			loaded_players++;
		} player_slots_iterate_end;

		/* Check that the number of players loaded matches the
		 * number of players set in the save file. */
		if (loaded_players != player_count()) 
		{
			freelog(LOG_ERROR, "The value of game.nplayers (%d) from the loaded "
					"game does not match the number of players present (%d). "
					"Setting game.nplayers to %d.",
					player_count(), loaded_players, loaded_players);
			set_player_count(loaded_players);
		}

		/* In case of tech_leakage, we can update research only after all
		 * the players have been loaded */
		players_iterate(pplayer) 
		{
			/* Mark the reachable techs */
			player_research_update(pplayer);
		} players_iterate_end;

		/* Some players may have invalid nations in the ruleset.  Once all 
		 * players are loaded, pick one of the remaining nations for them.
		 */
		players_iterate(pplayer) 
		{
			if (pplayer->nation == NO_NATION_SELECTED) 
			{
				player_set_nation(pplayer, pick_a_nation(NULL, FALSE, TRUE,
							NOT_A_BARBARIAN));
				/* TRANS: Minor error message: <Leader> ... <Poles>. */
				freelog(LOG_ERROR, _("%s had invalid nation; changing to %s."),
						player_name(pplayer),
						nation_plural_for_player(pplayer));
			}
		} players_iterate_end;

		/* Sanity check alliances, prevent allied-with-ally-of-enemy */
		players_iterate(plr) 
		{
			players_iterate(aplayer) 
			{
				if (plr->is_alive
						&& aplayer->is_alive
						&& pplayers_allied(plr, aplayer)
						&& pplayer_can_make_treaty(plr, aplayer, DS_ALLIANCE) 
						== DIPL_ALLIANCE_PROBLEM) 
				{
					freelog(LOG_ERROR, "Illegal alliance structure detected: "
							"%s alliance to %s reduced to peace treaty.",
							nation_rule_name(nation_of_player(plr)),
							nation_rule_name(nation_of_player(aplayer)));
					plr->diplstates[player_index(aplayer)].type = DS_PEACE;
					aplayer->diplstates[player_index(plr)].type = DS_PEACE;
				}
			} players_iterate_end;
		} players_iterate_end;

		/* Update all city information.  This must come after all cities are
		 * loaded (in player_load) but before player (dumb) cities are loaded
		 * in player_load_vision(). */
		cities_iterate(pcity) 
		{
			city_refresh_from_main_map(pcity, TRUE);
		} cities_iterate_end;

		/* Since the cities must be placed on the map to put them on the
		   player map we do this afterwards */
		players_iterate(pplayer) 
		{
			int n = player_index(pplayer);
			player_load_vision(pplayer, n, file, savefile_options, special_order,
					improvement_order, improvement_order_size,
					base_order, num_base_types);
		} players_iterate_end;

		whole_map_iterate(ptile) 
		{
			struct player *owner = tile_owner(ptile);

			if (owner) 
			{
				base_type_iterate(pbase) 
				{
					if (tile_has_base(ptile, pbase)) 
					{
						if (pbase->vision_main_sq > 0) 
						{
							map_refog_circle(owner, ptile, -1, pbase->vision_main_sq,
									game.info.vision_reveal_tiles, V_MAIN);
						}
						if (pbase->vision_invis_sq > 0) 
						{
							map_refog_circle(owner, ptile, -1, pbase->vision_invis_sq,
									game.info.vision_reveal_tiles, V_INVIS);
						}
					}
				} base_type_iterate_end;
			}
		} whole_map_iterate_end;

		/* We do this here since if the did it in player_load, player 1
		   would try to unfog (unloaded) player 2's map when player 1's units
		   were loaded */
		players_iterate(pplayer) 
		{
			pplayer->really_gives_vision = 0;
			pplayer->gives_shared_vision = 0;
		} players_iterate_end;

		players_iterate(pplayer) 
		{
			int n = player_index(pplayer);
			char *vision =
				secfile_lookup_str_default(file, NULL,
						"player%d.gives_shared_vision",
						n);
			if (vision) 
			{
				players_iterate(pplayer2) 
				{
					if (vision[player_index(pplayer2)] == '1') 
					{
						give_shared_vision(pplayer, pplayer2);
					}
				} players_iterate_end;
			}
		} players_iterate_end;

		initialize_globals();
		apply_unit_ordering();

		/* all vision is ready */
		map_calculate_borders(); /* does city_thaw_workers_queue() */
		/* city_refresh() below */

		/* Make sure everything is consistent. */
		players_iterate(pplayer) 
		{
			unit_list_iterate(pplayer->units, punit) 
			{
				if (!can_unit_continue_current_activity(punit)) 
				{
					freelog(LOG_ERROR, "Unit doing illegal activity in savegame!");
					punit->activity = ACTIVITY_IDLE;
				}
			} unit_list_iterate_end;

			city_list_iterate(pplayer->cities, pcity) 
			{
				repair_city_worker(pcity);
			} city_list_iterate_end;
		} players_iterate_end;

		cities_iterate(pcity) 
		{
			city_refresh(pcity); /* again */
			city_thaw_workers(pcity); /* may auto_arrange_workers() */
		} cities_iterate_end;
	}

	if (secfile_lookup_int_default(file, -1,
				"game.shuffled_player_%d", 0) >= 0) 
	{
		int shuffled_players[player_slot_count()];

		/* players_iterate() not used here */
		for (i = 0; i < player_slot_count(); i++) 
		{
			shuffled_players[i] = secfile_lookup_int_default(file,
					i, "game.shuffled_player_%d", i);
		}
		set_shuffled_players(shuffled_players);
	}
	else 
	{
		/* No shuffled players included, so shuffle them (this may include
		 * scenarios). */
		shuffle_players();
	}

	/* Fix ferrying sanity */
	players_iterate(pplayer) 
	{
		unit_list_iterate_safe(pplayer->units, punit) 
		{
			struct unit *ferry = game_find_unit_by_number(punit->transported_by);

			if (!ferry && !can_unit_exist_at_tile(punit, punit->tile)) 
			{
				freelog(LOG_ERROR, "Removing %s unferried %s in %s at (%d, %d)",
						nation_rule_name(nation_of_player(pplayer)),
						unit_rule_name(punit),
						terrain_rule_name(punit->tile->terrain),
						TILE_XY(punit->tile));
				bounce_unit(punit, TRUE);
			}
		} unit_list_iterate_safe_end;
	} players_iterate_end;

	/* Fix stacking issues.  We don't rely on the savegame preserving
	 * alliance invariants (old savegames often did not) so if there are any
	 * unallied units on the same tile we just bounce them. */
	players_iterate(pplayer) 
	{
		players_iterate(aplayer) 
		{
			resolve_unit_stacks(pplayer, aplayer, TRUE);
		} players_iterate_end;
	} players_iterate_end;

	players_iterate(pplayer) 
	{
		calc_civ_score(pplayer);
	} players_iterate_end;

	/* Recalculate the potential buildings for each city.  
	 * Has caused some problems with game random state. */
	players_iterate(pplayer) 
	{
		// [SB] Trying to fix a segfault on building_advisor_init() below.	
		//      Seems the aidata structure has not been initialized when	
		//      we get to that point, so adding the two lines below...		
		ai_data_phase_done (pplayer);
		ai_data_phase_init (pplayer, FALSE);
		// [SB-END]
	
		bool saved_ai_control = pplayer->ai_data.control;

		/* Recalculate for all players. */
		pplayer->ai_data.control = FALSE;

		if (pplayer->ai->funcs.building_advisor_init) 
		{
			pplayer->ai->funcs.building_advisor_init(pplayer);
		}

		pplayer->ai_data.control = saved_ai_control;
	} players_iterate_end;

	/* Restore game random state, just in case various initialization code
	 * inexplicably altered the previously existing state. */
	if (!game.info.is_new_game) 
	{
		set_myrand_state(rstate);
	}

	/* load event cache */
	event_cache_load(file, "event_cache");


cleanup_and_exit:
	fc_free (special_order);
	fc_free (base_order);
	fc_free (improvement_order);
	fc_free (technology_order);
}
