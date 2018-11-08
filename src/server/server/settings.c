/********************************************************************** 
 Freeciv - Copyright (C) 1996-2004 - The Freeciv Project
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
***********************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "fcintl.h"
#include "game.h"
#include "ioz.h"
#include "log.h"

#include "map.h"

#include "ggzserver.h"
#include "plrhand.h"
#include "report.h"
#include "settings.h"
#include "srv_main.h"
#include "stdinhand.h"

/* Category names must match the values in enum sset_category. */
const char *sset_category_names[] = {N_("Geological"),
				     N_("Sociological"),
				     N_("Economic"),
				     N_("Military"),
				     N_("Scientific"),
				     N_("Internal"),
				     N_("Networking")};

/* Level names must match the values in enum sset_level. */
const char *sset_level_names[] = {N_("None"),
				  N_("All"),
				  N_("Vital"),
				  N_("Situational"),
				  N_("Rare"),
				  N_("Changed")};
const int OLEVELS_NUM = ARRAY_SIZE(sset_level_names);

/*************************************************************************
  Verify that a given allowtake string is valid.  See
  game.allow_take.
*************************************************************************/
static bool allowtake_callback(const char *value,
                               struct connection *caller,
                               const char **error_string)
{
  int len = strlen(value), i;
  bool havecharacter_state = FALSE;

  /* We check each character individually to see if it's valid.  This
   * does not check for duplicate entries.
   *
   * We also track the state of the machine.  havecharacter_state is
   * true if the preceeding character was a primary label, e.g.
   * NHhAadb.  It is false if the preceeding character was a modifier
   * or if this is the first character. */

  for (i = 0; i < len; i++) {
    /* Check to see if the character is a primary label. */
    if (strchr("HhAadbOo", value[i])) {
      havecharacter_state = TRUE;
      continue;
    }

    /* If we've already passed a primary label, check to see if the
     * character is a modifier. */
    if (havecharacter_state && strchr("1234", value[i])) {
      havecharacter_state = FALSE;
      continue;
    }

    /* Looks like the character was invalid. */
    *error_string = _("Allowed take string contains invalid\n"
		      "characters.  Try \"help allowtake\".");
    return FALSE;
  }

  /* All characters were valid. */
  *error_string = NULL;
  return TRUE;
}

/*************************************************************************
  Verify that a given startunits string is valid.  See
  game.info.start_units.
*************************************************************************/
static bool startunits_callback(const char *value,
                                struct connection *caller,
                                const char **error_string)
{
  int len = strlen(value), i;
  bool have_founder = FALSE;

  /* We check each character individually to see if it's valid, and
   * also make sure there is at least one city founder. */

  for (i = 0; i < len; i++) {
    /* Check for a city founder */
    if (value[i] == 'c') {
      have_founder = TRUE;
      continue;
    }
    /* TODO: add 'f' back in here when we can support ferry units */
    if (strchr("cwxksdDaA", value[i])) {
      continue;
    }

    /* Looks like the character was invalid. */
    *error_string = _("Starting units string contains invalid\n"
		      "characters.  Try \"help startunits\".");
    return FALSE;
  }

  if (!have_founder) {
    *error_string = _("Starting units string does not contain\n"
		      "at least one city founder.  Try \n"
		      "\"help startunits\".");
    return FALSE;
  }
  /* All characters were valid. */
  *error_string = NULL;
  return TRUE;
}

/*************************************************************************
  Verify that a given endturn is valid.
*************************************************************************/
static bool endturn_callback(int value, struct connection *caller,
                             const char **error_string)
{
  if (value < game.info.turn) {
    /* Tried to set endturn earlier than current turn */
    *error_string = _("Cannot set endturn earlier than current turn.");
    return FALSE;
  }
  return TRUE;
}

/*************************************************************************
  Verify that a given maxplayers string is valid.
*************************************************************************/
static bool maxplayers_callback(int value, struct connection *caller,
                                const char **error_string)
{
#ifdef GGZ_SERVER
  if (with_ggz) {
    /* In GGZ mode the maxplayers is the number of actual players - set
     * when the game is lauched and not changed thereafter.  This may be
     * changed in future. */
    *error_string = _("Cannot change maxplayers in GGZ mode.");
    return FALSE;
  }
#endif
  if (value < player_count()) {
    *error_string =_("Number of players is higher than requested value;\n"
		     "Keeping old value.");
    return FALSE;
  }

  *error_string = NULL;
  return TRUE;
}

/*************************************************************************
  Disallow low timeout values for non-hack connections.
*************************************************************************/
static bool timeout_callback(int value, struct connection *caller,
                             const char **error_string)
{
  if (caller && caller->access_level < ALLOW_HACK && value < 30) {
    *error_string = _("You are not allowed to set timeout values less "
                      "than 30 seconds.");
    return FALSE;
  }

  *error_string = NULL;
  return TRUE;
}

/*************************************************************************
  Check that everyone is on a team for team-alternating simultaneous
  phases. NB: Assumes that it is not possible to first set team
  alternating phase mode then make teamless players.
*************************************************************************/
static bool phasemode_callback(int value, struct connection *caller,
                               const char **error_string)
{
  if (value == PMT_TEAMS_ALTERNATE) {
    players_iterate(pplayer) {
      if (!pplayer->team) {
        *error_string = _("All players must have a team if this option "
                          "value is used.");
        return FALSE;
      }
    } players_iterate_end;
  }
  *error_string = NULL;
  return TRUE;
}

#define GEN_BOOL(name, value, sclass, scateg, slevel, to_client,	\
		 short_help, extra_help, func, _default)		\
  {name, sclass, to_client, short_help, extra_help, SSET_BOOL,		\
      scateg, slevel, &value, _default, func,				\
      NULL, 0, NULL, 0, 0,						\
      NULL, NULL, NULL, 0},

#define GEN_INT(name, value, sclass, scateg, slevel, to_client,		\
		short_help, extra_help, func, _min, _max, _default)	\
  {name, sclass, to_client, short_help, extra_help, SSET_INT,		\
      scateg, slevel,							\
      NULL, FALSE, NULL,						\
      &value, _default, func, _min, _max,				\
      NULL, NULL, NULL, 0},

#define GEN_STRING(name, value, sclass, scateg, slevel, to_client,	\
		   short_help, extra_help, func, _default)		\
  {name, sclass, to_client, short_help, extra_help, SSET_STRING,	\
      scateg, slevel,							\
      NULL, FALSE, NULL,						\
      NULL, 0, NULL, 0, 0,						\
      value, _default, func, sizeof(value)},

#define GEN_END							\
  {NULL, SSET_LAST, SSET_SERVER_ONLY, NULL, NULL, SSET_INT,	\
      SSET_NUM_CATEGORIES, SSET_NONE,				\
      NULL, FALSE, NULL,					\
      NULL, 0, NULL, 0, 0,					\
      NULL, NULL, NULL},

struct setting settings[] = {

  /* These should be grouped by sclass */
  
  /* Map size parameters: adjustable if we don't yet have a map */  
  GEN_INT("size", map.server.size, SSET_MAP_SIZE,
	  SSET_GEOLOGY, SSET_VITAL, SSET_TO_CLIENT,
          N_(""),
          N_(""), NULL,
          MAP_MIN_SIZE, MAP_MAX_SIZE, MAP_DEFAULT_SIZE)
  GEN_INT("topology", map.topology_id, SSET_MAP_SIZE,
	  SSET_GEOLOGY, SSET_VITAL, SSET_TO_CLIENT,
	  N_(""),
	  /* TRANS: do not edit the ugly ASCII art */
	  N_(""
          ), NULL,
	  MAP_MIN_TOPO, MAP_MAX_TOPO, MAP_DEFAULT_TOPO)

  /* Map generation parameters: once we have a map these are of historical
   * interest only, and cannot be changed.
   */
  GEN_INT("generator", map.server.generator,
	  SSET_MAP_GEN, SSET_GEOLOGY, SSET_VITAL,  SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL,
	  MAP_MIN_GENERATOR, MAP_MAX_GENERATOR, MAP_DEFAULT_GENERATOR)

  GEN_INT("startpos", map.server.startpos,
	  SSET_MAP_GEN, SSET_GEOLOGY, SSET_VITAL,  SSET_TO_CLIENT,
	  N_(""),
	  N_(""),
	  NULL, MAP_MIN_STARTPOS, MAP_MAX_STARTPOS, MAP_DEFAULT_STARTPOS)

  GEN_BOOL("tinyisles", map.server.tinyisles,
	   SSET_MAP_GEN, SSET_GEOLOGY, SSET_RARE, SSET_TO_CLIENT,
	   N_(""),
	   N_(""), NULL,
	   MAP_DEFAULT_TINYISLES)

  GEN_BOOL("separatepoles", map.server.separatepoles,
	   SSET_MAP_GEN, SSET_GEOLOGY, SSET_SITUATIONAL, SSET_TO_CLIENT,
	   N_(""),
	   N_(""), NULL, 
	   MAP_DEFAULT_SEPARATE_POLES)

  GEN_BOOL("alltemperate", map.server.alltemperate, 
           SSET_MAP_GEN, SSET_GEOLOGY, SSET_RARE, SSET_TO_CLIENT,
	   N_(""),
	   N_(""),
	   NULL, MAP_DEFAULT_ALLTEMPERATE)

  GEN_INT("temperature", map.server.temperature,
 	  SSET_MAP_GEN, SSET_GEOLOGY, SSET_SITUATIONAL, SSET_TO_CLIENT,
 	  N_(""),
 	  N_(""), 
          NULL,
  	  MAP_MIN_TEMPERATURE, MAP_MAX_TEMPERATURE, MAP_DEFAULT_TEMPERATURE)
 
  GEN_INT("landmass", map.server.landpercent,
	  SSET_MAP_GEN, SSET_GEOLOGY, SSET_SITUATIONAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL,
	  MAP_MIN_LANDMASS, MAP_MAX_LANDMASS, MAP_DEFAULT_LANDMASS)

  GEN_INT("steepness", map.server.steepness,
	  SSET_MAP_GEN, SSET_GEOLOGY, SSET_SITUATIONAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL,
	  MAP_MIN_STEEPNESS, MAP_MAX_STEEPNESS, MAP_DEFAULT_STEEPNESS)

  GEN_INT("wetness", map.server.wetness,
 	  SSET_MAP_GEN, SSET_GEOLOGY, SSET_SITUATIONAL, SSET_TO_CLIENT,
 	  N_(""), 
	  N_(""), NULL, 
 	  MAP_MIN_WETNESS, MAP_MAX_WETNESS, MAP_DEFAULT_WETNESS)

  GEN_INT("mapseed", map.server.seed,
	  SSET_MAP_GEN, SSET_INTERNAL, SSET_RARE, SSET_SERVER_ONLY,
	  N_(""),
	  N_(""), NULL, 
	  MAP_MIN_SEED, MAP_MAX_SEED, MAP_DEFAULT_SEED)

  /* Map additional stuff: huts and specials.  gameseed also goes here
   * because huts and specials are the first time the gameseed gets used (?)
   * These are done when the game starts, so these are historical and
   * fixed after the game has started.
   */
  GEN_INT("gameseed", game.server.seed,
	  SSET_MAP_ADD, SSET_INTERNAL, SSET_RARE, SSET_SERVER_ONLY,
	  N_(""),
	  N_(""), NULL, 
	  GAME_MIN_SEED, GAME_MAX_SEED, GAME_DEFAULT_SEED)

  GEN_INT("specials", map.server.riches,
	  SSET_MAP_ADD, SSET_GEOLOGY, SSET_VITAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL,
	  MAP_MIN_RICHES, MAP_MAX_RICHES, MAP_DEFAULT_RICHES)

  GEN_INT("huts", map.server.huts,
	  SSET_MAP_ADD, SSET_GEOLOGY, SSET_VITAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""),
	  NULL, MAP_MIN_HUTS, MAP_MAX_HUTS, MAP_DEFAULT_HUTS)

  /* Options affecting numbers of players and AI players.  These only
   * affect the start of the game and can not be adjusted after that.
   * (Actually, minplayers does also affect reloads: you can't start a
   * reload game until enough players have connected (or are AI).)
   */
  GEN_INT("minplayers", game.info.min_players,
	  SSET_PLAYERS, SSET_INTERNAL, SSET_VITAL,
          SSET_TO_CLIENT,
	  N_(""),
	  N_(""),
	  NULL,
	  GAME_MIN_MIN_PLAYERS, GAME_MAX_MIN_PLAYERS, GAME_DEFAULT_MIN_PLAYERS)

  GEN_INT("maxplayers", game.info.max_players,
	  SSET_PLAYERS, SSET_INTERNAL, SSET_VITAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), maxplayers_callback,
	  GAME_MIN_MAX_PLAYERS, GAME_MAX_MAX_PLAYERS, GAME_DEFAULT_MAX_PLAYERS)

  GEN_INT("aifill", game.info.aifill,
	  SSET_PLAYERS, SSET_INTERNAL, SSET_VITAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL,
	  GAME_MIN_AIFILL, GAME_MAX_AIFILL, GAME_DEFAULT_AIFILL)

  GEN_INT("ec_turns", game.server.event_cache.turns,
          SSET_RULES_FLEXIBLE, SSET_INTERNAL, SSET_SITUATIONAL, SSET_TO_CLIENT,
          N_(""),
          N_(""),
          NULL, GAME_MIN_EVENT_CACHE_TURNS, GAME_MAX_EVENT_CACHE_TURNS,
          GAME_DEFAULT_EVENT_CACHE_TURNS)

  GEN_INT("ec_max_size", game.server.event_cache.max_size,
          SSET_RULES_FLEXIBLE, SSET_INTERNAL, SSET_SITUATIONAL, SSET_TO_CLIENT,
          N_(""),
          N_(""),
          NULL, GAME_MIN_EVENT_CACHE_MAX_SIZE, GAME_MAX_EVENT_CACHE_MAX_SIZE,
          GAME_DEFAULT_EVENT_CACHE_MAX_SIZE)

  GEN_BOOL("ec_chat", game.server.event_cache.chat,
           SSET_RULES_FLEXIBLE, SSET_INTERNAL, SSET_SITUATIONAL, SSET_TO_CLIENT,
           N_(""),
           N_(""),
           NULL, GAME_DEFAULT_EVENT_CACHE_CHAT)

  GEN_BOOL("ec_info", game.server.event_cache.info,
           SSET_RULES_FLEXIBLE, SSET_INTERNAL, SSET_SITUATIONAL, SSET_TO_CLIENT,
           N_(""),
           N_(""),
           NULL, GAME_DEFAULT_EVENT_CACHE_INFO)

  /* Game initialization parameters (only affect the first start of the game,
   * and not reloads).  Can not be changed after first start of game.
   */
  /* TODO: Add this line back when we can support Ferry units */
  /* "    f   = Ferryboat (eg., Trireme)\n" */
  GEN_STRING("startunits", game.info.start_units,
	     SSET_GAME_INIT, SSET_SOCIOLOGY, SSET_VITAL, SSET_TO_CLIENT,
		 N_(""),
		 N_(""),
		startunits_callback, GAME_DEFAULT_START_UNITS)

  GEN_INT("dispersion", game.info.dispersion,
	  SSET_GAME_INIT, SSET_SOCIOLOGY, SSET_SITUATIONAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL,
	  GAME_MIN_DISPERSION, GAME_MAX_DISPERSION, GAME_DEFAULT_DISPERSION)

  GEN_INT("gold", game.info.gold,
	  SSET_GAME_INIT, SSET_ECONOMICS, SSET_VITAL, SSET_TO_CLIENT,
	  N_(""), 
	  N_(""), NULL,
	  GAME_MIN_GOLD, GAME_MAX_GOLD, GAME_DEFAULT_GOLD)

  GEN_INT("techlevel", game.info.tech,
	  SSET_GAME_INIT, SSET_SCIENCE, SSET_VITAL, SSET_TO_CLIENT,
	  N_(""), 
	  N_(""), NULL,
	  GAME_MIN_TECHLEVEL, GAME_MAX_TECHLEVEL, GAME_DEFAULT_TECHLEVEL)

  GEN_INT("sciencebox", game.info.sciencebox,
	  SSET_RULES, SSET_SCIENCE, SSET_SITUATIONAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""),
	  NULL, GAME_MIN_SCIENCEBOX, GAME_MAX_SCIENCEBOX, 
	  GAME_DEFAULT_SCIENCEBOX)

  GEN_INT("techpenalty", game.info.techpenalty,
	  SSET_RULES, SSET_SCIENCE, SSET_RARE, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL,
	  GAME_MIN_TECHPENALTY, GAME_MAX_TECHPENALTY,
	  GAME_DEFAULT_TECHPENALTY)

  GEN_INT("diplcost", game.info.diplcost,
	  SSET_RULES, SSET_SCIENCE, SSET_RARE, SSET_TO_CLIENT,
	  N_(""),
	  N_(""),
	  NULL,
	  GAME_MIN_DIPLCOST, GAME_MAX_DIPLCOST, GAME_DEFAULT_DIPLCOST)

  GEN_INT("conquercost", game.info.conquercost,
	  SSET_RULES, SSET_SCIENCE, SSET_RARE, SSET_TO_CLIENT,
	  N_(""),
	  N_(""),
	  NULL,
	  GAME_MIN_CONQUERCOST, GAME_MAX_CONQUERCOST,
	  GAME_DEFAULT_CONQUERCOST)

  GEN_INT("freecost", game.info.freecost,
	  SSET_RULES, SSET_SCIENCE, SSET_RARE, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), 
	  NULL, 
	  GAME_MIN_FREECOST, GAME_MAX_FREECOST, GAME_DEFAULT_FREECOST)

  GEN_INT("foodbox", game.info.foodbox,
	  SSET_RULES, SSET_ECONOMICS, SSET_SITUATIONAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""),
	  NULL,
	  GAME_MIN_FOODBOX, GAME_MAX_FOODBOX, GAME_DEFAULT_FOODBOX)

  GEN_INT("aqueductloss", game.info.aqueductloss,
	  SSET_RULES, SSET_ECONOMICS, SSET_RARE, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL, 
	  GAME_MIN_AQUEDUCTLOSS, GAME_MAX_AQUEDUCTLOSS, 
	  GAME_DEFAULT_AQUEDUCTLOSS)

  GEN_INT("shieldbox", game.info.shieldbox,
	  SSET_RULES, SSET_ECONOMICS, SSET_SITUATIONAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""),
	  NULL, GAME_MIN_SHIELDBOX, GAME_MAX_SHIELDBOX,
	  GAME_DEFAULT_SHIELDBOX)

  /* Notradesize and fulltradesize used to have callbacks to prevent them
   * from being set illegally (notradesize > fulltradesize).  However this
   * provided a problem when setting them both through the client's settings
   * dialog, since they cannot both be set atomically.  So the callbacks were
   * removed and instead the game now knows how to deal with invalid
   * settings. */
  GEN_INT("fulltradesize", game.info.fulltradesize,
	  SSET_RULES, SSET_ECONOMICS, SSET_RARE, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL, 
	  GAME_MIN_FULLTRADESIZE, GAME_MAX_FULLTRADESIZE, 
	  GAME_DEFAULT_FULLTRADESIZE)

  GEN_INT("notradesize", game.info.notradesize,
	  SSET_RULES, SSET_ECONOMICS, SSET_RARE, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL,
	  GAME_MIN_NOTRADESIZE, GAME_MAX_NOTRADESIZE,
	  GAME_DEFAULT_NOTRADESIZE)

  GEN_INT("citymindist", game.info.citymindist,
	  SSET_RULES, SSET_SOCIOLOGY, SSET_SITUATIONAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""),
	  NULL,
	  GAME_MIN_CITYMINDIST, GAME_MAX_CITYMINDIST,
	  GAME_DEFAULT_CITYMINDIST)

  GEN_INT("trademindist", game.info.trademindist,
          SSET_RULES_FLEXIBLE, SSET_ECONOMICS, SSET_RARE, SSET_TO_CLIENT,
          N_(""),
          N_(""), NULL,
          GAME_MIN_TRADEMINDIST, GAME_MAX_TRADEMINDIST,
          GAME_DEFAULT_TRADEMINDIST)

  GEN_INT("rapturedelay", game.info.rapturedelay,
	  SSET_RULES, SSET_SOCIOLOGY, SSET_SITUATIONAL, SSET_TO_CLIENT,
          N_(""),
          N_(""),
		  NULL,
          GAME_MIN_RAPTUREDELAY, GAME_MAX_RAPTUREDELAY,
          GAME_DEFAULT_RAPTUREDELAY)

  GEN_INT("razechance", game.info.razechance,
	  SSET_RULES, SSET_MILITARY, SSET_RARE, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL, 
	  GAME_MIN_RAZECHANCE, GAME_MAX_RAZECHANCE, GAME_DEFAULT_RAZECHANCE)

  GEN_INT("occupychance", game.info.occupychance,
	  SSET_RULES, SSET_MILITARY, SSET_RARE, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL, 
	  GAME_MIN_OCCUPYCHANCE, GAME_MAX_OCCUPYCHANCE, 
	  GAME_DEFAULT_OCCUPYCHANCE)

  GEN_BOOL("autoattack", game.info.autoattack, SSET_RULES_FLEXIBLE, SSET_MILITARY,
         SSET_SITUATIONAL, SSET_TO_CLIENT,
         N_(""),
         N_(""), 
         NULL, GAME_DEFAULT_AUTOATTACK)

  GEN_INT("killcitizen", game.info.killcitizen,
          SSET_RULES, SSET_MILITARY, SSET_RARE, SSET_TO_CLIENT,
          N_(""),
          N_(""), NULL,
          GAME_MIN_KILLCITIZEN, GAME_MAX_KILLCITIZEN,
          GAME_DEFAULT_KILLCITIZEN)

  GEN_INT("borders", game.info.borders,
	  SSET_RULES, SSET_MILITARY, SSET_SITUATIONAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""),
	  NULL,
	  GAME_MIN_BORDERS, GAME_MAX_BORDERS, GAME_DEFAULT_BORDERS)

  GEN_BOOL("happyborders", game.info.happyborders,
	   SSET_RULES, SSET_MILITARY, SSET_SITUATIONAL,
	   SSET_TO_CLIENT,
	   N_(""),
	   N_(""), NULL,
	   GAME_DEFAULT_HAPPYBORDERS)

  GEN_INT("diplomacy", game.info.diplomacy,
	  SSET_RULES, SSET_MILITARY, SSET_SITUATIONAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL,
	  GAME_MIN_DIPLOMACY, GAME_MAX_DIPLOMACY, GAME_DEFAULT_DIPLOMACY)

  GEN_INT("citynames", game.info.allowed_city_names,
	  SSET_RULES, SSET_SOCIOLOGY, SSET_RARE, SSET_TO_CLIENT,
	  N_(""),
	  N_(""),
	  NULL,
	  GAME_MIN_ALLOWED_CITY_NAMES, GAME_MAX_ALLOWED_CITY_NAMES, 
	  GAME_DEFAULT_ALLOWED_CITY_NAMES)
  
  /* Flexible rules: these can be changed after the game has started.
   *
   * The distinction between "rules" and "flexible rules" is not always
   * clearcut, and some existing cases may be largely historical or
   * accidental.  However some generalizations can be made:
   *
   *   -- Low-level game mechanics should not be flexible (eg, rulesets).
   *   -- Options which would affect the game "state" (city production etc)
   *      should not be flexible (eg, foodbox).
   *   -- Options which are explicitly sent to the client (eg, in
   *      packet_game_info) should probably not be flexible, or at
   *      least need extra care to be flexible.
   */
  GEN_INT("barbarians", game.info.barbarianrate,
	  SSET_RULES_FLEXIBLE, SSET_MILITARY, SSET_VITAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL, 
	  GAME_MIN_BARBARIANRATE, GAME_MAX_BARBARIANRATE, 
	  GAME_DEFAULT_BARBARIANRATE)

  GEN_INT("onsetbarbs", game.info.onsetbarbarian,
	  SSET_RULES_FLEXIBLE, SSET_MILITARY, SSET_VITAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL,
	  GAME_MIN_ONSETBARBARIAN, GAME_MAX_ONSETBARBARIAN, 
	  GAME_DEFAULT_ONSETBARBARIAN)

  GEN_INT("revolen", game.info.revolution_length,
	  SSET_RULES_FLEXIBLE, SSET_SOCIOLOGY, SSET_RARE, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL, 
	  GAME_MIN_REVOLUTION_LENGTH, GAME_MAX_REVOLUTION_LENGTH, 
	  GAME_DEFAULT_REVOLUTION_LENGTH)

  GEN_BOOL("fogofwar", game.info.fogofwar,
	   SSET_RULES, SSET_MILITARY, SSET_RARE, SSET_TO_CLIENT,
	   N_(""),
	   N_(""), NULL, 
	   GAME_DEFAULT_FOGOFWAR)

  GEN_BOOL("foggedborders", game.server.foggedborders,
           SSET_RULES, SSET_MILITARY, SSET_RARE, SSET_TO_CLIENT,
           N_(""),
           N_(""), NULL,
           GAME_DEFAULT_FOGGEDBORDERS)

  GEN_INT("diplchance", game.info.diplchance,
	  SSET_RULES_FLEXIBLE, SSET_MILITARY, SSET_SITUATIONAL, SSET_TO_CLIENT,
	  N_(""),
	  /* xgettext:no-c-format */
	  N_(""),
          NULL,
	  GAME_MIN_DIPLCHANCE, GAME_MAX_DIPLCHANCE, GAME_DEFAULT_DIPLCHANCE)

  GEN_BOOL("spacerace", game.info.spacerace,
	   SSET_RULES_FLEXIBLE, SSET_SCIENCE, SSET_VITAL, SSET_TO_CLIENT,
	   N_(""),
	   N_(""),
	   NULL, 
	   GAME_DEFAULT_SPACERACE)

  GEN_BOOL("endspaceship", game.info.endspaceship, SSET_RULES_FLEXIBLE,
           SSET_SCIENCE, SSET_VITAL, SSET_TO_CLIENT,
           N_(""),
           N_(""), NULL,
           GAME_DEFAULT_END_SPACESHIP)

  GEN_INT("civilwarsize", game.info.civilwarsize,
	  SSET_RULES_FLEXIBLE, SSET_SOCIOLOGY, SSET_RARE, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL, 
	  GAME_MIN_CIVILWARSIZE, GAME_MAX_CIVILWARSIZE, 
	  GAME_DEFAULT_CIVILWARSIZE)

  GEN_INT("contactturns", game.info.contactturns,
	  SSET_RULES_FLEXIBLE, SSET_MILITARY, SSET_RARE, SSET_TO_CLIENT,
	  N_(""),
	  N_(""),
	  NULL,
	  GAME_MIN_CONTACTTURNS, GAME_MAX_CONTACTTURNS, 
	  GAME_DEFAULT_CONTACTTURNS)

  GEN_BOOL("savepalace", game.info.savepalace,
	   SSET_RULES_FLEXIBLE, SSET_MILITARY, SSET_RARE, SSET_TO_CLIENT,
	   N_(""),
	   N_(""),
	   NULL,
	   GAME_DEFAULT_SAVEPALACE)

  GEN_BOOL("naturalcitynames", game.info.natural_city_names,
           SSET_RULES_FLEXIBLE, SSET_SOCIOLOGY, SSET_RARE, SSET_TO_CLIENT,
           N_(""),
           N_(""),
           NULL, GAME_DEFAULT_NATURALCITYNAMES)

  GEN_BOOL("migration", game.info.migration,
           SSET_RULES_FLEXIBLE, SSET_SOCIOLOGY, SSET_RARE, SSET_TO_CLIENT,
           N_(""),
           N_(""),
           NULL, GAME_DEFAULT_MIGRATION)

  GEN_INT("mgr_turninterval", game.info.mgr_turninterval,
          SSET_RULES_FLEXIBLE, SSET_SOCIOLOGY, SSET_RARE, SSET_TO_CLIENT,
          N_(""),
          N_(""), NULL,
          GAME_MIN_MGR_TURNINTERVAL, GAME_MAX_MGR_TURNINTERVAL,
          GAME_DEFAULT_MGR_TURNINTERVAL)

  GEN_BOOL("mgr_foodneeded", game.info.mgr_foodneeded,
          SSET_RULES_FLEXIBLE, SSET_SOCIOLOGY, SSET_RARE, SSET_TO_CLIENT,
           N_(""),
           N_(""), NULL,
           GAME_DEFAULT_MGR_FOODNEEDED)

  GEN_INT("mgr_distance", game.info.mgr_distance,
          SSET_RULES_FLEXIBLE, SSET_SOCIOLOGY, SSET_RARE, SSET_TO_CLIENT,
          N_(""),
          N_(""), NULL,
          GAME_MIN_MGR_DISTANCE, GAME_MAX_MGR_DISTANCE,
          GAME_DEFAULT_MGR_DISTANCE)

  GEN_INT("mgr_nationchance", game.info.mgr_nationchance,
          SSET_RULES_FLEXIBLE, SSET_SOCIOLOGY, SSET_RARE, SSET_TO_CLIENT,
          N_(""),
          N_(""), NULL,
          GAME_MIN_MGR_NATIONCHANCE, GAME_MAX_MGR_NATIONCHANCE,
          GAME_DEFAULT_MGR_NATIONCHANCE)

  GEN_INT("mgr_worldchance", game.info.mgr_worldchance,
          SSET_RULES_FLEXIBLE, SSET_SOCIOLOGY, SSET_RARE, SSET_TO_CLIENT,
          N_(""),
          N_(""), NULL,
          GAME_MIN_MGR_WORLDCHANCE, GAME_MAX_MGR_WORLDCHANCE,
          GAME_DEFAULT_MGR_WORLDCHANCE)

  /* Meta options: these don't affect the internal rules of the game, but
   * do affect players.  Also options which only produce extra server
   * "output" and don't affect the actual game.
   * ("endturn" is here, and not RULES_FLEXIBLE, because it doesn't
   * affect what happens in the game, it just determines when the
   * players stop playing and look at the score.)
   */
  GEN_STRING("allowtake", game.server.allow_take,
	     SSET_META, SSET_NETWORK, SSET_RARE, SSET_TO_CLIENT,
             N_(""),
             N_(""),
                allowtake_callback, GAME_DEFAULT_ALLOW_TAKE)

  GEN_BOOL("autotoggle", game.info.auto_ai_toggle,
	   SSET_META, SSET_NETWORK, SSET_SITUATIONAL, SSET_TO_CLIENT,
	   N_(""),
	   N_(""),
	   NULL, GAME_DEFAULT_AUTO_AI_TOGGLE)

  GEN_INT("endturn", game.info.end_turn,
	  SSET_META, SSET_SOCIOLOGY, SSET_VITAL, SSET_TO_CLIENT,
		  N_(""),
          N_(""),
          endturn_callback,
          GAME_MIN_END_TURN, GAME_MAX_END_TURN, GAME_DEFAULT_END_TURN)

  GEN_INT("timeout", game.info.timeout,
	  SSET_META, SSET_INTERNAL, SSET_VITAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""),
          timeout_callback,
          GAME_MIN_TIMEOUT, GAME_MAX_TIMEOUT, GAME_DEFAULT_TIMEOUT)

  GEN_INT("timeaddenemymove", game.server.timeoutaddenemymove,
	  SSET_META, SSET_INTERNAL, SSET_VITAL, SSET_TO_CLIENT,
	  N_(""),
	  N_(""),
	  NULL, 0, GAME_MAX_TIMEOUT, GAME_DEFAULT_TIMEOUTADDEMOVE)
  
  /* This setting points to the "stored" value; changing it won't have
   * an effect until the next synchronization point (i.e., the start of
   * the next turn). */
  GEN_INT("phasemode", game.server.phase_mode_stored,
	  SSET_META, SSET_INTERNAL, SSET_SITUATIONAL, SSET_TO_CLIENT,
          N_(""),
          /* NB: The values must match enum phase_mode_types
           * defined in common/game.h */
		  N_(""),
          phasemode_callback, GAME_MIN_PHASE_MODE,
          GAME_MAX_PHASE_MODE, GAME_DEFAULT_PHASE_MODE)

  GEN_INT("nettimeout", game.info.tcptimeout,
	  SSET_META, SSET_NETWORK, SSET_RARE, SSET_TO_CLIENT,
	  N_(""),
	  N_(""),
	  NULL,
	  GAME_MIN_TCPTIMEOUT, GAME_MAX_TCPTIMEOUT, GAME_DEFAULT_TCPTIMEOUT)

  GEN_INT("netwait", game.info.netwait,
	  SSET_META, SSET_NETWORK, SSET_RARE, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL, 
	  GAME_MIN_NETWAIT, GAME_MAX_NETWAIT, GAME_DEFAULT_NETWAIT)

  GEN_INT("pingtime", game.info.pingtime,
	  SSET_META, SSET_NETWORK, SSET_RARE, SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL, 
	  GAME_MIN_PINGTIME, GAME_MAX_PINGTIME, GAME_DEFAULT_PINGTIME)

  GEN_INT("pingtimeout", game.info.pingtimeout,
	  SSET_META, SSET_NETWORK, SSET_RARE,
          SSET_TO_CLIENT,
	  N_(""),
	  N_(""), NULL, 
	  GAME_MIN_PINGTIMEOUT, GAME_MAX_PINGTIMEOUT, GAME_DEFAULT_PINGTIMEOUT)

  GEN_BOOL("turnblock", game.info.turnblock,
	   SSET_META, SSET_INTERNAL, SSET_SITUATIONAL, SSET_TO_CLIENT,
	   N_(""),
	   N_(""), NULL, 
	   GAME_DEFAULT_TURNBLOCK)

  GEN_BOOL("fixedlength", game.info.fixedlength,
	   SSET_META, SSET_INTERNAL, SSET_SITUATIONAL, SSET_TO_CLIENT,
	   N_(""),
	   N_(""), NULL,
	   FALSE)

  GEN_STRING("demography", game.server.demography,
	     SSET_META, SSET_INTERNAL, SSET_SITUATIONAL, SSET_TO_CLIENT,
	     N_(""),
	     N_(""),
	     is_valid_demography, GAME_DEFAULT_DEMOGRAPHY)

  GEN_INT("saveturns", game.info.save_nturns,
	  SSET_META, SSET_INTERNAL, SSET_VITAL, SSET_SERVER_ONLY,
	  N_(""),
	  N_(""), NULL, 
          GAME_MIN_SAVETURNS, GAME_MAX_SAVETURNS, GAME_DEFAULT_SAVETURNS)

  GEN_INT("compress", game.info.save_compress_level,
	  SSET_META, SSET_INTERNAL, SSET_RARE, SSET_SERVER_ONLY,
	  N_(""),
	  N_(""), NULL,
	  GAME_MIN_COMPRESS_LEVEL, GAME_MAX_COMPRESS_LEVEL,
	  GAME_DEFAULT_COMPRESS_LEVEL)

  GEN_INT("compresstype", game.info.save_compress_type,
          SSET_META, SSET_INTERNAL, SSET_RARE, SSET_SERVER_ONLY,
          N_(""),
          N_(""), NULL,
	  GAME_MIN_COMPRESS_TYPE, GAME_MAX_COMPRESS_TYPE,
	  GAME_DEFAULT_COMPRESS_TYPE)

  GEN_STRING("savename", game.server.save_name,
	     SSET_META, SSET_INTERNAL, SSET_VITAL, SSET_SERVER_ONLY,
	     N_(""),
	     N_(""), NULL,
	     GAME_DEFAULT_SAVE_NAME)

  GEN_BOOL("scorelog", game.server.scorelog,
	   SSET_META, SSET_INTERNAL, SSET_SITUATIONAL, SSET_SERVER_ONLY,
	   N_(""),
	   N_(""), NULL,
	   GAME_DEFAULT_SCORELOG)

  GEN_END
};

#undef GEN_BOOL
#undef GEN_INT
#undef GEN_STRING
#undef GEN_END

/* The number of settings, not including the END. */
const int SETTINGS_NUM = ARRAY_SIZE(settings) - 1;

/****************************************************************************
  Returns the setting to the given id.
****************************************************************************/
struct setting *setting_by_number(int id)
{
  assert(0 <= id && id < SETTINGS_NUM);
  return settings + id;
}

/****************************************************************************
  Returns the id to the given setting.
****************************************************************************/
int setting_number(const struct setting *pset)
{
  assert(pset != NULL);
  return pset - settings;
}

/****************************************************************************
  Access function for the setting name.
****************************************************************************/
const char *setting_name(const struct setting *pset)
{
  return pset->name;
}

/****************************************************************************
  Access function for the short help (not translated yet) of the setting.
****************************************************************************/
const char *setting_short_help(const struct setting *pset)
{
  return pset->short_help;
}

/****************************************************************************
  Access function for the long (extra) help (not translated yet) of
  the setting.
****************************************************************************/
const char *setting_extra_help(const struct setting *pset)
{
  return pset->extra_help;
}

/****************************************************************************
  Access function for the setting type.
****************************************************************************/
enum sset_type setting_type(const struct setting *pset)
{
  return pset->stype;
}

/****************************************************************************
  Access function for the setting level (used by the /show command).
****************************************************************************/
enum sset_level setting_level(const struct setting *pset)
{
  return pset->slevel;
}

/****************************************************************************
  Access function for the setting category name.
****************************************************************************/
const char *setting_category_name(const struct setting *pset)
{
  return sset_category_names[pset->scategory];
}

/****************************************************************************
  Access function for the setting level name.
****************************************************************************/
const char *setting_level_name(const struct setting *pset)
{
  return sset_level_names[pset->slevel];
}

/****************************************************************************
  Returns whether the specified server setting (option) can currently
  be changed by the caller.  If it returns FALSE, then a reject message is
  stored into the 'reject_msg' argument, if not NULL.
****************************************************************************/
bool setting_is_changeable(const struct setting *pset,
                           struct connection *caller,
                           const char **reject_msg)
{
	if (caller && 
			(caller->access_level < ALLOW_BASIC
			||
			(caller->access_level < ALLOW_HACK && !pset->to_client)
			)
		)
	{
		if (reject_msg)
			*reject_msg = _("You are not allowed to set this option.");

		return FALSE;
	}
	else if (!setting_class_is_changeable(pset->sclass))
	{
		if (reject_msg)
			*reject_msg = _("This setting can't be modified "
							"after the game has started.");

		return FALSE;
	}
	else
		return TRUE;
}

/****************************************************************************
  Returns whether the specified server setting (option) can be seen by the
  caller.
****************************************************************************/
bool setting_is_visible(const struct setting *pset,
                        struct connection *caller)
{
  return (!caller
          || pset->to_client
          || caller->access_level >= ALLOW_HACK);
}

/****************************************************************************
  Returns the current boolean value.
****************************************************************************/
bool setting_bool_get(const struct setting *pset)
{
  assert(pset->stype == SSET_BOOL);
  return *pset->bool_value;
}

/****************************************************************************
  Returns the default boolean value for this setting.
****************************************************************************/
bool setting_bool_def(const struct setting *pset)
{
  assert(pset->stype == SSET_BOOL);
  return pset->bool_default_value;
}

/****************************************************************************
  Set the setting to 'val'.  Returns TRUE on success.  If it fails, then
  the 'reject_msg' argument will point to the reason of the failure.
****************************************************************************/
bool setting_bool_set(struct setting *pset, bool val,
                      struct connection *caller, const char **reject_msg)
{
  assert(pset->stype == SSET_BOOL);

  if (!setting_bool_validate(pset, val, caller, reject_msg)) {
    return FALSE;
  }

  *pset->bool_value = val;
  return TRUE;
}

/****************************************************************************
  Returns TRUE if 'val' is a valid value for this setting.  If it's not,
  then reject_msg' argument will point to the reason.

  FIXME: also check the access level of pconn.
****************************************************************************/
bool setting_bool_validate(const struct setting *pset, bool val,
                           struct connection *caller,
                           const char **reject_msg)
{
  assert(pset->stype == SSET_BOOL);
  return (setting_is_changeable(pset, caller, reject_msg)
          && (!pset->bool_validate
              || pset->bool_validate(val, caller, reject_msg)));
}

/****************************************************************************
  Returns the current integer value.
****************************************************************************/
int setting_int_get(const struct setting *pset)
{
  assert(pset->stype == SSET_INT);
  return *pset->int_value;
}

/****************************************************************************
  Returns the default integer value for this setting.
****************************************************************************/
int setting_int_def(const struct setting *pset)
{
  assert(pset->stype == SSET_INT);
  return pset->int_default_value;
}

/****************************************************************************
  Returns the minimal integer value for this setting.
****************************************************************************/
int setting_int_min(const struct setting *pset)
{
  assert(pset->stype == SSET_INT);
  return pset->int_min_value;
}

/****************************************************************************
  Returns the maximal integer value for this setting.
****************************************************************************/
int setting_int_max(const struct setting *pset)
{
  assert(pset->stype == SSET_INT);
  return pset->int_max_value;
}

/****************************************************************************
  Set the setting to 'val'.  Returns TRUE on success.  If it fails, then
  the 'reject_msg' argument will point to the reason of the failure.
****************************************************************************/
bool setting_int_set(struct setting *pset, int val,
                     struct connection *caller, const char **reject_msg)
{
  assert(pset->stype == SSET_INT);

  if (!setting_int_validate(pset, val, caller, reject_msg)) {
    return FALSE;
  }

  *pset->int_value = val;
  return TRUE;
}

/****************************************************************************
  Returns TRUE if 'val' is a valid value for this setting.  If it's not,
  then reject_msg' argument will point to the reason.

  FIXME: also check the access level of pconn.
****************************************************************************/
bool setting_int_validate(const struct setting *pset, int val,
                          struct connection *caller, const char **reject_msg)
{
  assert(pset->stype == SSET_INT);

  if (val < pset->int_min_value || val > pset->int_max_value) {
    *reject_msg = _("Value out of range.");
    return FALSE;
  }

  return (setting_is_changeable(pset, caller, reject_msg)
          && (!pset->int_validate
              || pset->int_validate(val, caller, reject_msg)));
}

/****************************************************************************
  Returns the current string.
****************************************************************************/
const char *setting_str_get(const struct setting *pset)
{
  assert(pset->stype == SSET_STRING);
  return pset->string_value;
}

/****************************************************************************
  Returns the default string for this setting.
****************************************************************************/
const char *setting_str_def(const struct setting *pset)
{
  assert(pset->stype == SSET_STRING);
  return pset->string_default_value;
}

/****************************************************************************
  Set the setting to 'val'.  Returns TRUE on success.  If it fails, then
  the 'reject_msg' argument will point to the reason of the failure.
****************************************************************************/
bool setting_str_set(struct setting *pset, const char *val,
                     struct connection *caller, const char **reject_msg)
{
  assert(pset->stype == SSET_STRING);

  if (!setting_str_validate(pset, val, caller, reject_msg)) {
    return FALSE;
  }

  mystrlcpy(pset->string_value, val, pset->string_value_size);
  return TRUE;
}

/****************************************************************************
  Returns TRUE if 'val' is a valid value for this setting.  If it's not,
  then reject_msg' argument will point to the reason.

  FIXME: also check the access level of pconn.
****************************************************************************/
bool setting_str_validate(const struct setting *pset, const char *val,
                          struct connection *caller, const char **reject_msg)
{
  assert(pset->stype == SSET_STRING);

  if (strlen(val) > pset->string_value_size) {
    *reject_msg = _("String value too long.");
    return FALSE;
  }

  return (setting_is_changeable(pset, caller, reject_msg)
          && (!pset->string_validate
              || pset->string_validate(val, caller, reject_msg)));
}

/********************************************************************
  Update the setting to the default value
*********************************************************************/
static void setting_set_to_default(struct setting *pset)
{
  switch (pset->stype) {
  case SSET_BOOL:
    (*pset->bool_value) = pset->bool_default_value;
    break;
  case SSET_INT:
    (*pset->int_value) = pset->int_default_value;
    break;
  case SSET_STRING:
    mystrlcpy(pset->string_value, pset->string_default_value,
              pset->string_value_size);
    break;
  }

  /* FIXME: duplicates stdinhand.c:set_command() */
  if (pset->int_value == &game.info.aifill) {
    aifill(*pset->int_value);
  } else if (pset->bool_value == &game.info.auto_ai_toggle) {
    if (*pset->bool_value) {
      players_iterate(pplayer) {
        if (!pplayer->ai_data.control && !pplayer->is_connected) {
           toggle_ai_player_direct(NULL, pplayer);
          send_player_info_c(pplayer, game.est_connections);
        }
      } players_iterate_end;
    }
  }
}

/**************************************************************************
  Initialize stuff related to this code module.
**************************************************************************/
void settings_init(void)
{
  settings_iterate(pset) {
    setting_set_to_default(pset);
  } settings_iterate_end;
}

/********************************************************************
  Reset all settings iff they are changeable.
*********************************************************************/
void settings_reset(void)
{
  settings_iterate(pset) {
    if (setting_is_changeable(pset, NULL, NULL)) {
      setting_set_to_default(pset);
    }
  } settings_iterate_end;
}

/**************************************************************************
  Update stuff every turn that is related to this code module. Run this
  on turn end.
**************************************************************************/
void settings_turn(void)
{
  /* Nothing at the moment. */
}

/**************************************************************************
  Deinitialize stuff related to this code module.
**************************************************************************/
void settings_free(void)
{
  /* Nothing at the moment. */
}

/****************************************************************************
  Tell the client about just one server setting.  Call this after a setting
  is saved.
****************************************************************************/
void send_server_setting(struct conn_list *dest, const struct setting *pset)
{
  struct packet_options_settable packet;

  if (!dest) {
    dest = game.est_connections;
  }

  conn_list_iterate(dest, pconn) {
    memset(&packet, 0, sizeof(packet));

    packet.id = setting_number(pset);
    sz_strlcpy(packet.name, setting_name(pset));
    sz_strlcpy(packet.short_help, setting_short_help(pset));
    sz_strlcpy(packet.extra_help, setting_extra_help(pset));

    packet.stype = setting_type(pset);
    packet.scategory = pset->scategory;
    packet.sclass = pset->sclass;
    packet.is_visible = setting_is_visible(pset, pconn);

    if (packet.is_visible) {
      switch (packet.stype) {
      case SSET_BOOL:
        packet.min = FALSE;
        packet.max = TRUE;
        packet.val = setting_bool_get(pset);
        packet.default_val = setting_bool_def(pset);
        break;
      case SSET_INT:
        packet.min = setting_int_min(pset);
        packet.max = setting_int_max(pset);
        packet.val = setting_int_get(pset);
        packet.default_val = setting_int_def(pset);
        break;
      case SSET_STRING:
        sz_strlcpy(packet.strval, setting_str_get(pset));
        sz_strlcpy(packet.default_strval, setting_str_def(pset));
        break;
      };
    }

    packet.initial_setting = game.info.is_new_game;

    send_packet_options_settable(pconn, &packet);
  } conn_list_iterate_end;
}

/****************************************************************************
  Send the ALLOW_HACK server settings.  Usually called when the access level
  of the user changes.
****************************************************************************/
void send_server_hack_level_settings(struct conn_list *dest)
{
  settings_iterate(pset) {
    if (!pset->to_client) {
      send_server_setting(dest, pset);
    }
  } settings_iterate_end;
}

/****************************************************************************
  Tell the client about all server settings.
****************************************************************************/
void send_server_settings(struct conn_list *dest)
{
  struct packet_options_settable_control control;
  int i;

  if (!dest) {
    dest = game.est_connections;
  }

  /* count the number of settings */
  control.num_settings = SETTINGS_NUM;

  /* fill in the category strings */
  control.num_categories = SSET_NUM_CATEGORIES;
  for (i = 0; i < SSET_NUM_CATEGORIES; i++) {
    strcpy(control.category_names[i], sset_category_names[i]);
  }

  /* send off the control packet */
  lsend_packet_options_settable_control(dest, &control);

  settings_iterate(pset) {
    send_server_setting(dest, pset);
  } settings_iterate_end;
}
