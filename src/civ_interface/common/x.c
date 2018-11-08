bool is_req_active_3(const struct player *target_player,
		const struct requirement *req,
		const enum   req_problem_type prob_type)
{
	bool eval = FALSE;

	/* Note the target may actually not exist.  In particular, effects that
	 * have a VUT_SPECIAL or VUT_TERRAIN may often be passed to this function
	 * with a city as their target.  In this case the requirement is simply
	 * not met. */
	switch (req->source.kind) {
		case VUT_NONE:
			eval = TRUE;
			break;
		case VUT_ADVANCE:
			/* The requirement is filled if the player owns the tech. */
			eval = is_tech_in_range(target_player, req->range,
					advance_number(req->source.value.advance),
					prob_type);
			break;
		case VUT_GOVERNMENT:
			/* The requirement is filled if the player is using the government. */
			eval = (government_of_player(target_player) == req->source.value.govern);
			break;
		case VUT_IMPROVEMENT:
			/* The requirement is filled if there's at least one of the building
			 * in the city.  (This is a slightly nonstandard use of
			 * count_sources_in_range.) */
			abort ();
			eval = (count_buildings_in_range(target_player, NULL, NULL,
						req->range, req->survives,
						req->source.value.building) > 0);
			break;
		case VUT_SPECIAL:
			abort ();
			eval = is_special_in_range(NULL,
					req->range, req->survives,
					req->source.value.special);
			break;
		case VUT_TERRAIN:
			abort ();
			eval = is_terrain_in_range(NULL,
					req->range, req->survives,
					req->source.value.terrain);
			break;
		case VUT_NATION:
			eval = is_nation_in_range(target_player, req->range, req->survives,
					req->source.value.nation);
			break;
		case VUT_UTYPE:
			abort ();
			eval = is_unittype_in_range(NULL,
					req->range, req->survives,
					req->source.value.utype);
			break;
		case VUT_UTFLAG:
			abort ();
			eval = is_unitflag_in_range(NULL,
					req->range, req->survives,
					req->source.value.unitflag,
					prob_type);
			break;
		case VUT_UCLASS:
			abort ();
			eval = is_unitclass_in_range(NULL,
					req->range, req->survives,
					req->source.value.uclass);
			break;
		case VUT_UCFLAG:
			abort ();
			eval = is_unitclassflag_in_range(NULL,
					req->range, req->survives,
					req->source.value.unitclassflag);
			break;
		case VUT_OTYPE:
			abort ();
			eval = FALSE; // (target_output && target_output->index == req->source.value.outputtype);
			break;
		case VUT_SPECIALIST:
			abort ();
			eval = FALSE; //(target_specialist && target_specialist == req->source.value.specialist);
			break;
		case VUT_MINSIZE:
			abort ();
			eval = FALSE; //target_city && target_city->size >= req->source.value.minsize;
			break;
		case VUT_AI_LEVEL:
			eval = target_player
				&& target_player->ai_data.control
				&& target_player->ai_data.skill_level == req->source.value.ai_level;
			break;
		case VUT_TERRAINCLASS:
			abort ();
			eval = is_terrain_class_in_range(NULL,
					req->range, req->survives,
					req->source.value.terrainclass);
			break;
		case VUT_BASE:
			abort ();
			eval = is_base_type_in_range(NULL,
					req->range, req->survives,
					req->source.value.base);
			break;
		case VUT_MINYEAR:
			eval = game.info.year >= req->source.value.minyear;
			break;
		case VUT_TERRAINALTER:
			abort ();
			eval = is_terrain_alter_possible_in_range(NULL,
					req->range, req->survives,
					req->source.value.terrainalter);
			break;
		case VUT_CITYTILE:
			if (NULL) {
				if (req->source.value.citytile == CITYT_CENTER) {
					if (NULL) {
						eval = is_city_center(NULL, NULL);
					} else {
						eval = tile_city(NULL) != NULL;
					}
				} else {
					/* Not implemented */
					freelog(LOG_ERROR, "is_req_active(): citytile %d not supported.",
							req->source.value.citytile);
					return FALSE;
				}
			} else {
				eval = FALSE;
			}
			break;
		case VUT_LAST:
			freelog(LOG_ERROR, "is_req_active(): invalid source kind %d.",
					req->source.kind);
			return FALSE;
	}

	if (req->negated) {
		return !eval;
	} else {
		return eval;
	}
}


