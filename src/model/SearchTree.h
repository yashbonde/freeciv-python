#ifndef __SEARCH_TREE__
#define __SEARCH_TREE__

#include <map>
#include <nlp_string.h>
using namespace std;


class TreeAction;
class TreeUnit;
class TreeState;
typedef map <String, TreeAction*>	TreeAction_map_t;
typedef map <String, TreeUnit*>		TreeUnit_map_t;
typedef map <String, TreeState*>	TreeState_map_t;
typedef map <String, size_t>		ActionToIndex_map_t;


//									
class TreeAction
{
	friend class TreeUnit;
	friend class TreeState;
	friend class SearchTree;

	private:
		double	d_QValue;
		long	i_Count;

	public:
		TreeAction (void);
		TreeAction (double _dValue);
		void AddQValue (double _dValue);
};


//									
class TreeUnit
{
	friend class TreeState;
	friend class SearchTree;

	private:
		TreeAction_map_t	map_TreeActions;
		long				i_Count;

	public:
		TreeUnit (void);
		~TreeUnit (void);

		void AddAction (String& _rTreeAction);
		void AddQValue (String& _rTreeAction, double _dValue);
		void AddHeuristicQValue (String& _rTreeAction, double _dValue);

		size_t SelectAction (double _dExplorationBonus, ActionToIndex_map_t& _mapValidActions);
};


//									
class TreeState
{
	friend class SearchTree;

	private:
		TreeUnit_map_t	map_TreeUnits;

	public:
		TreeState (void);
		~TreeState (void);

		TreeUnit* GetTreeUnit (String& _sTreeUnitType);
};


//									
class SearchTree
{
	private:
		TreeState_map_t		map_TreeStates;

		// String SelectTreeAction_UCT (String& _rTreeState);

	public:
		SearchTree (void);
		~SearchTree (void);

		void Clear (void);

		TreeState* GetTreeState (String& _rTreeState);
		TreeUnit* GetTreeUnit (String& _rTreeState, String& _sTreeUnitType);
};



#endif
