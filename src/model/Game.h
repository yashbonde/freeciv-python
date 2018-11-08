#ifndef __GAME__
#define __GAME__

#include <nlp_string.h>
#include "Interface.h"
#include "Play.h"
#include "Montecarlo.h"

enum PlayMode_e
{
	pm_random,
	pm_text,
	pm_montecarlo
};


class Game : public Play
{
	private:
		GameInterface	o_GameInterface;
		int				i_MaxSteps;
		String			s_InstanceName;
		PlayMode_e		e_PlayMode;

		String SelectRandomCommand (void);
		String SelectCommandUsingText (MonteCarlo& _rMontecarlo);

	public:
		Game (void);
		~Game (void);

		bool Init (void);
		void Close (void);
		pair<double,double> Play (MonteCarlo& _rMontecarlo);
};


#endif
