#ifndef __LEARNER_COMMS__
#define __LEARNER_COMMS__


#define LCP_OBSERVE				"*"

#define LCP_CONNECT				"n"
#define LCP_DISCONNECT			"d"
#define LCP_SAVEGAME			"s"
#define LCP_LOADGAME			"l"
#define LCP_MOD_SAVEGAME		"m"
#define LCP_ENDGAME				"e"
#define LCP_GAMESCORE			"g"
#define LCP_GAMEFINISHED		"f"
#define LCP_ENABLE_EARLY_END	"y"
#define LCP_SET_TIMEOUT			"t"
#define LCP_TURNDONE			">"

#define LCP_NATION_COMMAND		'p'
#define LCP_SET_GOV				"pg"
#define LCP_RESEARCH			"qs"
#define LCP_RESEARCH_CONTINUE	"qc"

#define LCP_UNIT_COMMAND		'u'
#define LCP_UNIT_CONTINUE		"uc"
#define LCP_UNIT_MOVE			"uo"
#define LCP_UNIT_GOTO			"ug"
#define LCP_UNIT_AUTOEXPLORE	"ux"
#define LCP_UNIT_AUTOSETTLE		"ua"
#define LCP_UNIT_BUILDCITY		"ub"
#define LCP_UNIT_FORTIFY		"uf"
#define LCP_UNIT_SETHOME		"uh"
#define LCP_UNIT_IRRIGATE		"ui"
#define LCP_UNIT_MINE			"um"
#define LCP_UNIT_BUILDROAD		"ur"
#define LCP_UNIT_SENTRY			"us"
#define LCP_UNIT_TRANSFORM		"ut"

#define LCP_CITY_COMMAND		'b'
#define LCP_CITY_SET_GOV		"cg"
#define LCP_CITY_BUILD_CHANGE	"bs"
#define LCP_CITY_BUILD_BUY		"bb"
#define LCP_CITY_BUILD_CONTINUE	"bc"

#define LCP_FEATURE_SEPARATOR	';'
#define LCP_BOW_SEPARATOR		','

#define LCP_TYPE_MARKER			'@'
#define LCP_INFO_MARKER			':'
#define LCP_TERMINATOR			'\x05'

#define LCP_AGENT_TERMINATOR	'\x03'
#define LCP_ACTION_TERMINATOR	'\x02'
#define LCP_COMMAND_TERMINATOR	'\x01'


#endif
