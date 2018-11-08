#include <stdio.h>
#include "config.h"
#include "client_main.h"
#include "message.h"
#include "hash.h"
#include "fc_types.h"
#include "climap.h"
#include "control.h"
#include "options.h"
#include "text.h"
#include "game.h"
#include "citydlg_common.h"
#include "government.h"
#include "mapctrl_common.h"
#include "cma_fec.h"

#include "tile.ih"

void ListNations (struct Message* _pResponse);
void ListGovernment (struct Message* _pResponse);
void ListGrid (struct Message* _pResponse);
void ListUnits (struct Message* _pResponse);
void ListCities (struct Message* _pResponse);
void ListTechnologies (struct Message* _pResponse, bool _bAll);
void ListCityGovernors (struct Message* _pResponse);

void UnitGoto (struct Message* _pResponse, int _iId, int _x, int _y);
void UnitMove (struct Message* _pResponse, int _iId, const char* _zDirection);
void UnitActMisc (struct Message* _pResponse, int _iId, const char* _zAction);
void CityBuild (struct Message* _pResponse, int _iId, const char* _zAction, const char* _zItem);
void SetResearch (struct Message* _pResponse, int _iId);
void SetCityGovernor (struct Message* _pResponse, int _iCity, int _iGovernor);
void SetGovernment (struct Message* _pResponse, int _iId);

enum direction8 NameToDirection (const char* _zDir);
struct unit* GetUnit (int _iId, struct Message* _pResponse);
int GetActivityCompletionTurns (struct unit* _pUnit);
void AvailableCityImprovements (struct Message* _pResponse, struct city* _pCity);
void GetTileOutput (struct tile* _pTile, int* _pOutput);
const char* GetAiRoleText (enum ai_unit_task _eAiTask);



//													
int GetActivityCompletionTurns (struct unit* _pUnit)
{
	return -1;
}


//													
void ListNations (struct Message* _pResponse)
{
	players_iterate (pPlayer)
	{
		char zText [1000];
		sprintf (zText, "   [%s] gov:%s (%d, %d, %d), %d:[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d] [%d,%d,%d,%d]\n",
				 pPlayer->name,
				 government_name_translation (pPlayer->government),
				 pPlayer->is_alive,
				 pPlayer->is_dying,
				 pPlayer->surrendered,
				 pPlayer->score.game,
				 pPlayer->score.happy,
				 pPlayer->score.content,
				 pPlayer->score.unhappy,
				 pPlayer->score.angry,
				 pPlayer->score.wonders,
				 pPlayer->score.techs,
				 pPlayer->score.techout,
				 pPlayer->score.landarea,
				 pPlayer->score.settledarea,
				 pPlayer->score.population,
				 pPlayer->score.cities,
				 pPlayer->score.units,
				 pPlayer->score.pollution,
				 pPlayer->score.literacy,
				 pPlayer->score.bnp,
				 pPlayer->score.mfg,
				 pPlayer->score.spaceship,
				 pPlayer->economic.gold,
				 pPlayer->economic.tax,
				 pPlayer->economic.science,
				 pPlayer->economic.luxury);

		append_to_message (_pResponse, zText);
	}
	players_iterate_end;
}


//													
void ListGovernment (struct Message* _pResponse)
{
	struct government* pCurrentGov = government_of_player (client.conn.playing);

	int g;
	for (g = 0; g < government_count (); ++ g)
	{
		struct government* pGov = government_by_number (g);
		char zStatus [100];
		if (pGov == pCurrentGov)
			strcpy (zStatus, "current");
		else if (true == can_change_to_government (client.conn.playing, pGov))
			strcpy (zStatus, "available");
		else
			*zStatus = '\0';

		char zText [1000];
		// sprintf (zText, "   %d [%s]  %s\n", g, pGov->name.translated, zStatus);
		sprintf (zText, "   %d [%s]  %s\n", g, government_name_translation (pGov), zStatus);
		assert (strlen (zText) < 1000);

		append_to_message (_pResponse, zText);
	}
}


//													
void ListGrid (struct Message* _pResponse)
{
	int x, y;
	for (x = 0; x < map.xsize; ++ x)
	{
		for (y = 0; y < map.ysize; ++ y)
		{
			struct tile* pTile = native_pos_to_tile (x, y);
			if (TILE_UNKNOWN == client_tile_get_known (pTile))
				continue;

			int iOutput [O_LAST];
			GetTileOutput (pTile, iOutput);

			char zSpecial [100];
			*zSpecial = '\0';
			if (true == tile_has_special (pTile, S_RIVER))
				strcat (zSpecial, " + river");
			if (true == tile_has_special (pTile, S_HUT))
				strcat (zSpecial, " + hut");
			if (true == tile_has_special (pTile, S_ROAD))
				strcat (zSpecial, " + road");
			if (true == tile_has_special (pTile, S_RAILROAD))
				strcat (zSpecial, " + railroad");
			if (true == tile_has_special (pTile, S_IRRIGATION))
				strcat (zSpecial, " + irrigated");
			if (true == tile_has_special (pTile, S_MINE))
				strcat (zSpecial, " + mine");
			if (true == tile_has_special (pTile, S_FARMLAND))
				strcat (zSpecial, " + farmland");
			assert (strlen (zSpecial) < 100);

			char zResource [100];
			*zResource = '\0';
			if (NULL != pTile->resource)
				sprintf (zResource, "%s + %s", zSpecial, resource_name_translation (pTile->resource));
			else
				strcpy (zResource, zSpecial);

			char zOwner [MAX_LEN_NAME];
			*zOwner = '\0';
			if (NULL != pTile->owner)
				strcpy (zOwner, pTile->owner->name);

			char zCityName [100];
			*zCityName = '\0';
			struct city* pCity = tile_city (pTile);
			if (NULL != pCity)
				strcpy (zCityName, pCity->name);
			assert (strlen (zCityName) < 100);

			char zUnits [1000];
			*zUnits = '\0';
			unit_list_iterate (pTile->units, pUnit)
			{
				char zName [100];
				// sprintf (zName, "%s:(%s)", pUnit->utype->name.translated, pUnit->owner->name);
				sprintf (zName, "%s:(%s)", unit_name_translation (pUnit), pUnit->owner->name);
				assert (strlen (zName) < 100);
				if (strlen (zUnits) > 0)
					strcat (zUnits, ", ");
				strcat (zUnits, zName);
			}
			unit_list_iterate_end;
			assert (strlen (zUnits) < 1000);
	
			char zText [10000];
			sprintf (zText, "   @(%d,%d) [%s%s] [%d,%d,%d,%d,%d,%d] [move:%d] [owner:%s] {%s} {%s}\n", 
					 x, y, 
					 // pTile->terrain->name.translated,
					 terrain_name_translation (pTile->terrain),
					 zResource,
					 iOutput[0], iOutput[1], iOutput[2], iOutput[3], iOutput[4], iOutput[5],
					 pTile->terrain->movement_cost,
					 zOwner,
					 zCityName,
					 zUnits);
			assert (strlen (zText) < 10000);

			append_to_message (_pResponse, zText);
		}
	}
}


//													
void ListUnits (struct Message* _pResponse)
{
	unit_list_iterate(client.conn.playing->units, punit)
	{
		if (NULL != _pResponse)
		{
			char zCityName [100];
			*zCityName = '\0';
			struct city* pCity = tile_city (punit->tile);
			if (NULL != pCity)
				strcpy (zCityName, pCity->name);
			assert (strlen (zCityName) < 100);
			
			char zText [1000];
			sprintf (zText, "   %d, @(%d,%d) [%s] [%s-%s:%d] %d/%d %d %s @[%s]\n", 
					punit->id, 
					punit->tile->x,
					punit->tile->y,
					// punit->utype->name.translated,
					unit_name_translation (punit),
					get_activity_text (punit->activity),
					GetAiRoleText (punit->ai.ai_role),
					punit->moves_left,
					punit->hp,
					punit->utype->hp,
					punit->veteran,
					concat_tile_activity_text (punit->tile),
					zCityName);

			assert (strlen (zText) < 1000);

			append_to_message (_pResponse, zText);
		}
	}
	unit_list_iterate_end;
}


//													
void ListCities (struct Message* _pResponse)
{
	city_list_iterate(client.conn.playing->cities, pcity)
	{
		char zSurplus [1000];
		sprintf (zSurplus, "%d,%d,%d,%d,%d,%d",
				 pcity->surplus[0],
				 pcity->surplus[1],
				 pcity->surplus[2],
				 pcity->surplus[3],
				 pcity->surplus[4],
				 pcity->surplus[5]);

		char zImprovements [10000];
		*zImprovements = '\0';
		int i;
		for (i = 0; i < B_LAST; ++ i)
		{
			if (pcity->built[i].turn <= 0)
				continue;
			struct impr_type* pImprovType = improvement_by_number (i);
			if ('\0' != *zImprovements)
				strcat (zImprovements, ", ");
			// strcat (zImprovements, pImprovType->name.translated);
			strcat (zImprovements, improvement_name_translation (pImprovType));
		}
		assert (strlen (zImprovements) < 10000);

		char zCurrentlyBuilding [101];
		universal_name_translation (&pcity->production, zCurrentlyBuilding, 100);

		char zText [10000];
		sprintf (zText, "   %d, @(%d,%d) [%s] size:%d(%d), %s f:%d, s:%d, +[%s], %s [%d] [%s]\n", 
				pcity->id, 
				pcity->tile->x,
				pcity->tile->y,
				pcity->name,
				pcity->size,
				city_turns_to_grow (pcity),
				cmafec_get_short_descr_of_city (pcity),
				pcity->food_stock,
				pcity->shield_stock,
				zSurplus,
				zCurrentlyBuilding,
				city_production_turns_to_build (pcity, TRUE),
				zImprovements);
		assert (strlen (zText) < 10000);

		append_to_message (_pResponse, zText);
	}
	city_list_iterate_end;
}


//													
void ListCityGovernors (struct Message* _pResponse)
{
	int i;
	for (i = 0; i < cmafec_preset_num (); ++ i)
	{
		char zText [100];
		sprintf (zText, "   %d [%s]\n", i, cmafec_preset_get_descr (i));
		append_to_message (_pResponse, zText);
	}
}


//													
void ListTechnologies (struct Message* _pResponse, bool _bAll)
{
	struct player_research* pResearch = get_player_research (client.conn.playing);
	int a;
	/* technology 0 is a dummy index for some reason unknown to me. */
	for (a = 1; a < advance_count (); ++ a)
	{
		struct advance* pAdvance = advance_by_number (a);
		if (NULL == pAdvance)
			continue;
		enum tech_state eState = pResearch->inventions [a].state;

		if (false == _bAll)
			if (TECH_UNKNOWN == eState)
				continue;

		char zState [100];
		if (a == pResearch->researching)
		{
			int iOurs, iTheirs;
			get_bulbs_per_turn (&iOurs, &iTheirs);
			int iTotal = iOurs + iTheirs;
			if (iTotal > 0)
			{
				int iTurns = (total_bulbs_required (client.conn.playing) + iTotal - 1) / iTotal;
				sprintf (zState, "researching (%d)", iTurns);
			}
			else
				sprintf (zState, "researching (never)");
		}
		else
			strcpy (zState, 
					(TECH_UNKNOWN == eState)? 
						"unknown" : (TECH_PREREQS_KNOWN == eState)? "prereqs_known" : "known");
		assert (strlen (zState) < 100);

		char zText [1000];
		// sprintf (zText, "   %d %s [%s]\n", a, pAdvance->name.translated, zState);
		sprintf (zText, "   %d %s [%s]\n", a, advance_name_translation (pAdvance), zState);
		assert (strlen (zText) < 1000);

		append_to_message (_pResponse, zText);
	}
}


//													
enum direction8 NameToDirection (const char* _zDir)
{
	if ('n' == *_zDir)
	{
		if ('e' == *(_zDir + 1))
			return DIR8_NORTHEAST;
		else if ('w' == *(_zDir + 1))
			return DIR8_NORTHWEST;
		else
			return DIR8_NORTH;
	}
	else if ('s' == *_zDir)
	{
		if ('e' == *(_zDir + 1))
			return DIR8_SOUTHEAST;
		else if ('w' == *(_zDir + 1))
			return DIR8_SOUTHWEST;
		else
			return DIR8_SOUTH;
	}
	else if ('e' == *_zDir)
		return DIR8_EAST;
	else if ('w' == *_zDir)
		return DIR8_WEST;

	// alternative direction tags ...
	else if ('^' == *_zDir)
		return DIR8_NORTH;
	else if ('v' == *_zDir)
		return DIR8_SOUTH;
	else if ('>' == *_zDir)
		return DIR8_EAST;
	else if ('<' == *_zDir)
		return DIR8_WEST;
	else if ('/' == *_zDir)
	{
		if ('>' == *(_zDir + 1))
			return DIR8_NORTHEAST;
		else if ('<' == *(_zDir + 1))
			return DIR8_SOUTHWEST;
	}
	else if ('\\' == *_zDir)
	{
		if ('>' == *(_zDir + 1))
			return DIR8_SOUTHEAST;
		else if ('<' == *(_zDir + 1))
			return DIR8_NORTHWEST;
	}

	return -1; /* there is no 'unknown' defined in enum direction8 */
}


//													
struct unit* GetUnit (int _iId, struct Message* _pResponse)
{
	struct unit* pUnit = game_find_unit_by_number (_iId);
	if (NULL == pUnit)
	{
		char zError [100];
		sprintf (zError, "[ERROR] Unit not found for id %d\n", _iId);
		assert (strlen (zError) < 100);
		append_to_message (_pResponse, zError);
		return NULL;
	}

	return pUnit;
}


//													
void UnitGoto (struct Message* _pResponse, int _iId, int _x, int _y)
{
	struct unit* pUnit = GetUnit (_iId, _pResponse);
	if (NULL == pUnit)
		return;

	struct tile* pTile = native_pos_to_tile (_x, _y);
	if (NULL == pTile)
	{
		char zError [100];
		sprintf (zError, "[ERROR] Goto command to invalid location (%d,%d)\n", _x, _y);
		assert (strlen (zError) < 100);
		append_to_message (_pResponse, zError);
		return;
	}
	if (pTile == pUnit->tile)
	{
		char zError [100];
		sprintf (zError, "Already at (%d,%d)\n", _x, _y);
		assert (strlen (zError) < 100);
		append_to_message (_pResponse, zError);
		return;
	}
	set_unit_focus (pUnit);
	keyboardless_goto_active = TRUE;
	request_unit_goto (ORDER_LAST);

	do_unit_goto (pTile);
	set_hover_state (NULL, HOVER_NONE, ACTIVITY_LAST, ORDER_LAST);
	keyboardless_goto_active = FALSE;

	// keyboardless_goto_start_tile = pUnit->tile;
	// maybe_activate_keyboardless_goto (_x, _y);
	// release_goto_button (_x, _y);
}


//													
void UnitMove (struct Message* _pResponse, int _iId, const char* _zDirection)
{
	struct unit* pUnit = GetUnit (_iId, _pResponse);
	if (NULL == pUnit)
		return;
	enum direction8 dirMove = NameToDirection (_zDirection);
	if (-1 == dirMove)
	{
		char zError [100];
		sprintf (zError, "[ERROR] Unknown direction [%s]\n", _zDirection);
		assert (strlen (zError) < 100);
		append_to_message (_pResponse, zError);
		return;
	}

	printf ("move unit %d %s\n", _iId, dir_get_name (dirMove));

	set_unit_focus (pUnit);
	key_unit_move (dirMove);
}


//													
void UnitActMisc (struct Message* _pResponse, int _iId, const char* _zAction)
{
	struct unit* pUnit = NULL;
	if (-1 != _iId)
	{
		pUnit = GetUnit (_iId, _pResponse);
		if (NULL == pUnit)
			return;
	}

	printf ("unit %d %s\n", _iId, _zAction);
	set_unit_focus (pUnit);

	if (0 == strcmp ("unit-center", _zAction))
		center_tile_mapcanvas (pUnit->tile);


	else if (0 == strcmp ("unit-cancel", _zAction))
		key_cancel_action ();
	else if (0 == strcmp ("unit-autoexplore", _zAction))
		key_unit_auto_explore ();
	else if (0 == strcmp ("unit-autosettle", _zAction))
		key_unit_auto_settle ();
	else if (0 == strcmp ("unit-build-city", _zAction))
	{
		ask_city_name = FALSE;
		key_unit_build_city ();
	}
	else if (0 == strcmp ("unit-build-wonder", _zAction))
		key_unit_build_wonder ();
	else if (0 == strcmp ("unit-disband", _zAction))
		key_unit_disband ();
	else if (0 == strcmp ("unit-clean-fallout", _zAction))
		key_unit_fallout ();
	else if (0 == strcmp ("unit-fortify", _zAction))
		key_unit_fortify ();
	else if (0 == strcmp ("unit-build-fortress", _zAction))
		key_unit_fortress ();
	else if (0 == strcmp ("unit-set-homecity", _zAction))
		key_unit_homecity ();
	else if (0 == strcmp ("unit-irrigate", _zAction))
		key_unit_irrigate ();
	else if (0 == strcmp ("unit-mine", _zAction))
		key_unit_mine ();
	else if (0 == strcmp ("unit-nuke", _zAction))
		key_unit_nuke ();
	else if (0 == strcmp ("unit-patrol", _zAction))
		key_unit_patrol ();
	else if (0 == strcmp ("unit-paradrop", _zAction))
		key_unit_paradrop ();
	else if (0 == strcmp ("unit-pillage", _zAction))
		key_unit_pillage ();
	else if (0 == strcmp ("unit-clean-pollution", _zAction))
		key_unit_pollution ();
	else if (0 == strcmp ("unit-build-road", _zAction))
		key_unit_road ();
	else if (0 == strcmp ("unit-sentry", _zAction))
		key_unit_sentry ();
	else if (0 == strcmp ("unit-transform", _zAction))
		key_unit_transform ();
	else if (0 == strcmp ("unit-unload-all", _zAction))
		key_unit_unload_all ();
	else if (0 == strcmp ("unit-wait", _zAction))
		key_unit_wait ();
	else if (0 == strcmp ("unit-wakeup-others", _zAction))
		key_unit_wakeup_others ();
	else
	{
		char zError [100];
		sprintf (zError, "[ERROR] Unknown action [%s]\n", _zAction);
		assert (strlen (zError) < 100);
		append_to_message (_pResponse, zError);
	}
}


//													
void AvailableCityImprovements (struct Message* _pResponse, struct city* _pCity)
{
	int i;
	for (i = 0; i < improvement_count (); ++i)
	{
		struct universal oTarget = universal_by_number (VUT_IMPROVEMENT, i);
		if (false == can_city_build_now (_pCity, oTarget))
			continue;
		char zName [101];
		universal_name_translation (&oTarget, zName, 100);
		char zText [100];
		sprintf (zText, "   i%d %s\n", i, zName);
		append_to_message (_pResponse, zText);
	}

	int u;
	for (u = 0; u < game.control.num_unit_types; ++u)
	{
		struct universal oTarget = universal_by_number (VUT_UTYPE, u);
		if (false == can_city_build_now (_pCity, oTarget))
			continue;
		char zName [101];
		universal_name_translation (&oTarget, zName, 100);
		char zText [100];
		sprintf (zText, "   u%d %s\n", u, zName);
		append_to_message (_pResponse, zText);
	}
}


//													
void CityBuild (struct Message* _pResponse, int _iId, const char* _zAction, const char* _zItem)
{
	struct city* pCity = game_find_city_by_number (_iId);
	if (0 == strcmp ("city-can-build", _zAction))
	{
		AvailableCityImprovements (_pResponse, pCity);
		return;
	}

	enum universals_n eBuildType = VUT_NONE;
	if ('u' == *_zItem)
		eBuildType = VUT_UTYPE;
	else if ('i' == *_zItem)
		eBuildType = VUT_IMPROVEMENT;

	if (VUT_NONE == eBuildType)
	{
		char zError [100];
		sprintf (zError, "[ERROR] Unknown build item [%s]\n", _zItem);
		assert (strlen (zError) < 100);
		append_to_message (_pResponse, zError);
		return;
	}

	struct universal oTarget = universal_by_number (eBuildType, atoi (_zItem + 1));
	if (0 == strcmp ("city-build-change", _zAction))
		city_change_production (pCity, oTarget);
	else if (0 == strcmp ("city-build-add", _zAction))
		city_queue_insert (pCity, -1, oTarget);
	else
	{
		char zError [100];
		sprintf (zError, "[ERROR] Unknown action [%s]\n", _zAction);
		assert (strlen (zError) < 100);
		append_to_message (_pResponse, zError);
		return;
	}
}


//													
void SetResearch (struct Message* _pResponse, int _iId)
{
	struct player_research* pResearch = get_player_research (client.conn.playing);
	if (_iId == pResearch->researching)
		return;
	dsend_packet_player_research (&client.conn, _iId);
}


//													
void SetGovernment (struct Message* _pResponse, int _iId)
{
	if (-1 == _iId)
		return;
	struct government* pGov = government_by_number (_iId);
	if ((NULL != client.conn.playing) &&
		(pGov != government_of_player (client.conn.playing)))
		dsend_packet_player_change_government (&client.conn, _iId);
}


//													
void SetCityGovernor (struct Message* _pResponse, int _iCity, int _iGovernor)
{
	struct city* pCity = game_find_city_by_number (_iCity);
	if (NULL == pCity)
		return;
	
	if (-1 == _iGovernor)
	{
		cma_release_city (pCity);
		return;
	}

	if (_iGovernor > cmafec_preset_num ())
		return;
	const struct cm_parameter* pGovernor = cmafec_preset_get_parameter (_iGovernor);
	if (NULL == pGovernor)
	{
		char zError [100];
		sprintf (zError, "[ERROR] Unknown city governor type [%d]\n", _iGovernor);
		assert (strlen (zError) < 100);
		append_to_message (_pResponse, zError);
		return;
	}

	cmafec_set_fe_parameter (pCity, pGovernor);

	if (true == cma_is_city_under_agent (pCity, NULL))
		cma_release_city (pCity);
	cma_put_city_under_agent (pCity, pGovernor);
}


//													
void GetTileOutput (struct tile* _pTile, int* _pOutput)
{
	int i;
	for (i = 0; i < O_LAST; i++) 
	{
		int iPenalty = 0;
		int iOutput = city_tile_output(NULL, _pTile, FALSE, i);

		if (NULL != client.conn.playing) 
		{
			iPenalty = get_player_output_bonus(client.conn.playing,
													get_output_type(i),
													EFT_OUTPUT_PENALTY_TILE);
		}

		if ((iPenalty > 0) && (iOutput > iPenalty))
			_pOutput [i] = iOutput - 1;
		else
			_pOutput [i] = iOutput;
	}

}


//													
const char* GetAiRoleText (enum ai_unit_task _eAiTask)
{
	switch (_eAiTask)
	{
		case AIUNIT_NONE:
			return "";
		case AIUNIT_AUTO_SETTLER:
			return "auto-settle";
		case AIUNIT_BUILD_CITY:
			return "build-city";
		case AIUNIT_DEFEND_HOME:
			return "defend-home";
		case AIUNIT_ATTACK:
			return "attack";
		case AIUNIT_ESCORT:
			return "escort";
		case AIUNIT_EXPLORE:
			return "explore";
		case AIUNIT_RECOVER:
			return "recover";
		case AIUNIT_HUNTER:
			return "hunt";
		default:
			return "";
	};
}






