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
#include "learner_comms.h"
#include "unitlist.h"
#include "goto.h"
#include "movement.h"
#include "tech.h"
#include "player.h"

#include "tile.ih"

extern unsigned int game_won;
extern unsigned int game_lost;

int i_TotalArea;
int i_MySettledArea;
int i_MyKnownArea;
int i_EnemySettledArea;
int i_EnemyKnownArea;

int i_TotalCities;
int i_TotalUnits;
int i_TotalVeteranUnits;
int i_TotalCitySize;
float f_AverageCitySize;
int i_TotalGold;
int i_ExcessFood;
int i_ExcessShield;
int i_ExcessTrade;
int i_ExcessScience;
int i_ExcessGold;
int i_ExcessLuxury;
float f_MyGameScore;
float f_EnemyGameScore;

void ComputeWorldStats (void);
void GetPlayerScore (struct Message* _pResponse, struct player* _pPlayer);
void GetGameScore (struct Message* _pResponse);
void MeasureState (struct Message* _pResponse);
void MeasureStateNation (struct Message* _pResponse);
void MeasureStateCities (struct Message* _pResponse);
void MeasureStateUnits (struct Message* _pResponse);
void UnitGotoMI (struct Message* _pResponse, struct unit* _pUnit, int _x, int _y);
void UnitMoveMI (struct Message* _pResponse, struct unit* _pUnit, const char* _zDirection);

struct city* GetTileCity (struct tile* _pTile);
void SetCityGovernorMI (struct Message* _pResponse, struct city* _pCity, int _iGovernor);
void SetCityBuildMI (struct Message* _pResponse, struct city* _pCity, const char* _zItem);
void SetCityBuyMI (struct Message* _pResponse, struct city* _pCity, const char* _zItem);
int GetLastRequestSent (void);
int GetLastRequestProcessed (void);
const char* GetTileLabels (int _x, int _y, struct unit* _pCurrentUnit);
const char* GetCityImprovements (struct city* _pCity);
bool IsAlive (void);
void DistanceToClosestCity (int _x, int _y, struct city* _pIgnore, 
							float* _pDistanceToNearest, float* _pAverageDistance);
bool IsCityCenter (int _x, int _y);
void OpponentUnitDirections (int _x, int _y, char* _pOpponentPresense);

extern enum direction8 NameToDirection (const char* _zDir);
extern const char* GetAiRoleText (enum ai_unit_task _eAiTask);
extern void GetUnitGotoActions (struct Message* _pResponse,
								int _iUnitId,
								const char* _zUnitName,
								struct pf_map* _pPathMap,
								int _x, int _y);
extern enum unit_activity GetUnitActivity (struct unit* _pUnit);

bool PlayerHasCities (struct player* _pPlayer);
void ProcessLeftoverData (void);

//													
void ProcessLeftoverData (void)
{
	if (NULL == client.conn.buffer)
		return;
	if (client.conn.buffer->ndata > 0)
	{
		// printf ("calling input_from_server() on left-over data.\n");
		input_from_server (client.conn.sock, true, false, false);
		// printf ("done processing left-overs ------\n");
	}
	if (client.conn.buffer->ndata > 0)
		printf ("data leftover even after input_from_server: %d.\n", client.conn.buffer->ndata);
}

bool IsAlive (void)
{
	if (NULL == client.conn.playing)
		return true;
	return client.conn.playing->is_alive;
}

bool PlayerHasCities (struct player* _pPlayer)
{
	printf ("PlayerHasCities [%s]\n", _pPlayer->name);
	bool bHaveCities = (0 != city_list_size (_pPlayer->cities));
	bool bHaveSettler = false;
	unit_list_iterate (_pPlayer->units, pUnit)
	{
		if (false == can_unit_build_city (pUnit))
		{
			printf ("  unit [%s] can't build city\n", unit_name_translation (pUnit));
			continue;
		}
		printf ("  unit [%s] can build city !\n", unit_name_translation (pUnit));
		bHaveSettler = true;
		break;
	}
	unit_list_iterate_end;

	printf ("  c%d, u%d\n", bHaveCities, bHaveSettler);
	return (bHaveCities || bHaveSettler);
}


int GetLastRequestSent (void)
{
	return client.conn.client.last_request_id_used;
}

int GetLastRequestProcessed (void)
{
	return client.conn.client.last_processed_request_id_seen;
}

enum unit_activity GetUnitActivity (struct unit* _pUnit)
{
	return _pUnit->activity;
}


//													
bool IsCityCenter (int _x, int _y)
{
	struct tile* pTile = native_pos_to_tile (_x, _y);
	if (NULL == pTile)
		return false;
	struct city *pCity = pTile->worked;
	if (NULL == pCity)
		return false;
	return (pCity->tile == pTile);
}


//													
struct city* GetTileCity (struct tile* _pTile)
{
	int x, y;
	for (x = _pTile->x - 1; x <= _pTile->x + 1; ++ x)
	{
		if ((x < 0) || (x >= map.xsize))
			continue;
		for (y = _pTile->y - 2; y <= _pTile->y + 2; ++ y)
		{
			if ((y < 0) || (y >= map.ysize))
				continue;
			struct tile* pTile = native_pos_to_tile (x, y);
			if (NULL == pTile)
				continue;
			struct city *pCity = pTile->worked;
			if (NULL == pCity)
				continue;
			if (pCity->tile == pTile)
				return pCity;
		}
	}

	x = _pTile->x - 2;
	if (x >= 0)
	{
		for (y = _pTile->y - 1; y <= _pTile->y + 1; ++ y)
		{
			if ((y < 0) || (y >= map.ysize))
				continue;
			struct tile* pTile = native_pos_to_tile (x, y);
			if (NULL == pTile)
				continue;
			struct city *pCity = pTile->worked;
			if (NULL == pCity)
				continue;
			if (pCity->tile == pTile)
				return pCity;
		}
	}

	x = _pTile->x + 2;
	if (x < map.xsize)
	{
		for (y = _pTile->y - 1; y <= _pTile->y + 1; ++ y)
		{
			if ((y < 0) || (y >= map.ysize))
				continue;
			struct tile* pTile = native_pos_to_tile (x, y);
			if (NULL == pTile)
				continue;
			struct city *pCity = pTile->worked;
			if (NULL == pCity)
				continue;
			if (pCity->tile == pTile)
				return pCity;
		}
	}

	return NULL;
}


//													
void DistanceToClosestCity (int _x, int _y, struct city* _pIgnore,
							float* _pDistanceToNearest, float* _pAverageDistance)
{
	float fMinDistance = 0;
	float fTotalDistance = 0;
	int iCount = 0;
	city_list_iterate(client.conn.playing->cities, pCity)
	{
		if (pCity == _pIgnore)
			continue;
		if (NULL == game_find_city_by_number (pCity->id))
			continue;

		int dx = _x - pCity->tile->x;
		int dy = _y - pCity->tile->y;
		
		float fDistance = sqrt (dx * dx + dy * dy);
		if ((0 == fMinDistance) || (fMinDistance > fDistance))
			fMinDistance = fDistance;

		if (dx < 0)
			dx = -dx;
		if (dy < 0)
			dy = -dy;
		fTotalDistance += dx + dy;
		++ iCount;
	}
	city_list_iterate_end;

	*_pAverageDistance = (0 != iCount)? (fTotalDistance / (float) iCount) : 0;
	*_pDistanceToNearest = fMinDistance;
}


//													
void ComputeWorldStats (void)
{
	i_TotalArea = map.xsize * map.ysize;
	i_MyKnownArea = 0;
	i_MySettledArea = 0;
	i_EnemyKnownArea = 0;
	i_EnemySettledArea = 0;
	f_EnemyGameScore = 0;
	f_MyGameScore = client.conn.playing->score.game;

	//							
	int iMe = -1;
	players_iterate (pPlayer)
	{
		if (client.conn.playing == pPlayer)
			iMe = player_index (pPlayer);
		else
		{
			if (f_EnemyGameScore < pPlayer->score.game)
				f_EnemyGameScore = pPlayer->score.game;
		}
	}
	players_iterate_end;

	// 							
	i_TotalCities = 0;
	i_TotalCitySize = 0;
	int iSurplus [O_LAST];
	memset (iSurplus, 0, O_LAST * sizeof (int));
	city_list_iterate(client.conn.playing->cities, pcity)
	{
		if (NULL == game_find_city_by_number (pcity->id))
			continue;
		++ i_TotalCities;
		i_TotalCitySize += pcity->size;
		int i;
		for (i = 0; i < O_LAST; ++ i)
			iSurplus [i] += pcity->surplus [i];
	}
	city_list_iterate_end;

	if (0 != i_TotalCities)
		f_AverageCitySize = i_TotalCitySize / (float)i_TotalCities;
	else
		f_AverageCitySize = 0;

	i_TotalGold = client.conn.playing->economic.gold;
	i_ExcessFood = iSurplus [O_FOOD];
	i_ExcessShield = iSurplus [O_SHIELD];
	i_ExcessTrade = iSurplus [O_TRADE];
	i_ExcessGold = iSurplus [O_GOLD];
	i_ExcessScience = iSurplus [O_SCIENCE];
	i_ExcessLuxury = iSurplus [O_LUXURY];

	// 							
	i_TotalUnits = 0;
	i_TotalVeteranUnits = 0;
	unit_list_iterate (client.conn.playing->units, pUnit)
	{
		++ i_TotalUnits;
		i_TotalVeteranUnits += pUnit->veteran;
	}
	unit_list_iterate_end;

	//							
	int x, y;
	for (x = 0; x < map.xsize; ++ x)
	{
		for (y = 0; y < map.ysize; ++ y)
		{
			struct tile* pTile = native_pos_to_tile (x, y);
			if (NULL == pTile)
				continue;

			struct player* pOwner = pTile->owner;
			if (client.conn.playing == pOwner)
				++ i_MySettledArea;
			else if (NULL != pOwner)
				++ i_EnemySettledArea;

			if (true == BV_ISSET (pTile->tile_known, iMe))
				++ i_MyKnownArea;

			// if (true == BV_ISSET (pTile->tile_known, iEnemy))
			else if (true == BV_ISSET_ANY (pTile->tile_known))
				++ i_EnemyKnownArea;
		}
	}
}


//													
const char* GetTileLabels (int _x, int _y, struct unit* _pCurrentUnit)
{
	static char zLabels [10001];
	*zLabels = '\0';

	struct tile* pTile = native_pos_to_tile (_x, _y);
	if (NULL == pTile)
		return zLabels;
	if (TILE_UNKNOWN == client_tile_get_known (pTile))
	{
		strcpy (zLabels, "unknown-tile");
		return zLabels;
	}

	// sprintf (zLabels, "%s", pTile->terrain->name.translated);
	sprintf (zLabels, "%s", terrain_name_translation (pTile->terrain));

	if (true == tile_has_special (pTile, S_RIVER))
		strcat (zLabels, ",river");
	if (true == tile_has_special (pTile, S_HUT))
		strcat (zLabels, ",hut");
	if (true == tile_has_special (pTile, S_ROAD))
		strcat (zLabels, ",road");
	if (true == tile_has_special (pTile, S_RAILROAD))
		strcat (zLabels, ",railroad");
	if (true == tile_has_special (pTile, S_IRRIGATION))
		strcat (zLabels, ",irrigated");
	if (true == tile_has_special (pTile, S_MINE))
		strcat (zLabels, ",mine");
	if (true == tile_has_special (pTile, S_FARMLAND))
		strcat (zLabels, ",farmland");

	struct city* pCity = tile_city (pTile);
	if (NULL != pCity)
	{
		if (pTile->owner == client.conn.playing)
		{
			strcat (zLabels, ",city");
			const char* pzImprov = GetCityImprovements (pCity);
			if ('\0' != *pzImprov)
			{
				strcat (zLabels, ",");
				strcat (zLabels, pzImprov);
			}
		}
		else
			strcat (zLabels, ",enemy-city");
	}

	if (NULL != pTile->resource)
	{
		strcat (zLabels, ",");
		strcat (zLabels, resource_name_translation (pTile->resource));
	}

	unit_list_iterate (pTile->units, pUnit)
	{
		if (pUnit == _pCurrentUnit)
			continue;
		// if (unit_owner (pUnit) == client.conn.playing)
		//	continue;

		char zUnit [1000];
		// strcpy (zUnit, pUnit->utype->name.translated);
		strcpy (zUnit, unit_name_translation (pUnit));
		char* p = zUnit;
		while ('\0' != *p)
		{
			if (' ' == *p)
				*p = '-';
			++ p;
		}
		if (unit_owner (pUnit) != client.conn.playing)
		{
			if (true == pplayers_at_war (unit_owner(pUnit), client.conn.playing))
				strcat (zUnit, "-nmy");
			else
				strcat (zUnit, "-frgn");
		}
		assert (strlen (zUnit) < 1000);
		strcat (zLabels, ",");
		strcat (zLabels, zUnit);
	}
	unit_list_iterate_end;


	// other unit actions ...
	strcat (zLabels, ";");
	bool bFirst = true;
	unit_list_iterate (pTile->units, pUnit)
	{
		if (pUnit == _pCurrentUnit)
			continue;
		if (unit_owner (pUnit) != client.conn.playing)
			continue;

		char zUnit [1000];
		sprintf (zUnit, "%s-%s", 
				 //pUnit->utype->name.translated,
				 unit_name_translation (pUnit),
				 get_activity_text (pUnit->activity));
		char* p = zUnit;
		while ('\0' != *p)
		{
			if (' ' == *p)
				*p = '-';
			++ p;
		}
		assert (strlen (zUnit) < 1000);

		if (false == bFirst)
			strcat (zLabels, ",");
		bFirst = false;
		strcat (zLabels, zUnit);
	}
	unit_list_iterate_end;

	assert (strlen (zLabels) < 10000);
	return zLabels;
}


//													
const char* GetCityImprovements (struct city* _pCity)
{
	static char zAlreadyBuilt [10001];
	*zAlreadyBuilt = '\0';
	int i;
	for (i = 0; i < improvement_count (); ++i)
	{
		struct impr_type* pImprov = improvement_by_number (i);
		if (NULL == pImprov)
			continue;
		if (false == city_has_building (_pCity, pImprov))
			continue;

		if ('\0' != *zAlreadyBuilt)
			strcat (zAlreadyBuilt, LCP_BOW_SEPARATOR);
		strcat (zAlreadyBuilt, improvement_name_translation (pImprov));
	}
	assert (strlen (zAlreadyBuilt) < 10000);

	return zAlreadyBuilt;
}


//													
void GetPlayerScore (struct Message* _pResponse, struct player* _pPlayer)
{
	// city counts & surpluses ...	
	int iCities = 0;
	int iSurplus [O_LAST];
	memset (iSurplus, 0, O_LAST * sizeof (int));
	city_list_iterate(client.conn.playing->cities, pcity)
	{
		if (NULL == game_find_city_by_number (pcity->id))
			continue;
		++ iCities;
		int i;
		for (i = 0; i < O_LAST; ++ i)
			iSurplus [i] += pcity->surplus [i];
	}
	city_list_iterate_end;

	// unit counts ...				
	int iUnits = unit_list_size (client.conn.playing->units);

	// tech counts ...				
	int iTechs = 0;
	struct player_research* pResearch = get_player_research (_pPlayer);
	if (NULL != pResearch)
	{
		int a;
		for (a = 1; a < advance_count (); ++ a)
		{
			struct advance* pAdvance = advance_by_number (a);
			if (NULL == pAdvance)
				continue;
			enum tech_state eState = pResearch->inventions [a].state;
			if (TECH_KNOWN == eState)
				++ iTechs;
		}
	}

	// check for anarchy/revolution	
	int iNotAnarchy = 1;
	if (government_of_player(_pPlayer) == game.government_during_revolution) 
		iNotAnarchy = 0;

	// overall score ...	
	char zText [1000];
	sprintf (zText, 
			 "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d" LCP_ACTION_TERMINATOR,

			 _pPlayer->score.game,
			 _pPlayer->is_alive,
			 _pPlayer->is_dying,
			 _pPlayer->surrendered,
			 _pPlayer->score.happy,
			 _pPlayer->score.content,
			 _pPlayer->score.unhappy,
			 _pPlayer->score.angry,
			 _pPlayer->score.wonders,
			 _pPlayer->score.techs,
			 _pPlayer->score.techout,
			 _pPlayer->score.landarea,
			 _pPlayer->score.settledarea,
			 _pPlayer->score.population,
			 _pPlayer->score.cities,
			 _pPlayer->score.units,
			 _pPlayer->score.pollution,
			 _pPlayer->score.literacy,
			 _pPlayer->score.bnp,
			 _pPlayer->score.mfg,
			 _pPlayer->score.spaceship,
			 _pPlayer->economic.gold,
			 _pPlayer->economic.tax,
			 _pPlayer->economic.science,
			 _pPlayer->economic.luxury,
			 iUnits,
			 iCities,
			 iTechs,
			 iSurplus [O_FOOD],
			 iSurplus [O_SHIELD],
			 iSurplus [O_TRADE],
			 iSurplus [O_GOLD],
			 iSurplus [O_LUXURY],
			 iSurplus [O_SCIENCE],
			 iNotAnarchy,
			 ((true == game_won)? 1 : ((true == game_lost)? -1 : 0)));
	assert (strlen (zText) < 1000);

	append_to_message (_pResponse, zText);
	append_to_message (_pResponse, LCP_AGENT_TERMINATOR);
}


//													
void GetGameScore (struct Message* _pResponse)
{
	struct player* pMe = client.conn.playing;
	GetPlayerScore (_pResponse, pMe);

	players_iterate(pPlayer)
	{
		if (pPlayer == pMe)
			continue;
		GetPlayerScore (_pResponse, pPlayer);
	}
	players_iterate_end;

	append_to_message (_pResponse, LCP_TERMINATOR);
}


//													
void OpponentUnitDirections (int _x, int _y, char* _pOpponentPresense)
{
	memset (_pOpponentPresense, 0, 9 * sizeof (char));
	players_iterate (pPlayer)
	{
		if (client.conn.playing == pPlayer)
			continue;

		city_list_iterate(pPlayer->cities, pcity)
		{
			int dx = pcity->tile->x - _x;
			int dy = pcity->tile->y - _y;
			dx = (dx > 0)? 2 : ((dx < 0)? 0 : 1);
			dy = (dy > 0)? 2 : ((dy < 0)? 0 : 1);
			_pOpponentPresense [3 * dx + dy] = 1;
		}
		city_list_iterate_end;

		unit_list_iterate (pPlayer->units, pUnit)
		{
			int dx = pUnit->tile->x - _x;
			int dy = pUnit->tile->y - _y;
			dx = (dx > 0)? 2 : ((dx < 0)? 0 : 1);
			dy = (dy > 0)? 2 : ((dy < 0)? 0 : 1);
			_pOpponentPresense [3 * dx + dy] = 1;
		}
		unit_list_iterate_end;
	}
	players_iterate_end;

	//   0  
	// 0 1 2
	//   2  
	if (1 == _pOpponentPresense [3 * 0 + 0])
	{
		_pOpponentPresense [3 * 0 + 1] = 2;
		_pOpponentPresense [3 * 1 + 0] = 2;
	}
	if (1 == _pOpponentPresense [3 * 0 + 1])
	{
		_pOpponentPresense [3 * 0 + 0] = 2;
		_pOpponentPresense [3 * 0 + 2] = 2;
	}
	if (1 == _pOpponentPresense [3 * 1 + 0])
	{
		_pOpponentPresense [3 * 0 + 0] = 2;
		_pOpponentPresense [3 * 2 + 0] = 2;
	}
	if (1 == _pOpponentPresense [3 * 0 + 2])
	{
		_pOpponentPresense [3 * 0 + 1] = 2;
		_pOpponentPresense [3 * 1 + 2] = 2;
	}
	if (1 == _pOpponentPresense [3 * 2 + 0])
	{
		_pOpponentPresense [3 * 1 + 0] = 2;
		_pOpponentPresense [3 * 2 + 1] = 2;
	}
	if (1 == _pOpponentPresense [3 * 1 + 2])
	{
		_pOpponentPresense [3 * 0 + 2] = 2;
		_pOpponentPresense [3 * 2 + 2] = 2;
	}
	if (1 == _pOpponentPresense [3 * 2 + 1])
	{
		_pOpponentPresense [3 * 2 + 0] = 2;
		_pOpponentPresense [3 * 2 + 2] = 2;
	}
	if (1 == _pOpponentPresense [3 * 2 + 2])
	{
		_pOpponentPresense [3 * 1 + 2] = 2;
		_pOpponentPresense [3 * 2 + 1] = 2;
	}
}


//													
void MeasureStateNation (struct Message* _pResponse)
{
	ComputeWorldStats ();

	// player can change government only if a			
	// government change is not happening already...	
	struct government* pCurrentGov = government_of_player (client.conn.playing);
	char zGovName [100];
	*zGovName = '\0';
	if (NULL != pCurrentGov)
		strcpy (zGovName, government_name_translation (pCurrentGov));

	struct player_research* pResearch = get_player_research (client.conn.playing);
	char zTechs [10001];
	*zTechs = '\0';
	if (NULL != pResearch)
	{
		bool bFirst = true;
		int a;
		for (a = 1; a < advance_count (); ++ a)
		{
			struct advance* pAdvance = advance_by_number (a);
			if (NULL == pAdvance)
				continue;
			enum tech_state eState = pResearch->inventions [a].state;
			if (TECH_KNOWN != eState)
				continue;

			if (false == bFirst)
				strcat (zTechs, LCP_BOW_SEPARATOR);
			// strcat (zTechs, pAdvance->name.translated);
			strcat (zTechs, advance_name_translation (pAdvance));
			bFirst = false;
		}
		assert (strlen (zTechs) < 10000);
	}


	// check for anarchy/revolution	
	int iNotAnarchy = 1;
	if (government_of_player(client.conn.playing) == game.government_during_revolution) 
		iNotAnarchy = 0;


	//								
	char zGovFeatures [10001];
	sprintf (zGovFeatures, 
			 LCP_TYPE_MARKER "p" LCP_AGENT_TERMINATOR LCP_INFO_MARKER "%f;%f;%f;%f;%d;%d;%f;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%s;%s " LCP_AGENT_TERMINATOR, 
			 i_MySettledArea / (double)i_TotalArea,
			 i_MyKnownArea / (double)i_TotalArea,
			 // i_EnemySettledArea / (double)i_TotalArea,
			 // i_EnemyKnownArea / (double)i_TotalArea,
			 f_MyGameScore,
			 f_EnemyGameScore,
			 i_TotalCities,
			 i_TotalCitySize,
			 f_AverageCitySize,
			 i_TotalUnits,
			 i_TotalVeteranUnits,
			 i_TotalGold,
			 i_ExcessFood,
			 i_ExcessShield,
			 i_ExcessTrade,
			 i_ExcessScience,
			 i_ExcessGold,
			 i_ExcessLuxury,
			 iNotAnarchy,
			 // pCurrentGov->name.translated,
			 zGovName,
			 zTechs);
	assert (strlen (zGovFeatures) < 10000);
	append_to_message (_pResponse, zGovFeatures);

	if (NULL != client.conn.playing)
	{
		if (client.conn.playing->revolution_finishes < game.info.turn)
		{
			int g;
			for (g = 0; g < government_count (); ++ g)
			{
				struct government* pGov = government_by_number (g);
				if (NULL == pGov)
					continue;
				if (false == can_change_to_government (client.conn.playing, pGov))
					continue;

				char zText [1000];
				sprintf (zText, LCP_SET_GOV " %d"
						 LCP_COMMAND_TERMINATOR "%c" LCP_COMMAND_TERMINATOR
						 "%s" LCP_ACTION_TERMINATOR, 
						 g, ((pGov == pCurrentGov)? 'c':' '),
						 // pGov->name.translated);
						 government_name_translation (pGov));
				assert (strlen (zText) < 1000);

				append_to_message (_pResponse, zText);
			}
		}
		else
			append_to_message (_pResponse, 
								LCP_SET_GOV " -1" 
								LCP_COMMAND_TERMINATOR "c" LCP_COMMAND_TERMINATOR
								LCP_ACTION_TERMINATOR);
	}
	append_to_message (_pResponse, LCP_AGENT_TERMINATOR);



	// tech tree ...										

	// turns to research completion...					
	int iOurs, iTheirs;
	get_bulbs_per_turn (&iOurs, &iTheirs);
	int iTotal = iOurs + iTheirs;
	int iTotalBulbsRequired = total_bulbs_required (client.conn.playing);

	int iTurnsToCompletion = -1;
	if (iTotal > 0)
		iTurnsToCompletion = (iTotalBulbsRequired + iTotal - 1) / iTotal;
	
	// % completion values ...							
	struct player_research* pPlayerResearch = get_player_research (client.conn.playing);
	float fRatioComplete = 0;
	if (NULL != pPlayerResearch)
	{
		int iBulbsComplete = pPlayerResearch->bulbs_researched;
		if (iTotalBulbsRequired > 0)
		{
			fRatioComplete = iBulbsComplete / (float) iTotalBulbsRequired;
			if (fRatioComplete > 1)
				printf ("[ERROR] research ratio complete > 1 : %d / %d\n", iBulbsComplete, iTotalBulbsRequired);
		}
	}
	
	// % done, % remaining, turns remaining, tech bow	
	char zNameResearching [200];
	*zNameResearching = '\0';
	if (NULL != client.conn.playing)
		strcpy (zNameResearching, advance_name_researching (client.conn.playing));

	char zTechFeatures [10000];
	sprintf (zTechFeatures, 
			 LCP_TYPE_MARKER "q" LCP_AGENT_TERMINATOR
			 LCP_INFO_MARKER "%f;%d;%s" LCP_AGENT_TERMINATOR,
			 fRatioComplete, 
			 iTurnsToCompletion,
			 zNameResearching);

	append_to_message (_pResponse, zTechFeatures);
	assert (strlen (zTechFeatures) < 10000);


	// struct advance* pCurrentlyResearching = NULL;
	/* technology 0 is a dummy index for some reason unknown to me. */
	int a;
	for (a = 1; a < advance_count (); ++ a)
	{
		struct advance* pAdvance = advance_by_number (a);
		if (NULL == pAdvance)
			continue;
		// if (a == pResearch->researching)
		//	pCurrentlyResearching = pAdvance;

		enum tech_state eState = pResearch->inventions [a].state;

		if (TECH_PREREQS_KNOWN != eState)
			continue;

		char zText [1000];
		sprintf (zText, 
				 LCP_RESEARCH " %d"
				 LCP_COMMAND_TERMINATOR "%c" LCP_COMMAND_TERMINATOR
				 "%s" LCP_ACTION_TERMINATOR,
				 a, ((a == pResearch->researching)? 'c':' '),
				 // pAdvance->name.translated);
				 advance_name_translation (pAdvance));
		assert (strlen (zText) < 1000);

		append_to_message (_pResponse, zText);
	}

	/*
	if (NULL != pCurrentlyResearching)
	{
		char zText [1000];
		sprintf (zText, 
				 LCP_RESEARCH_CONTINUE " " 
				 LCP_COMMAND_TERMINATOR "c" LCP_COMMAND_TERMINATOR
				 "%s" LCP_ACTION_TERMINATOR,
				 pCurrentlyResearching->name.translated);
		assert (strlen (zText) < 1000);
		append_to_message (_pResponse, zText);
	}
	*/

	append_to_message (_pResponse, LCP_AGENT_TERMINATOR);
}


//													
void MeasureStateCities (struct Message* _pResponse)
{
	city_list_iterate(client.conn.playing->cities, pcity)
	{
		if (NULL == game_find_city_by_number (pcity->id))
			continue;

		// current city production...	
		char zCurrentlyBuilding [101];
		universal_name_translation (&pcity->production, zCurrentlyBuilding, 100);
		assert (strlen (zCurrentlyBuilding) < 100);

		// city governor				
		char zCityGov [101];
		strcpy (zCityGov, cmafec_get_short_descr_of_city (pcity));
		if (0 == strcmp ("none", zCityGov))
			*zCityGov = '\0';
		assert (strlen (zCityGov) < 100);

		float fDistanceToClosestCity = 0;
		float fAverageDistance = 0;
		DistanceToClosestCity (pcity->tile->x, pcity->tile->y, pcity,
								&fDistanceToClosestCity, &fAverageDistance);

		int iTurnsToGrow = city_turns_to_grow (pcity);
		if (FC_INFINITY == iTurnsToGrow)
			iTurnsToGrow = -1;

		int iTurnsToBuild = city_production_turns_to_build (pcity, TRUE);
		if (FC_INFINITY == iTurnsToBuild)
			iTurnsToBuild = -1;

		char zCityFeatures [10001];
		sprintf (zCityFeatures, 
				 LCP_TYPE_MARKER "b" LCP_AGENT_TERMINATOR LCP_INFO_MARKER 
				 // "%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%f;%f;%s;%s;%s" LCP_ACTION_TERMINATOR,
				 "%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%f;%f;%s;%s;%s" LCP_AGENT_TERMINATOR,
				 pcity->id,
				 pcity->tile->x,
				 pcity->tile->y,
				 pcity->size,
				 iTurnsToGrow,
				 pcity->food_stock,
				 pcity->shield_stock,
				 iTurnsToBuild,
				 pcity->surplus[0],
				 pcity->surplus[1],
				 pcity->surplus[2],
				 pcity->surplus[3],
				 pcity->surplus[4],
				 pcity->surplus[5],
				 fDistanceToClosestCity,
				 fAverageDistance,
				 zCityGov,
				 zCurrentlyBuilding,
				 GetCityImprovements (pcity));

		assert (strlen (zCityFeatures) < 10000);
		append_to_message (_pResponse, zCityFeatures);
		// append_to_message (_pResponse, LCP_AGENT_TERMINATOR);

		// city governor			
		int g;
		for (g = 0; g < cmafec_preset_num (); ++ g)
		{
			char zText [100];
			sprintf (zText, 
					 LCP_CITY_SET_GOV " %d %d" 
					 LCP_COMMAND_TERMINATOR " " LCP_COMMAND_TERMINATOR 
					 "%s" LCP_ACTION_TERMINATOR,
					 pcity->id, g, cmafec_preset_get_descr (g));
			assert (strlen (zText) < 100);
			append_to_message (_pResponse, zText);
		}

		append_to_message (_pResponse, LCP_AGENT_TERMINATOR);

		// build city improvement	
		int i;
		for (i = 0; i < improvement_count (); ++i)
		{
			struct universal oTarget = universal_by_number (VUT_IMPROVEMENT, i);
			if (false == can_city_build_now (pcity, oTarget))
				continue;
			char zName [101];
			universal_name_translation (&oTarget, zName, 100);
			assert (strlen (zName) < 100);
			char zText [101];
			sprintf (zText, 
					 LCP_CITY_BUILD_CHANGE " %d i%d"
					 LCP_COMMAND_TERMINATOR "%c" LCP_COMMAND_TERMINATOR
					 "city,build,%s" LCP_ACTION_TERMINATOR,
					 pcity->id, i, 
					 ((true == are_universals_equal (&oTarget, &pcity->production))? 'c':' '),
					 zName);
			assert (strlen (zText) < 100);
			append_to_message (_pResponse, zText);
		}

		// build unit 				
		int u;
		for (u = 0; u < game.control.num_unit_types; ++u)
		{
			struct universal oTarget = universal_by_number (VUT_UTYPE, u);
			if (false == can_city_build_now (pcity, oTarget))
				continue;
			char zName [101];
			universal_name_translation (&oTarget, zName, 100);
			assert (strlen (zName) < 100);
			char zText [101];
			sprintf (zText, 
					 LCP_CITY_BUILD_CHANGE " %d u%d"
					 LCP_COMMAND_TERMINATOR "%c" LCP_COMMAND_TERMINATOR
					 "city,build,%s" LCP_ACTION_TERMINATOR,
					 pcity->id, u,
					 ((true == are_universals_equal (&oTarget, &pcity->production))? 'c':' '),
					 zName);
			assert (strlen (zText) < 100);
			append_to_message (_pResponse, zText);
		}

		// check if the current production can be bought for gold ...
		if (city_production_buy_gold_cost (pcity) < i_TotalGold)
		{
			char zName [101];
			universal_name_translation (&pcity->production, zName, 100);
			assert (strlen (zName) < 100);
			char zText [101];
			sprintf (zText, 
					 LCP_CITY_BUILD_BUY " %d iu"
					 LCP_COMMAND_TERMINATOR " " LCP_COMMAND_TERMINATOR
					 "city,buy,%s" LCP_ACTION_TERMINATOR,
					 pcity->id, zName);
			assert (strlen (zText) < 100);
			append_to_message (_pResponse, zText);
		}
		
		append_to_message (_pResponse, LCP_AGENT_TERMINATOR);
	}
	city_list_iterate_end;
}


//													
void MeasureStateUnits (struct Message* _pResponse)
{
	unit_list_iterate(client.conn.playing->units, punit)
	{
		// This shouldn't be necessary as far as I know,	
		// but when a settler builds a city, this iteration	
		// still returns the unit on the next turn!			
		if (NULL == game_find_unit_by_number (punit->id))
			continue;

		bool bExtendedActivity = false;
		char zExtendedActivityName [101];
		*zExtendedActivityName = '\0';
		if (ACTIVITY_IRRIGATE == punit->activity)
		{
			bExtendedActivity = true;
			strcpy (zExtendedActivityName, "irrigate");
		}
		if (ACTIVITY_MINE == punit->activity)
		{
			bExtendedActivity = true;
			strcpy (zExtendedActivityName, "mine");
		}
		if (ACTIVITY_EXPLORE == punit->activity)
		{
			bExtendedActivity = true;
			strcpy (zExtendedActivityName, "explore,autoexplore");
		}
		if (ACTIVITY_ROAD == punit->activity)
		{
			bExtendedActivity = true;
			strcpy (zExtendedActivityName, "build,road");
		}
		if (ACTIVITY_RAILROAD == punit->activity)
		{
			bExtendedActivity = true;
			strcpy (zExtendedActivityName, "build,railroad");
		}
		if (ACTIVITY_TRANSFORM == punit->activity)
		{
			bExtendedActivity = true;
			strcpy (zExtendedActivityName, "transform");
		}
		if (AIUNIT_AUTO_SETTLER == punit->ai.ai_role)
		{
			bExtendedActivity = true;
			strcpy (zExtendedActivityName, "autosettle");
		}
		if (AIUNIT_EXPLORE == punit->ai.ai_role)
		{
			bExtendedActivity = true;
			strcpy (zExtendedActivityName, "explore,autoexplore");
		}
		assert (strlen (zExtendedActivityName) < 100);

		int iCityId = -1;
		struct city* pCity = GetTileCity (punit->tile);
		if (NULL != pCity)
			iCityId = pCity->id;

		char zUnitName [1001];
		// strcpy (zUnitName, punit->utype->name.translated);
		strcpy (zUnitName, unit_name_translation (punit));
		/*
		if (0 != punit->veteran)
		{
			char zTemp [100];
			sprintf (zTemp, ",%s-vtrn", punit->utype->name.translated);
			assert (strlen (zTemp) < 100);
			strcat (zUnitName, zTemp);
		}
		*/
		/*
		if (-1 != iCityId)
		{
			char zTemp [100];
			sprintf (zTemp, ",%s-city", punit->utype->name.translated);
			assert (strlen (zTemp) < 100);
			strcat (zUnitName, zTemp);
		}
		*/
		assert (strlen (zUnitName) < 1000);

		float fDistanceToClosestCity = 0;
		float fAverageDistance = 0;
		DistanceToClosestCity (punit->tile->x, punit->tile->y, NULL,
								&fDistanceToClosestCity, &fAverageDistance);

		// current location labels...	
		const char* zTileLabels = GetTileLabels (punit->tile->x, punit->tile->y, punit);
	
		char zText [10001];
		sprintf (zText, LCP_TYPE_MARKER "u" LCP_AGENT_TERMINATOR LCP_INFO_MARKER
				// "%d;%d;%d;%d;%d;%d;%d;%f;%f;%d;%s;%s;%s" LCP_ACTION_TERMINATOR,
				"%d;%d;%d;%d;%d;%d;%d;%f;%f;%d;%s;%s;%s" LCP_AGENT_TERMINATOR,
				punit->id, 
				punit->tile->x,
				punit->tile->y,
				punit->moves_left,
				punit->hp,
				punit->utype->hp,
				punit->veteran,
				fDistanceToClosestCity,
				fAverageDistance,
				iCityId,
				zUnitName,
				get_activity_text (punit->activity),
				zTileLabels);

		assert (strlen (zText) < 10000);
		append_to_message (_pResponse, zText);
		// append_to_message (_pResponse, LCP_AGENT_TERMINATOR);


		// unit actions ...	
		struct unit_list* plstUnit = unit_list_new ();
		unit_list_append (plstUnit, punit);
		enter_goto_state (plstUnit);

		int d1, d2;
		struct pf_parameter oParam;
		struct pf_map* pPathMap;
		fill_client_goto_parameter (punit, &oParam, &d1, &d2);
		pPathMap = pf_map_new (&oParam);

		GetUnitGotoActions (_pResponse, 
							punit->id,
							// punit->utype->name.translated,
							unit_name_translation (punit),
							pPathMap,
							punit->tile->x,
							punit->tile->y);
		pf_map_destroy (pPathMap);

		exit_goto_state ();
		unit_list_clear (plstUnit);
		unit_list_free (plstUnit);

		bool bInCityCenter = IsCityCenter (punit->tile->x, punit->tile->y);
		if (true == bExtendedActivity)
		{
			sprintf (zText, LCP_UNIT_CONTINUE " %d" 
							LCP_COMMAND_TERMINATOR "c" LCP_COMMAND_TERMINATOR
					 		"%s" LCP_ACTION_TERMINATOR, punit->id, zExtendedActivityName);
			append_to_message (_pResponse, zText);
		}
		/**/
		if (true == can_unit_do_activity (punit, ACTIVITY_EXPLORE))
		{
			sprintf (zText, LCP_UNIT_AUTOEXPLORE " %d" 
							LCP_COMMAND_TERMINATOR " " LCP_COMMAND_TERMINATOR 
							"explore,autoexplore" LCP_ACTION_TERMINATOR, punit->id);
			append_to_message (_pResponse, zText);
		}
		/**/
		/**/
		if (true == can_unit_do_autosettlers (punit))
		{
			sprintf (zText, LCP_UNIT_AUTOSETTLE " %d"
							LCP_COMMAND_TERMINATOR " " LCP_COMMAND_TERMINATOR
							"autosettle" LCP_ACTION_TERMINATOR, punit->id);
			append_to_message (_pResponse, zText);
		}
		/**/
		if (true == can_unit_build_city (punit))
		{
			sprintf (zText, LCP_UNIT_BUILDCITY " %d"
							LCP_COMMAND_TERMINATOR " " LCP_COMMAND_TERMINATOR
							"build,city" LCP_ACTION_TERMINATOR, punit->id);
			append_to_message (_pResponse, zText);
		}
		if (true == can_unit_do_activity (punit, ACTIVITY_FORTIFIED))
		{
			sprintf (zText, LCP_UNIT_FORTIFY " %d" 
							LCP_COMMAND_TERMINATOR " " LCP_COMMAND_TERMINATOR
							"fortify" LCP_ACTION_TERMINATOR, punit->id);
			append_to_message (_pResponse, zText);
		}
		// sprintf (zText, LCP_UNIT_SETHOME " %d" LCP_ACTION_TERMINATOR, punit->id);
		// append_to_message (_pResponse, zText);

		if ((true == can_unit_do_activity (punit, ACTIVITY_IRRIGATE)) &&
			(false == bInCityCenter))
		{
			sprintf (zText, LCP_UNIT_IRRIGATE " %d"
							LCP_COMMAND_TERMINATOR " " LCP_COMMAND_TERMINATOR
							"irrigate" LCP_ACTION_TERMINATOR, punit->id);
			append_to_message (_pResponse, zText);
		}
		if ((true == can_unit_do_activity (punit, ACTIVITY_MINE)) &&
			(false == bInCityCenter))
		{
			sprintf (zText, LCP_UNIT_MINE " %d" 
							LCP_COMMAND_TERMINATOR " " LCP_COMMAND_TERMINATOR
							"mine" LCP_ACTION_TERMINATOR, punit->id);
			append_to_message (_pResponse, zText);
		}
		if (((true == can_unit_do_activity (punit, ACTIVITY_ROAD)) ||
			(true == can_unit_do_activity (punit, ACTIVITY_RAILROAD))) &&
			(false == bInCityCenter))
		{
			sprintf (zText, LCP_UNIT_BUILDROAD " %d" 
							LCP_COMMAND_TERMINATOR " " LCP_COMMAND_TERMINATOR
							"build,road,railroad" LCP_ACTION_TERMINATOR, punit->id);
			append_to_message (_pResponse, zText);
		}
		if (true == can_unit_do_activity (punit, ACTIVITY_SENTRY))
		{
			sprintf (zText, LCP_UNIT_SENTRY " %d" 
							LCP_COMMAND_TERMINATOR " " LCP_COMMAND_TERMINATOR
							"sentry" LCP_ACTION_TERMINATOR, punit->id);
			append_to_message (_pResponse, zText);
		}
		if (true == can_unit_do_activity (punit, ACTIVITY_TRANSFORM))
		{
			sprintf (zText, LCP_UNIT_TRANSFORM " %d" 
							LCP_COMMAND_TERMINATOR " " LCP_COMMAND_TERMINATOR 
							"transform" LCP_ACTION_TERMINATOR, punit->id);
			append_to_message (_pResponse, zText);
		}

		append_to_message (_pResponse, LCP_AGENT_TERMINATOR);
	}
	unit_list_iterate_end;
}

//													
void MeasureState (struct Message* _pResponse)
{
	MeasureStateNation (_pResponse);
	MeasureStateCities (_pResponse);
	MeasureStateUnits (_pResponse);
	append_to_message (_pResponse, LCP_TERMINATOR);
}



//													
void UnitGotoMI (struct Message* _pResponse, struct unit* _pUnit, int _x, int _y)
{
	struct tile* pTile = native_pos_to_tile (_x, _y);
	if (NULL == pTile)
	{
		char zError [1000];
		sprintf (zError, "[ERROR] Goto command to invalid location (%d,%d)\n", _x, _y);
		assert (strlen (zError) < 100);
		append_to_message (_pResponse, zError);
		append_to_message (_pResponse, "." LCP_TERMINATOR);
		append_to_message (_pResponse, "." LCP_TERMINATOR);
		return;
	}
	if (pTile == _pUnit->tile)
	{
		char zError [1000];
		sprintf (zError, "Already at (%d,%d)\n", _x, _y);
		assert (strlen (zError) < 100);
		append_to_message (_pResponse, zError);
		append_to_message (_pResponse, LCP_TERMINATOR);
		return;
	}
	set_unit_focus (_pUnit);
	keyboardless_goto_active = TRUE;
	request_unit_goto (ORDER_LAST);

	do_unit_goto (pTile);
	set_hover_state (NULL, HOVER_NONE, ACTIVITY_LAST, ORDER_LAST);
	keyboardless_goto_active = FALSE;
}


//													
void UnitMoveMI (struct Message* _pResponse, struct unit* _pUnit, const char* _zDirection)
{
	enum direction8 dirMove = NameToDirection (_zDirection);
	if (-1 == dirMove)
	{
		char zError [1000];
		sprintf (zError, "[ERROR] Unknown direction [%s]\n", _zDirection);
		assert (strlen (zError) < 100);
		append_to_message (_pResponse, zError);
		append_to_message (_pResponse, "." LCP_TERMINATOR);
		append_to_message (_pResponse, "." LCP_TERMINATOR);
		return;
	}

	set_unit_focus (_pUnit);
	key_unit_move (dirMove);
}



//													
void SetCityBuildMI (struct Message* _pResponse, struct city* _pCity, const char* _zItem)
{
	enum universals_n eBuildType = VUT_NONE;
	if ('u' == *_zItem)
		eBuildType = VUT_UTYPE;
	else if ('i' == *_zItem)
		eBuildType = VUT_IMPROVEMENT;

	if (VUT_NONE == eBuildType)
	{
		char zError [1000];
		sprintf (zError, "[ERROR] Unknown build item [%s]\n", _zItem);
		assert (strlen (zError) < 100);
		append_to_message (_pResponse, zError);
		append_to_message (_pResponse, "." LCP_TERMINATOR);
		append_to_message (_pResponse, "." LCP_TERMINATOR);
		return;
	}

	struct universal oTarget = universal_by_number (eBuildType, atoi (_zItem + 1));
	if (false == are_universals_equal (&oTarget, &_pCity->production))
		city_change_production (_pCity, oTarget);
}


//													
void SetCityBuyMI (struct Message* _pResponse, struct city* _pCity, const char* _zItem)
{
	city_buy_production (_pCity);
}


//													
void SetCityGovernorMI (struct Message* _pResponse, struct city* _pCity, int _iGovernor)
{
	if (-1 == _iGovernor)
	{
		cma_release_city (_pCity);
		return;
	}

	if (_iGovernor > cmafec_preset_num ())
		return;
	const struct cm_parameter* pGovernor = cmafec_preset_get_parameter (_iGovernor);
	if (NULL == pGovernor)
	{
		char zError [1000];
		sprintf (zError, "[ERROR] Unknown city governor type [%d]\n", _iGovernor);
		assert (strlen (zError) < 100);
		append_to_message (_pResponse, zError);
		append_to_message (_pResponse, "." LCP_TERMINATOR);
		append_to_message (_pResponse, "." LCP_TERMINATOR);
		return;
	}

	cmafec_set_fe_parameter (_pCity, pGovernor);

	if (true == cma_is_city_under_agent (_pCity, NULL))
		cma_release_city (_pCity);
	cma_put_city_under_agent (_pCity, pGovernor);
}








