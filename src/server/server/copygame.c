//																		
void save_resource (void)
{
	memcpy (last_load_civ_resources, civ_resources, MAX_NUM_RESOURCES * sizeof (resource));
}

void load_resource (void)
{
	memcpy (civ_resources, last_load_civ_resources, MAX_NUM_RESOURCES * sizeof (resource));
}


//																		
void save_terrain (void)
{
	// struct name_translation name; ????? might cause crashes...

	memcpy (last_load_civ_terrain, civ_terrain, MAX_NUM_TERRAINS * sizeof (terrain));

	for (int t = 0; t < MAX_NUM_TERRAINS; ++ t)
	{
		struct terrain* pTo = &last_load_civ_terrain [t];
		struct terrain* pFrom = &civ_terrain [t];

		// result terrain ...	
		pTo->irrigation_result =
			last_load_civ_terrain [terrain_number (pFrom->irrigation_result)];
		pTo->mining_result =
			last_load_civ_terrain [terrain_number (pFrom->mining_result)];
		pTo->transform_result = 
			last_load_civ_terrain [terrain_number (pFrom->transform_result)];
		pTo->warmer_wetter_result = 
			last_load_civ_terrain [terrain_number (pFrom->warmer_wetter_result)];
		pTo->warmer_drier_result =
			last_load_civ_terrain [terrain_number (pFrom->warmer_drier_result)];
		pTo->cooler_wetter_result =
			last_load_civ_terrain [terrain_number (pFrom->cooler_wetter_result)];
		pTo->cooler_drier_result = 
			last_load_civ_terrain [terrain_number (pFrom->cooler_drier_result)];

		// resources ...		
		if (NULL != pTo->resources)
			free (pTo->resources);

		pTo->resources = malloc ((1 + pTo->resource_count) * sizeof (*pTo->resources));
		for (int i = 0; i < pTo->resource_count; ++ i)
		{
			Resource_type_id idRes = resource_number (pFrom->resources [i]);
			pTo->resources [i] = last_load_civ_resources [idRes];
		}
		pTo->resources [pTo->resource_count] = NULL;
	}
}


//																		
void load_terrain (void)
{
	// struct name_translation name; ????? might cause crashes...

	memcpy (civ_terrain, last_load_civ_terrain, MAX_NUM_TERRAINS * sizeof (terrain));

	for (int t = 0; t < MAX_NUM_TERRAINS; ++ t)
	{
		struct terrain* pTo = &civ_terrain [t];
		struct terrain* pFrom = &last_load_civ_terrain [t];

		// result terrain ...	
		pTo->irrigation_result =
			civ_terrain [pFrom->irrigation_result.item_number];
		pTo->mining_result =
			civ_terrain [pFrom->mining_result.item_number];
		pTo->transform_result = 
			civ_terrain [pFrom->transform_result.item_number];
		pTo->warmer_wetter_result = 
			civ_terrain [pFrom->warmer_wetter_result.item_number];
		pTo->warmer_drier_result =
			civ_terrain [pFrom->warmer_drier_result.item_number];
		pTo->cooler_wetter_result =
			civ_terrain [pFrom->cooler_wetter_result.item_number];
		pTo->cooler_drier_result = 
			civ_terrain [pFrom->cooler_drier_result.item_number];

		// resources ...		
		if (NULL != pTo->resources)
			free (pTo->resources);

		pTo->resources = malloc ((1 + pTo->resource_count) * sizeof (*pTo->resources));
		for (int i = 0; i < pTo->resource_count; ++ i)
		{
			Resource_type_id idRes = pFrom->resources [i].item_number;
			pTo->resources [i] = civ_resources [idRes];
		}
		pTo->resources [pTo->resource_count] = NULL;
	}
}


//																		
void copy_tile (struct tile* _pTo, struct tile* _pFrom)
{
	_pTo->x = _pFrom->x;
	_pTo->y = _pFrom->y;
	_pTo->nat_x = _pFrom->nat_x;
	_pTo->nat_y = _pFrom->nat_y;
	_pTo->index = _pFrom->index;
	_pTo->continent = _pFrom->continent;

	memcpy (_pTo->tile_known, _pFrom->tile_known, sizeof(bv_player));
	memcpy (_pTo->tile_seen, _pFrom->tile_seen, V_COUNT * sizeof(bv_player));
	memcpy (_pTo->special, _pFrom->special, sizeof(bv_special));
	memcpy (_pTo->bases, _pFrom->bases, sizeof(bv_bases));
}


//																		
void save_tile (struct tile* _pTo, struct tile* _pFrom)
{
	copy_tile (_pTo, _pFrom);
	
	// check if resource can be a vector ...
	_pTo->resource = (NULL == _pFrom->resource)?  NULL :
					 last_load_civ_resources [_pFrom->resource->item_number];
	_pTo->terrain = (NULL == _pFrom->terrain)?  NULL :
					 last_load_civ_terrain [_pFrom->terrain->item_number];

	// these need to be set after cities & units are loaded	
	_pTo->units = NULL;
	_pTo->worked = NULL;
	_pTo->owner = NULL;
	_pTo->claimer = NULL;
	_pTo->spec_sprite = NULL;
	struct tile *claimer;
}


//																		
void load_tile (struct tile* _pTo, struct tile* _pFrom)
{
	copy_tile (_pTo, _pFrom);
	
	// check if resource can be a vector ...
	_pTo->resource = (NULL == _pFrom->resource)?  NULL :
					 civ_resources [_pFrom->resource->item_number];
	_pTo->terrain = (NULL == _pFrom->terrain)?  NULL :
					 civ_terrain [_pFrom->terrain->item_number];

	// these need to be set after cities & units are loaded	
	_pTo->units = NULL;
	_pTo->worked = NULL;
	_pTo->owner = NULL;
	_pTo->claimer = NULL;
	_pTo->spec_sprite = NULL;
}


//																		
void save_tile_units_and_cities (struct tile* _pTo, struct tile* _pFrom)
{
	if (NULL != _pTo->units)
		unit_list_free (_pTo->units);
	_pTo->units = unit_list_new ();
	unit_list_iterate (_pFrom->units, pUnitFrom)
	{
		unit_list_append (_pTo->units, last_loaded_units [pUnitFrom->id]);
	}
	unit_list_iterate_end;


	_pTo->worked = (NULL == _pFrom->worked)? NULL :
					last_loaded_cities [_pFrom->worked->id];
	_pTo->owner = (NULL == _pFrom->owner)? NULL :
					last_loaded_game.players [_pFrom->owner - game.players];
	_pTo->claimer = (NULL == _pFrom->claimer)? NULL :
					(last_loaded_map + _pFrom->claimer->index);

	// might cause crash...
	_pTo->spec_sprite = NULL;
}


//																		
void load_tile_units_and_cities (struct tile* _pTo, struct tile* _pFrom)
{
	if (NULL != _pTo->units)
		unit_list_free (_pTo->units);
	_pTo->units = unit_list_new ();
	unit_list_iterate (_pFrom->units, pUnitFrom)
	{
		unit_list_append (_pTo->units, idex_lookup_unit (pUnitFrom->id));
	}
	unit_list_iterate_end;


	_pTo->worked = (NULL == _pFrom->worked)? NULL :
					idex_lookup_city (_pFrom->worked->id);
	_pTo->owner = (NULL == _pFrom->owner)? NULL :
					game.players [_pFrom->owner - last_loaded_game.players];
	_pTo->claimer = (NULL == _pFrom->claimer)? NULL :
					(map + _pFrom->claimer->index);

	// might cause crash...
	_pTo->spec_sprite = NULL;
}


//																		
void save_map (void)
{
	civ_map* pTo = &last_loaded_map;
	civ_map* pFrom = &map;

	struct tile* pTileTo = pTo->tiles;
	if (NULL != pTo->start_positions)
		free (pTo->start_positions);
	if (NULL != pTo->startpos_table)
		hash_free (pTo->startpos_table);


	// copy all trivial data	
	memcpy (last_loaded_map, map, sizeof (civ_map));


	// tiles					
	pTo->tiles = pTileTo;
	for (int i = 0; i < MAP_INDEX_SIZE; ++ i)
		save_tile (pTo->tiles + i, pFrom->tiles + i);


	// start_positions			
	pTo->start_positions =
		malloc (pTo->num_start_positions * sizeof (*pTo->start_positions));
	for (int i = 0; i < pTo->num_start_positions; ++ i)
	{
		pTo->start_positions [i].tile = 
			(NULL == pFrom->start_positions [i].tile)? NULL :
					(last_loaded_map + pFrom->start_positions [i]->index);
		pTo->start_positions [i].nation =
			last_loaded_nations [pFrom->start_positions [i].nation->item_number];
	}

	// startpos_table			
	pTo->startpos_table = 
		 hash_new_full(hash_fval_int, hash_fcmp_int, NULL, fc_free);
	unsigned int iEntries = hash_num_entries (pFrom->startpos_table);
	for (unsigned int i = 0; i < iEntries; ++ )
	{
		struct startpos_entry* spe;
		spe = fc_calloc (1, sizeof (*spe));
		spe->key = hash_key_by_number (pFrom->startpos_table, i);
		struct nation* pNation = hash_value_by_number (pFrom->startpos_table, i);
		spe->nation = last_loaded_nations [pNation->item_number];

		hash_insert (pTo->startpos_table, FC_INT_TO_PTR (spe->key), spe);
	}
}


//																		
void load_map (void)
{
	civ_map* pTo = &map;
	civ_map* pFrom = &last_loaded_map;

	struct tile* pTileTo = pTo->tiles;
	if (NULL != pTo->start_positions)
		free (pTo->start_positions);
	if (NULL != pTo->startpos_table)
		hash_free (pTo->startpos_table);


	// copy all trivial data	
	memcpy (map, last_loaded_map, sizeof (civ_map));


	// tiles					
	pTo->tiles = pTileTo;
	for (int i = 0; i < MAP_INDEX_SIZE; ++ i)
		save_tile (pTo->tiles + i, pFrom->tiles + i);


	// start_positions			
	pTo->start_positions =
		malloc (pTo->num_start_positions * sizeof (*pTo->start_positions));
	for (int i = 0; i < pTo->num_start_positions; ++ i)
	{
		pTo->start_positions [i].tile = 
			(NULL == pFrom->start_positions [i].tile)? NULL :
					(map + pFrom->start_positions [i]->index);
		pTo->start_positions [i].nation =
			nations [pFrom->start_positions [i].nation->item_number];
	}

	// startpos_table			
	pTo->startpos_table = 
		 hash_new_full(hash_fval_int, hash_fcmp_int, NULL, fc_free);
	unsigned int iEntries = hash_num_entries (pFrom->startpos_table);
	for (unsigned int i = 0; i < iEntries; ++ )
	{
		struct startpos_entry* spe;
		spe = fc_calloc (1, sizeof (*spe));
		spe->key = hash_key_by_number (pFrom->startpos_table, i);
		struct nation* pNation = hash_value_by_number (pFrom->startpos_table, i);
		spe->nation = nations [pNation->item_number];

		hash_insert (pTo->startpos_table, FC_INT_TO_PTR (spe->key), spe);
	}
}


void save_to_memory (void)
{
	save_resource ();
	save_terrain ();
	save_map ();

	save_players ();
	save_cities ();
	save_units ();
	save_tile_units_and_cities ();
}



