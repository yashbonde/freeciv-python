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

#ifndef FC__REQUIREMENTS_INLINE_H
#define FC__REQUIREMENTS_INLINE_H

#include "fc_types.h"

#include "tech.h"
#include "terrain.h"
#include "unittype.h"

/*
 inline int num_world_buildings_total(const struct impr_type *building);
 inline int num_world_buildings(const struct impr_type *building);
 inline int num_player_buildings(const struct player *pplayer,
		const struct impr_type *building);
 inline int num_continent_buildings(const struct player *pplayer,
		int continent,
		const struct impr_type *building);
 inline int num_city_buildings(const struct city *pcity,
		const struct impr_type *building);
 inline int count_buildings_in_range(const struct player *target_player,
		const struct city *target_city,
		const struct impr_type *target_building,
		enum req_range range,
		bool survives,
		const struct impr_type *source);
 inline bool is_tech_in_range(const struct player *target_player,
		enum req_range range,
		Tech_type_id tech,
		enum req_problem_type prob_type);
 inline bool is_special_in_range(const struct tile *target_tile,
		enum req_range range, bool survives,
		enum tile_special_type special);
 inline bool is_terrain_in_range(const struct tile *target_tile,
		enum req_range range, bool survives,
		const struct terrain *pterrain);
 inline bool is_terrain_class_in_range(const struct tile *target_tile,
		enum req_range range, bool survives,
		enum terrain_class class);
 inline bool is_base_type_in_range(const struct tile *target_tile,
		enum req_range range, bool survives,
		struct base_type *pbase);
 inline bool is_terrain_alter_possible_in_range(const struct tile *target_tile,
		enum req_range range, bool survives,
		enum terrain_alteration alteration);
 inline bool is_nation_in_range(const struct player *target_player,
		enum req_range range, bool survives,
		const struct nation_type *nation);
 inline bool is_unittype_in_range(const struct unit_type *target_unittype,
		enum req_range range, bool survives,
		struct unit_type *punittype);
 inline bool is_unitflag_in_range(const struct unit_type *target_unittype,
		enum req_range range, bool survives,
		enum unit_flag_id unitflag,
		enum req_problem_type prob_type);
 inline bool is_unitclass_in_range(const struct unit_type *target_unittype,
		enum req_range range, bool survives,
		struct unit_class *pclass);
 inline bool is_unitclassflag_in_range(const struct unit_type *target_unittype,
		enum req_range range, bool survives,
		enum unit_class_flag_id ucflag);
 inline bool is_req_active(const struct player *target_player,
		const struct city *target_city,
		const struct impr_type *target_building,
		const struct tile *target_tile,
		const struct unit_type *target_unittype,
		const struct output_type *target_output,
		const struct specialist *target_specialist,
		const struct requirement *req,
		const enum   req_problem_type prob_type);
*/

/****************************************************************************
  Returns the number of total world buildings (this includes buildings
  that have been destroyed).
 ****************************************************************************/
static inline int num_world_buildings_total(const struct impr_type *building)
{
	if (is_great_wonder(building)) 
	{
		return (great_wonder_is_built(building)
				|| great_wonder_is_destroyed(building) ? 1 : 0);
	}
	else 
	{
		freelog(LOG_ERROR,
				/* TRANS: Obscure ruleset error. */
				_("World-ranged requirements are only supported for wonders."));
		return 0;
	}
}

/****************************************************************************
  Returns the number of buildings of a certain type in the world.
 ****************************************************************************/
static inline int num_world_buildings(const struct impr_type *building)
{
	if (is_great_wonder(building)) 
	{
		return (great_wonder_is_built(building) ? 1 : 0);
	}
	else 
	{
		freelog(LOG_ERROR,
				/* TRANS: Obscure ruleset error. */
				_("World-ranged requirements are only supported for wonders."));
		return 0;
	}
}

/****************************************************************************
  Returns the number of buildings of a certain type owned by plr.
 ****************************************************************************/
static inline int num_player_buildings(const struct player *pplayer,
		const struct impr_type *building)
{
	if (is_wonder(building)) 
	{
		return (wonder_is_built(pplayer, building) ? 1 : 0);
	}
	else 
	{
		freelog(LOG_ERROR,
				/* TRANS: Obscure ruleset error. */
				_("Player-ranged requirements are only supported for wonders."));
		return 0;
	}
}

/****************************************************************************
  Returns the number of buildings of a certain type on a continent.
 ****************************************************************************/
static inline int num_continent_buildings(const struct player *pplayer,
		int continent,
		const struct impr_type *building)
{
	if (is_wonder(building)) 
	{
		const struct city *pcity;

		pcity = find_city_from_wonder(pplayer, building);
		if (pcity && tile_continent(pcity->tile) == continent) 
		{
			return 1;
		}
	}
	else 
	{
		freelog(LOG_ERROR,
				/* TRANS: Obscure ruleset error. */
				_("Island-ranged requirements are only supported for wonders."));
	}
	return 0;
}

/****************************************************************************
  Returns the number of buildings of a certain type in a city.
 ****************************************************************************/
static inline int num_city_buildings(const struct city *pcity,
		const struct impr_type *building)
{
	return (city_has_building(pcity, building) ? 1 : 0);
}

/****************************************************************************
  How many of the source building are there within range of the target?

  The target gives the type of the target.  The exact target is a player,
  city, or building specified by the target_xxx arguments.

  The range gives the range of the requirement.

  "Survives" specifies whether the requirement allows destroyed sources.
  If set then all source buildings ever built are counted; if not then only
  living buildings are counted.

  source gives the building type of the source in question.

  Note that this function does a lookup into the source caches to find
  the number of available sources.  However not all source caches exist: if
  the cache doesn't exist then we return 0.
 ****************************************************************************/
static inline int count_buildings_in_range(const struct player *target_player,
		const struct city *target_city,
		const struct impr_type *target_building,
		enum req_range range,
		bool survives,
		const struct impr_type *source)
{
	if (improvement_obsolete(target_player, source)) 
	{
		return 0;
	}

	if (survives) 
	{
		if (range == REQ_RANGE_WORLD) 
		{
			return num_world_buildings_total(source);
		}
		else 
		{
			/* There is no sources cache for this. */
			freelog(LOG_ERROR,
					/* TRANS: Obscure ruleset error. */
					_("Surviving requirements are only "
						"supported at world range."));
			return 0;
		}
	}

	switch (range) 
	{
		case REQ_RANGE_WORLD:
			return num_world_buildings(source);
		case REQ_RANGE_PLAYER:
			return target_player ? num_player_buildings(target_player, source) : 0;
		case REQ_RANGE_CONTINENT:
			if (target_player && target_city) 
			{
				int continent = tile_continent(target_city->tile);

				return num_continent_buildings(target_player, continent, source);
			}
			else 
			{
				/* At present, "Continent" effects can affect only
				 * cities and units in cities. */
				return 0;
			}
		case REQ_RANGE_CITY:
			return target_city ? num_city_buildings(target_city, source) : 0;
		case REQ_RANGE_LOCAL:
			if (target_building && target_building == source) 
			{
				return num_city_buildings(target_city, source);
			}
			else 
			{
				/* TODO: other local targets */
				return 0;
			}
		case REQ_RANGE_ADJACENT:
			return 0;
		case REQ_RANGE_LAST:
			break;
	}

	freelog(LOG_ERROR, "count_buildings_in_range(): invalid range %d.", range);
	return 0;
}

/****************************************************************************
  Is there a source tech within range of the target?
 ****************************************************************************/
static inline bool is_tech_in_range(const struct player *target_player,
		enum req_range range,
		Tech_type_id tech,
		enum req_problem_type prob_type)
{
	switch (range) 
	{
		case REQ_RANGE_PLAYER:
			/* If target_player is NULL and prob_type RPT_POSSIBLE, then it will
			 * consider the advance is in range. */
			if (NULL != target_player) 
			{
				return TECH_KNOWN == player_invention_state(target_player, tech);
			}
			else 
			{
				return RPT_POSSIBLE == prob_type;
			}
		case REQ_RANGE_WORLD:
			return game.info.global_advances[tech];
		case REQ_RANGE_LOCAL:
		case REQ_RANGE_ADJACENT:
		case REQ_RANGE_CITY:
		case REQ_RANGE_CONTINENT:
		case REQ_RANGE_LAST:
			break;
	}

	freelog(LOG_ERROR, "is_tech_in_range(): invalid range %d.", range);
	return FALSE;
}

/****************************************************************************
  Is there a source special within range of the target?
 ****************************************************************************/
static inline bool is_special_in_range(const struct tile *target_tile,
		enum req_range range, bool survives,
		enum tile_special_type special)
{
	switch (range) 
	{
		case REQ_RANGE_LOCAL:
			return target_tile && tile_has_special(target_tile, special);
		case REQ_RANGE_ADJACENT:
			return target_tile && is_special_near_tile(target_tile, special, TRUE);
		case REQ_RANGE_CITY:
		case REQ_RANGE_CONTINENT:
		case REQ_RANGE_PLAYER:
		case REQ_RANGE_WORLD:
		case REQ_RANGE_LAST:
			break;
	}

	freelog(LOG_ERROR, "is_special_in_range(): invalid range %d.", range);
	return FALSE;
}

/****************************************************************************
  Is there a source tile within range of the target?
 ****************************************************************************/
static inline bool is_terrain_in_range(const struct tile *target_tile,
		enum req_range range, bool survives,
		const struct terrain *pterrain)
{
	if (!target_tile) 
	{
		return FALSE;
	}

	switch (range) 
	{
		case REQ_RANGE_LOCAL:
			/* The requirement is filled if the tile has the terrain. */
			return pterrain && tile_terrain(target_tile) == pterrain;
		case REQ_RANGE_ADJACENT:
			return pterrain && is_terrain_near_tile(target_tile, pterrain, TRUE);
		case REQ_RANGE_CITY:
		case REQ_RANGE_CONTINENT:
		case REQ_RANGE_PLAYER:
		case REQ_RANGE_WORLD:
		case REQ_RANGE_LAST:
			break;
	}

	freelog(LOG_ERROR, "is_terrain_in_range(): invalid range %d.", range);
	return FALSE;
}

/****************************************************************************
  Is there a source terrain class within range of the target?
 ****************************************************************************/
static inline bool is_terrain_class_in_range(const struct tile *target_tile,
		enum req_range range, bool survives,
		enum terrain_class class)
{
	if (!target_tile) 
	{
		return FALSE;
	}

	switch (range) 
	{
		case REQ_RANGE_LOCAL:
			/* The requirement is filled if the tile has the terrain of correct class. */
			return terrain_belongs_to_class(tile_terrain(target_tile), class);
		case REQ_RANGE_ADJACENT:
			return is_terrain_class_near_tile(target_tile, class);
		case REQ_RANGE_CITY:
		case REQ_RANGE_CONTINENT:
		case REQ_RANGE_PLAYER:
		case REQ_RANGE_WORLD:
		case REQ_RANGE_LAST:
			break;
	}

	freelog(LOG_ERROR, "is_terrain_class_in_range(): invalid range %d.",
			range);
	return FALSE;
}

/****************************************************************************
  Is there a source base type within range of the target?
 ****************************************************************************/
static inline bool is_base_type_in_range(const struct tile *target_tile,
		enum req_range range, bool survives,
		struct base_type *pbase)
{
	if (!target_tile) 
	{
		return FALSE;
	}

	switch (range) 
	{
		case REQ_RANGE_LOCAL:
			/* The requirement is filled if the tile has base of requested type. */
			return tile_has_base(target_tile, pbase);
		case REQ_RANGE_ADJACENT:
			return is_base_near_tile(target_tile, pbase);
		case REQ_RANGE_CITY:
		case REQ_RANGE_CONTINENT:
		case REQ_RANGE_PLAYER:
		case REQ_RANGE_WORLD:
		case REQ_RANGE_LAST:
			break;
	}

	freelog(LOG_ERROR, "is_base_type_in_range(): invalid range %d.", range);
	return FALSE;
}

/****************************************************************************
  Is there a terrain which can support the specified infrastructure
  within range of the target?
 ****************************************************************************/
static inline bool is_terrain_alter_possible_in_range(const struct tile *target_tile,
		enum req_range range, bool survives,
		enum terrain_alteration alteration)
{
	if (!target_tile) 
	{
		return FALSE;
	}

	switch (range) 
	{
		case REQ_RANGE_LOCAL:
			return terrain_can_support_alteration(tile_terrain(target_tile),
					alteration);
		case REQ_RANGE_ADJACENT: /* XXX Could in principle support ADJACENT. */
		case REQ_RANGE_CITY:
		case REQ_RANGE_CONTINENT:
		case REQ_RANGE_PLAYER:
		case REQ_RANGE_WORLD:
		case REQ_RANGE_LAST:
			break;
	}

	freelog(LOG_ERROR,
			"is_terrain_alter_possible_in_range(): invalid range %d.", range);
	return FALSE;
}

/****************************************************************************
  Is there a nation within range of the target?
 ****************************************************************************/
static inline bool is_nation_in_range(const struct player *target_player,
		enum req_range range, bool survives,
		const struct nation_type *nation)
{
	switch (range) 
	{
		case REQ_RANGE_PLAYER:
			return target_player && nation_of_player(target_player) == nation;
		case REQ_RANGE_WORLD:
			/* FIXME: inefficient */
			players_iterate(pplayer) 
			{
				if (nation_of_player(pplayer) == nation && (pplayer->is_alive || survives)) 
				{
					return TRUE;
				}
			} players_iterate_end;
			return FALSE;
		case REQ_RANGE_LOCAL:
		case REQ_RANGE_ADJACENT:
		case REQ_RANGE_CITY:
		case REQ_RANGE_CONTINENT:
		case REQ_RANGE_LAST:
			break;
	}

	freelog(LOG_ERROR, "is_nation_in_range(): invalid range %d.", range);
	return FALSE;
}

/****************************************************************************
  Is there a unit of the given type within range of the target?
 ****************************************************************************/
static inline bool is_unittype_in_range(const struct unit_type *target_unittype,
		enum req_range range, bool survives,
		struct unit_type *punittype)
{
	/* If no target_unittype is given, we allow the req to be met.  This is
	 * to allow querying of certain effect types (like the presence of city
	 * walls) without actually knowing the target unit. */
	return (range == REQ_RANGE_LOCAL
			&& (!target_unittype
				|| target_unittype == punittype));
}

/****************************************************************************
  Is there a unit with the given flag within range of the target?
 ****************************************************************************/
static inline bool is_unitflag_in_range(const struct unit_type *target_unittype,
		enum req_range range, bool survives,
		enum unit_flag_id unitflag,
		enum req_problem_type prob_type)
{
	/* If no target_unittype is given, we allow the req to be met.  This is
	 * to allow querying of certain effect types (like the presence of city
	 * walls) without actually knowing the target unit. */
	if (range != REQ_RANGE_LOCAL) 
	{
		return FALSE;
	}
	if (!target_unittype) 
	{
		/* Unknow means TRUE  for RPT_POSSIBLE
		 *              FALSE for RPT_CERTAIN
		 */
		return prob_type == RPT_POSSIBLE;
	}

	return utype_has_flag(target_unittype, unitflag);
}

/****************************************************************************
  Is there a unit with the given flag within range of the target?
 ****************************************************************************/
static inline bool is_unitclass_in_range(const struct unit_type *target_unittype,
		enum req_range range, bool survives,
		struct unit_class *pclass)
{
	/* If no target_unittype is given, we allow the req to be met.  This is
	 * to allow querying of certain effect types (like the presence of city
	 * walls) without actually knowing the target unit. */
	return (range == REQ_RANGE_LOCAL
			&& (!target_unittype
				|| utype_class(target_unittype) == pclass));
}

/****************************************************************************
  Is there a unit with the given flag within range of the target?
 ****************************************************************************/
static inline bool is_unitclassflag_in_range(const struct unit_type *target_unittype,
		enum req_range range, bool survives,
		enum unit_class_flag_id ucflag)
{
	/* If no target_unittype is given, we allow the req to be met.  This is
	 * to allow querying of certain effect types (like the presence of city
	 * walls) without actually knowing the target unit. */
	return (range == REQ_RANGE_LOCAL
			&& (!target_unittype
				|| uclass_has_flag(utype_class(target_unittype), ucflag)));
}

/* The is_req_active() function below has been transplanted into	
 * the header here to ensure inlining across *.c files.*/
static inline bool is_req_active(const struct player *target_player,
		const struct city *target_city,
		const struct impr_type *target_building,
		const struct tile *target_tile,
		const struct unit_type *target_unittype,
		const struct output_type *target_output,
		const struct specialist *target_specialist,
		const struct requirement *req,
		const enum   req_problem_type prob_type)
{
	bool eval = FALSE;

	/* Note the target may actually not exist.  In particular, effects that
	 * have a VUT_SPECIAL or VUT_TERRAIN may often be passed to this function
	 * with a city as their target.  In this case the requirement is simply
	 * not met. */
	switch (req->source.kind) 
	{
		case VUT_NONE:
			eval = TRUE;
			break;
		case VUT_ADVANCE:
			// printf ("VUT_ADVANCE\n");
			/* The requirement is filled if the player owns the tech. */
			eval = is_tech_in_range(target_player, req->range,
					advance_number(req->source.value.advance),
					prob_type);
			break;
		case VUT_GOVERNMENT:
			// printf ("VUT_GOVERNMENT\n");
			/* The requirement is filled if the player is using the government. */
			eval = (government_of_player(target_player) == req->source.value.govern);
			break;
		case VUT_IMPROVEMENT:
			// printf ("VUT_IMPROVEMENT\n");
			/* The requirement is filled if there's at least one of the building
			 * in the city.  (This is a slightly nonstandard use of
			 * count_sources_in_range.) */
			eval = (count_buildings_in_range(target_player, target_city,
						target_building,
						req->range, req->survives,
						req->source.value.building) > 0);
			break;
		case VUT_SPECIAL:
			// printf ("VUT_SPECIAL\n");
			eval = is_special_in_range(target_tile,
					req->range, req->survives,
					req->source.value.special);
			break;
		case VUT_TERRAIN:
			// printf ("VUT_TERRAIN\n");
			eval = is_terrain_in_range(target_tile,
					req->range, req->survives,
					req->source.value.terrain);
			break;
		case VUT_NATION:
			// printf ("VUT_NATION\n");
			eval = is_nation_in_range(target_player, req->range, req->survives,
					req->source.value.nation);
			break;
		case VUT_UTYPE:
			// printf ("VUT_UTYPE\n");
			eval = is_unittype_in_range(target_unittype,
					req->range, req->survives,
					req->source.value.utype);
			break;
		case VUT_UTFLAG:
			// printf ("VUT_UTFLAG\n");
			eval = is_unitflag_in_range(target_unittype,
					req->range, req->survives,
					req->source.value.unitflag,
					prob_type);
			break;
		case VUT_UCLASS:
			// printf ("VUT_UCLASS\n");
			eval = is_unitclass_in_range(target_unittype,
					req->range, req->survives,
					req->source.value.uclass);
			break;
		case VUT_UCFLAG:
			// printf ("VUT_UCFLAG\n");
			eval = is_unitclassflag_in_range(target_unittype,
					req->range, req->survives,
					req->source.value.unitclassflag);
			break;
		case VUT_OTYPE:
			// printf ("VUT_OTYPE\n");
			eval = (target_output
					&& target_output->index == req->source.value.outputtype);
			break;
		case VUT_SPECIALIST:
			// printf ("VUT_SPECIALIST\n");
			eval = (target_specialist
					&& target_specialist == req->source.value.specialist);
			break;
		case VUT_MINSIZE:
			// printf ("VUT_MINSIZE\n");
			eval = target_city && target_city->size >= req->source.value.minsize;
			break;
		case VUT_AI_LEVEL:
			// printf ("VUT_AI_LEVEL\n");
			eval = target_player
				&& target_player->ai_data.control
				&& target_player->ai_data.skill_level == req->source.value.ai_level;
			break;
		case VUT_TERRAINCLASS:
			// printf ("VUT_TERRAINCLASS\n");
			eval = is_terrain_class_in_range(target_tile,
					req->range, req->survives,
					req->source.value.terrainclass);
			break;
		case VUT_BASE:
			// printf ("VUT_BASE\n");
			eval = is_base_type_in_range(target_tile,
					req->range, req->survives,
					req->source.value.base);
			break;
		case VUT_MINYEAR:
			// printf ("VUT_MINYEAR\n");
			eval = game.info.year >= req->source.value.minyear;
			break;
		case VUT_TERRAINALTER:
			// printf ("VUT_TERRAINALTER\n");
			eval = is_terrain_alter_possible_in_range(target_tile,
					req->range, req->survives,
					req->source.value.terrainalter);
			break;
		case VUT_CITYTILE:
			// printf ("VUT_CITYTILE\n");
			if (target_tile) 
			{
				if (req->source.value.citytile == CITYT_CENTER) 
				{
					if (target_city) 
						eval = is_city_center(target_city, target_tile);
					else 
						eval = tile_city(target_tile) != NULL;
				}
				else 
				{
					/* Not implemented */
					freelog(LOG_ERROR, "is_req_active(): citytile %d not supported.",
							req->source.value.citytile);
					return FALSE;
				}
			}
			else 
				eval = FALSE;
			break;
		case VUT_LAST:
			freelog(LOG_ERROR, "is_req_active(): invalid source kind %d.",
					req->source.kind);
			return FALSE;
	}

	if (req->negated) 
		return !eval;
	else 
		return eval;
}



#endif
