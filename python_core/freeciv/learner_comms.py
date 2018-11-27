# this has the communication dictionary for the actions in the game

comms = {
	"LCP_OBSERVE" :				"*"

	"LCP_CONNECT" :				"n"
	"LCP_DISCONNECT" :			"d"
	"LCP_SAVEGAME" :			"s"
	"LCP_LOADGAME" :			"l"
	"LCP_MOD_SAVEGAME" :		"m"
	"LCP_ENDGAME" :				"e"
	"LCP_GAMESCORE" :			"g"
	"LCP_GAMEFINISHED" :		"f"
	"LCP_ENABLE_EARLY_END" :	"y"
	"LCP_SET_TIMEOUT" :			"t"
	"LCP_TURNDONE" :			">"

	"LCP_NATION_COMMAND" :		"p"
	"LCP_SET_GOV" :				"pg"
	"LCP_RESEARCH" :			"qs"
	"LCP_RESEARCH_CONTINUE" :	"qc"

	"LCP_UNIT_COMMAND" :		"u"
	"LCP_UNIT_CONTINUE" :		"uc"
	"LCP_UNIT_MOVE" :			"uo"
	"LCP_UNIT_GOTO" :			"ug"
	"LCP_UNIT_AUTOEXPLORE" :	"ux"
	"LCP_UNIT_AUTOSETTLE" :		"ua"
	"LCP_UNIT_BUILDCITY" :		"ub"
	"LCP_UNIT_FORTIFY" :		"uf"
	"LCP_UNIT_SETHOME" :		"uh"
	"LCP_UNIT_IRRIGATE" :		"ui"
	"LCP_UNIT_MINE" :			"um"
	"LCP_UNIT_BUILDROAD" :		"ur"
	"LCP_UNIT_SENTRY" :			"us"
	"LCP_UNIT_TRANSFORM" :		"ut"

	"LCP_CITY_COMMAND" :		"b"
	"LCP_CITY_SET_GOV" :		"cg"
	"LCP_CITY_BUILD_CHANGE" :	"bs"
	"LCP_CITY_BUILD_BUY" :		"bb"
	"LCP_CITY_BUILD_CONTINUE" :	"bc"

	"LCP_FEATURE_SEPARATOR" :	";"
	"LCP_BOW_SEPARATOR" :		","

	"LCP_TYPE_MARKER" :			"@"
	"LCP_INFO_MARKER" :			":"
	"LCP_TERMINATOR" :			"\x05"

	"LCP_AGENT_TERMINATOR" :	"\x03"
	"LCP_ACTION_TERMINATOR" :	"\x02"
	"LCP_COMMAND_TERMINATOR" :	"\x01"
}