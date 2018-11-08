#include "SearchTree.h"
#include <nlp_macros.h>
#include <math.h>
#include <assert.h>



//												
TreeAction::TreeAction (void)
{
	d_QValue = 0;
	i_Count = 0;
}


//												
TreeAction::TreeAction (double _dValue)
{
	d_QValue = _dValue;
	i_Count = 1;
}


//												
void TreeAction::AddQValue (double _dValue)
{
	d_QValue = (d_QValue * i_Count + _dValue) 
				/ (double) (++ i_Count);
}


//												
TreeUnit::TreeUnit (void)
{
	i_Count = 0;
}

TreeUnit::~TreeUnit (void)
{
	i_Count = -1;
	ITERATE (TreeAction_map_t, map_TreeActions, ite)
		delete ite->second;
	map_TreeActions.clear ();
}


//												
void TreeUnit::AddAction (String& _rTreeAction)
{
	TreeAction_map_t::iterator	ite;
	ite = map_TreeActions.find (_rTreeAction);
	if (map_TreeActions.end () == ite)
	{
		TreeAction* pTreeAction = new TreeAction;
		map_TreeActions.insert (make_pair (_rTreeAction, pTreeAction));
	}
}


//												
void TreeUnit::AddHeuristicQValue (String& _rTreeAction, double _dValue)
{
	if (0 == i_Count)
		++ i_Count;

	TreeAction_map_t::iterator	ite;
	ite = map_TreeActions.find (_rTreeAction);
	if (map_TreeActions.end () == ite)
	{
		TreeAction* pTreeAction = new TreeAction (_dValue);
		map_TreeActions.insert (make_pair (_rTreeAction, pTreeAction));
	}
}


//												
void TreeUnit::AddQValue (String& _rTreeAction, double _dValue)
{
	++ i_Count;

	TreeAction_map_t::iterator	ite;
	ite = map_TreeActions.find (_rTreeAction);
	if (map_TreeActions.end () == ite)
	{
		TreeAction* pTreeAction = new TreeAction (_dValue);
		map_TreeActions.insert (make_pair (_rTreeAction, pTreeAction));
	}
	else
		ite->second->AddQValue (_dValue);
}


//												
size_t TreeUnit::SelectAction (double _dExplorationBonus, 
							ActionToIndex_map_t& _mapValidActions)
{
	double d2logN = 2 * log (i_Count);

	long iSelectedAction = -1;
	double dMaxValue = -10000000;
	ITERATE (ActionToIndex_map_t, _mapValidActions, iteValid)
	{
		size_t iAction = iteValid->second;

		TreeAction_map_t::iterator	iteAction;
		iteAction = map_TreeActions.find (iteValid->first);
		assert (map_TreeActions.end () != iteAction);

		TreeAction* pTreeAction = iteAction->second;
		double dValue = 1000000;
		if (0 != pTreeAction->i_Count)
			dValue = pTreeAction->d_QValue
					 + _dExplorationBonus * sqrt (d2logN / (double)pTreeAction->i_Count);
		if ((dMaxValue < dValue) || (-1 == iSelectedAction))
		{
			dMaxValue = dValue;
			iSelectedAction = iAction;
		}
	}

	return iSelectedAction;
}


//												
TreeState::TreeState (void)
{
}


TreeState::~TreeState (void)
{
	ITERATE (TreeUnit_map_t, map_TreeUnits, ite)
		delete ite->second;
	map_TreeUnits.clear ();
}


//												
TreeUnit* TreeState::GetTreeUnit (String& _sTreeUnitType)
{
	TreeUnit_map_t::iterator	ite;
	ite = map_TreeUnits.find (_sTreeUnitType);
	if (map_TreeUnits.end () == ite)
	{
		TreeUnit* pTreeUnit = new TreeUnit;
		map_TreeUnits.insert (make_pair (_sTreeUnitType, pTreeUnit));
		return pTreeUnit;
	}
	else
		return ite->second;
}


//												
SearchTree::SearchTree (void)
{
}

SearchTree::~SearchTree (void)
{
	Clear ();
}

void SearchTree::Clear (void)
{
	ITERATE (TreeState_map_t, map_TreeStates, ite)
		delete ite->second;
	map_TreeStates.clear ();
}


//												
TreeState* SearchTree::GetTreeState (String& _rTreeState)
{
	TreeState_map_t::iterator	ite;
	ite = map_TreeStates.find (_rTreeState);
	if (map_TreeStates.end () == ite)
	{
		TreeState* pTreeState = new TreeState;
		map_TreeStates.insert (make_pair (_rTreeState, pTreeState));
		return pTreeState;
	}
	else
		return ite->second;
}


//												
TreeUnit* SearchTree::GetTreeUnit (String& _rTreeState, String& _sTreeUnitType)
{
	TreeState_map_t::iterator	ite;
	ite = map_TreeStates.find (_rTreeState);
	if (map_TreeStates.end () == ite)
	{
		TreeState* pTreeState = new TreeState;
		map_TreeStates.insert (make_pair (_rTreeState, pTreeState));
		return pTreeState->GetTreeUnit (_sTreeUnitType);
	}
	else
		return ite->second->GetTreeUnit (_sTreeUnitType);
}







