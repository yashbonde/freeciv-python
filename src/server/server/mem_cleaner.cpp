#include <set>
#include <map>
#include <iostream>
#include <nlp_string.h>
using namespace std;


// typedef set<void*>			Allocation_set_t;
// Allocation_set_t			set_Allocations;
typedef map<void*, String>	Allocation_map_t;
Allocation_map_t			map_Allocations;


extern "C"{
#include "mem.h"

bool b_RememberAllocations;

//														
void start_allocation_watch (void)
{
	map_Allocations.clear ();
	b_RememberAllocations = true;
}


//														
void end_allocation_watch (void)
{
	b_RememberAllocations = false;
	cout << "[mem watch] " << map_Allocations.size () << " known allocations." << endl;
}


//														
bool known_memory (void* _pMem)
{
	return (map_Allocations.end () != map_Allocations.find (_pMem));
}


//														
void remember_allocation_if_in_watch_mode (void* _pMem, int _iLine, const char* _zFile)
{
	if (true == b_RememberAllocations)
	{
		String sLocation;
		sLocation << _zFile << ":" << _iLine;
		map_Allocations.insert (make_pair (_pMem, sLocation));
	}
}


//														
void forget_allocation_if_in_watch_mode (void* _pMem)
{
	map_Allocations.erase (_pMem);
}


//														
void change_allocation_if_in_watch_mode (void* _pFrom, void* _pTo, int _iLine, const char* _zFile)
{
	if (false == b_RememberAllocations)
		return;

	if (_pFrom == _pTo)
		return;
	if (0 != map_Allocations.erase (_pFrom))
	{
		String sLocation;
		sLocation << _zFile << ":" << _iLine;
		map_Allocations.insert (make_pair (_pTo, sLocation));
	}
}


//														
void free_known_allocs (void)
{
	cout << "[mem watch] freeing " << map_Allocations.size () << " known allocations." << endl;
	Allocation_map_t::iterator	ite = map_Allocations.begin ();
	Allocation_map_t::iterator	ite_end = map_Allocations.end ();
	for (; ite != ite_end; ++ ite)
	{
		free (ite->first);
		cout << ite->first << "   " << ite->second << endl;
	}
	map_Allocations.clear ();
}


}
