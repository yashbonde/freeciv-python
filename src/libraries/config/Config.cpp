#include "nlp_config.h"
#include <sys/types.h>
#include <unistd.h>
using namespace std;


Config::Key_to_Config_map_t Config::map_KeytoConfig;
String Config::s_ConsolidatedConfig;
String Config::s_ConfigFile;


//-------------------------------------------------
void Config::SetCommandLine (int argc, const char* argv[])
{
	String sCommandLine;
	for (int i = 1; i < argc; ++ i)
		sCommandLine << argv [i] << " ";

	SetCommandLine (sCommandLine);
}


//-------------------------------------------------
void Config::SetCommandLine (const char* _zCommandLine)
{
	String sCommandLine (_zCommandLine);
	sCommandLine.ReplacePattern ("[ \\t]+=[ \\t]+", "=");

	list <String>	lstSwitches;
	sCommandLine.Split (lstSwitches);

	s_ConsolidatedConfig << "# options specified on command line	\n";
	list <String>::iterator	ite;
	for (ite = lstSwitches.begin ();
		ite != lstSwitches.end ();
		++ ite)
	{
		string::size_type lEquals = ite->Find ("=");
		if (string::npos == lEquals)
		{
			if ("" == s_ConfigFile)
				s_ConfigFile = *ite;
			continue;
		}

		s_ConsolidatedConfig << *ite << '\n';
		String sSwitchKey = ite->substr (0, lEquals);
		String sSwitchValue = ite->substr (lEquals + 1);

		if (0 == sSwitchKey.CaseCompare ("cfg"))
			s_ConfigFile = sSwitchValue;
		else
			map_KeytoConfig.insert (make_pair (sSwitchKey, sSwitchValue));
	}

	s_ConsolidatedConfig << "\n\n";
	if (s_ConfigFile != "")
		LoadConfig (PreprocessConfig (s_ConfigFile));
}


//-------------------------------------------------
bool Config::LoadConfig (const char* _zFileName)
{
    ifstream file (_zFileName);
    if (false == file.is_open ())
    {
        cerr << "Config::LoadConfig - Failed to open file '" << _zFileName << "'." << endl;
        return false;
    }

	// built-in configurations ...			
	String sPID;
	sPID = (long) getpid ();
	map_KeytoConfig.insert (make_pair ("_PID_", sPID));

	// configurations read from file ...	
    while (false == file.eof ())
    {
        char zLine [10000];
        file.getline (zLine, 10000);

		s_ConsolidatedConfig << zLine << '\n';

		// check if the line starts with a # - then it's a comment...
		char* p = zLine;
		while ((' ' == *p) || ('\t' == *p))
		{
			if ('\0' == *p)
				break;
			++ p;
		}
		if (('#' == *p) || ('\0' == *p))
			continue;

		// check for last line being blank - if so, ignore line & stop
        if ((true == file.eof ()) && (0 == strlen (zLine)))
            break;

		// extract configurations...
        char* pSplit = strchr (zLine, '=');
        if (NULL == pSplit)
            continue;

        *pSplit = '\0';

        char* pKey = zLine;
        char* pValue = pSplit + 1;

        -- pSplit;
        while ((' ' == *pSplit) || ('\t' == *pSplit))
            *(pSplit --) = '\0';
        while ((' ' == *pValue) || ('\t' == *pValue))
            ++ pValue;

		if (0 == strcmp (pKey, "_PID_"))
		{
			cerr << "[WARNING]  Configuration file '" << _zFileName 
				 << "' contains entry '_PID_' which is a reserved config entry, "
				 	"and is automatically set to the process id of the current process. "
					"The entry in the file will be ignored." 
				 << endl;
			continue;
		}
        map_KeytoConfig.insert (make_pair (pKey, pValue));
    }

	Config::PreprocessConfigs ();
    return true;
}


//-------------------------------------------------
bool Config::WriteConfig (const char* _zFileName)
{
    ofstream file (_zFileName);
    if (false == file.is_open ())
    {
        cerr << "Config::WriteConfig - Failed to open file '" << _zFileName << "'." << endl;
        return false;
    }

	file << s_ConsolidatedConfig << endl;
	file.close ();
	return true;
}


//-------------------------------------------------
void Config::PreprocessConfigs (void)
{
	Key_to_Config_map_t::iterator	ite;
	for (ite = map_KeytoConfig.begin ();
		ite != map_KeytoConfig.end ();
		++ ite)
	{
		Config::PreprocessConfig (ite);
	}
}



//-------------------------------------------------
String Config::PreprocessConfig (Key_to_Config_map_t::iterator& _iteConfig)
{
	_iteConfig->second = Config::PreprocessConfig (_iteConfig->second);
	return _iteConfig->second;
}



//-------------------------------------------------
String Config::PreprocessConfig (const char* _zConfig)
{
	String sConfig;

	const char* pLastBlockEnd = _zConfig;
	const char* pEmbeddedConfigStart = strstr (_zConfig, "${");
	while (NULL != pEmbeddedConfigStart)
	{
		char* pEmbeddedConfigEnd = (char*) strchr (pEmbeddedConfigStart, '}');
		if (NULL == pEmbeddedConfigEnd)
			break;

		int iLength = (pEmbeddedConfigEnd - pEmbeddedConfigStart - 2);
		char* pzEmbeddedConfigKey = new char [iLength + 1];
		strncpy (pzEmbeddedConfigKey, pEmbeddedConfigStart + 2, iLength);
		pzEmbeddedConfigKey [iLength] = '\0';

		if (NULL != strstr (pzEmbeddedConfigKey, "_USER_INPUT_:"))
		{
			String sEmbeddedKey (pzEmbeddedConfigKey);
			String_dq_t dqPrompt;
			sEmbeddedKey.Split (dqPrompt, ':');
			cout << "  " << dqPrompt [1] << " : ";
			cout.flush ();
			String sUserResponse;
			getline (cin, sUserResponse);

			sConfig << sUserResponse;
		}
		else
		{
			String sEmbeddedConfig;
			Key_to_Config_map_t::iterator   ite;
			ite = map_KeytoConfig.find (pzEmbeddedConfigKey);
			if (ite != map_KeytoConfig.end ())
				sEmbeddedConfig = Config::PreprocessConfig (ite->second);
			else
			{
				char* pEnv = getenv (pzEmbeddedConfigKey);
				if (NULL != pEnv)
					sEmbeddedConfig = pEnv;
				else
				{
					cerr << "Embedded configuration '" << pzEmbeddedConfigKey 
						 << "' present neither in config nor the environment." << endl;
					delete[] pzEmbeddedConfigKey;
					return String (_zConfig);
				}
			}

			iLength = (pEmbeddedConfigStart - pLastBlockEnd);
			if (iLength > 0)
			{
				char* pLastBlock = new char [iLength + 1];
				strncpy (pLastBlock, pLastBlockEnd, iLength);
				pLastBlock [iLength] = '\0';
				sConfig << pLastBlock;
				delete[] pLastBlock;
			}

			sEmbeddedConfig = Config::PreprocessConfig (sEmbeddedConfig);
			sConfig << sEmbeddedConfig;
		}

		pLastBlockEnd = pEmbeddedConfigEnd + 1;
		pEmbeddedConfigStart = strstr (pLastBlockEnd, "${");

		delete[] pzEmbeddedConfigKey;
	}

	sConfig << pLastBlockEnd;

	return sConfig;
}



//-------------------------------------------------
void Config::Clear (void)
{
    map_KeytoConfig.clear ();
}



//-------------------------------------------------
String Config::Get (const char* _zKey)
{
    Key_to_Config_map_t::iterator   ite;
    ite = map_KeytoConfig.find (_zKey);
    if (ite != map_KeytoConfig.end ())
		return ite->second;

	char* pEnv = getenv (_zKey);
	if (NULL != pEnv)
		return String (pEnv);

	cerr << "Config entry '" << _zKey << "' not found!" << endl;
	throw;
	return String ();
}


//-------------------------------------------------
String Config::GetTempFileName (const char* _zKey)
{
    String sTempPath;

    String sPathName = Config::Get (_zKey);
	if (sPathName == "")
	{
		char zTempFileName [L_tmpnam + 1];
		if (NULL == tmpnam (zTempFileName))
		{
			cerr << "[ERROR]  Unable to create temporary file. "
					"tmpnam () returned NULL in call Config::GetTempFileName ("
				 << _zKey << ")." << endl;
			return sTempPath;
		}
		sTempPath = zTempFileName;
		return sTempPath;
	}

    String sPath;
    String sPrefix;

    string::size_type iExtention = sPathName.ReverseFind ("/");
    if (string::npos == iExtention)
    {
        sPath = sPath;
        sPrefix = "tgt.";
    }
    else
    {
        sPath = sPathName.substr (0, iExtention);
        sPrefix = sPathName.substr (iExtention + 1);
    }

    char* pzTempPath = tempnam (sPath, sPrefix);
    sTempPath = pzTempPath;
    free (pzTempPath);

    return sTempPath;
}


//-------------------------------------------------
void Config::Set (const char* _zKey, const char* _zValue)
{
	map_KeytoConfig.insert (make_pair (_zKey, PreprocessConfig (_zValue)));
}


//-------------------------------------------------
String Config::GetConfigFileName (void)
{
	return s_ConfigFile;
}


