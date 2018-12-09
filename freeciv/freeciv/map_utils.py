# this is he mapformat for the maps in freeciv game

map_topology = {
	"Glacier" : "a",
	"Deep Ocean" : ":",
	"Desert" : "d",
	"Forest" : "f",
	"Plains" : "p",
	"Grasslands" : "g",
	"Hills" : "h",
	"Jungle" : "j",
	"Lake" : "+",
	"Mountains" : "m",
	"Ocean" : " " # Space,
	"Swamp" : "s",
	"Tundra" : "t"
}

map_resources = {
	"buffalo" : "b",
	"coal" : "c",
	"fish" : "y",
	"fruit" : "f",
	"furs" : "u",
	"game" : "e",
	"gems" : "g",
	"gold" : "$",
	"iron" : "/",
	"ivory" : "i",
	"oasis" : "o",
	"oil" : "x",
	"peat" : "a",
	"pheasant" : "p",
	"resources" : "r",
	"silk" : "s",
	"spice" : "t",
	"whale" : "v",
	"wheat" : "j",
	"wine" : "w"
}

'''
All the topology in the map is given under the tag

t_XXXXXX list of all the topology in the map, codes given above

bXX_XXX list of all bases located on the map (1=Fortress, 2= Airbase, 3=Both)
spe_XX_XXXX list of all specials located on the map
resXXXX list of all resources located on the map
ownerXXXX owner of the tiles (border information; player number)
sourceXXXX list of all border sources
workedXXXX tile is work by the city with the given ID
kXX_XXXX last seen information, player knowledge


'''