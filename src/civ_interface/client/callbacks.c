#include <stdio.h>
#include "config.h"
#include "client_main.h"
#include "message.h"
#include "hash.h"
#include "fc_types.h"
#include "climap.h"
#include "control.h"
#include "options.h"
#include "citydlg_g.h"
#include "cityrep_g.h"
#include "dialogs_g.h"

extern void SendSocketMessage (const char* _zMessage);


//													
void popup_revolution_dialog (struct government* _pGov)
{
	if (TRUE == gui_interactive_mode)
	{
		popup_revolution_dialog_original (_pGov);
		return;
	}

}


//													
void popup_city_dialog (struct city* _pCity)
{
	if (TRUE == gui_interactive_mode)
	{
		popup_city_dialog_original (_pCity);
		return;
	}

	// send city info to remote client...
	// char zReply [1000];
	// sprintf (zReply, "new city constructed : %s\n", _pCity->name);
	// SendSocketMessage (zReply);
}


//													
void popup_city_report_dialog (bool raise)
{
	if (TRUE == gui_interactive_mode)
	{
		popup_city_report_dialog_original (raise);
		return;
	}

}


//													
void popup_caravan_dialog (struct unit* _pUnit, struct city* _pHomeCity, struct city* _pDestCity)
{
	if (TRUE == gui_interactive_mode)
	{
		popup_caravan_dialog_original (_pUnit, _pHomeCity, _pDestCity);
		return;
	}

}


//													
void popup_diplomat_dialog (struct unit* _pUnit, struct tile* _pTile)
{
	if (TRUE == gui_interactive_mode)
	{
		popup_diplomat_dialog_original (_pUnit, _pTile);
		return;
	}

}


//													
void popup_incite_dialog (struct city* _pCity, int _iCost)
{
	if (TRUE == gui_interactive_mode)
	{
		popup_incite_dialog_original (_pCity, _iCost);
		return;
	}

}


//													
void popup_bribe_dialog (struct unit* _pUnit, int _iCost)
{
	if (TRUE == gui_interactive_mode)
	{
		popup_bribe_dialog_original (_pUnit, _iCost);
		return;
	}

}


//													
void popup_sabotage_dialog (struct city* _pCity)
{
	if (TRUE == gui_interactive_mode)
	{
		popup_sabotage_dialog_original (_pCity);
		return;
	}

}


//													
void popup_pillage_dialog (struct unit* _pUnit, bv_special _bvMayPillage, bv_bases _bvBases)
{
	if (TRUE == gui_interactive_mode)
	{
		popup_pillage_dialog_original (_pUnit, _bvMayPillage, _bvBases);
		return;
	}

}
