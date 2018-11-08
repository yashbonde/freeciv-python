#include <iostream>
#include <fstream>
#include <map>
#include <nlp_string.h>



class Config
{
        typedef std::map <String, String>    Key_to_Config_map_t;
        static Key_to_Config_map_t	map_KeytoConfig;
		static String				s_ConsolidatedConfig;
		static String s_ConfigFile;

		static void SetCommandLine (const char* _zCommandLine);
		static void PreprocessConfigs (void);
		static String PreprocessConfig (Key_to_Config_map_t::iterator& _iteConfig);
		static String PreprocessConfig (const char* _zConfig);

    public:
		static void SetCommandLine (int argc, const char* argv[]);
		static bool LoadConfig (const char* _zFileName);
		static bool WriteConfig (const char* _zFileName);
		static void Clear (void);
		
		static void Set (const char* _zKey, const char* _zValue);
		static String Get (const char* _zKey);
		static String GetTempFileName (const char* _zKey);

		static String GetConfigFileName (void);
};


class config : public String
{
	public:
		config (const char* _zKey)	{ (*(String*)this) = Config::Get (_zKey); };

		operator bool (void)			{ return (bool) *(String*)this; };
		operator int (void)				{ return (long) *(String*)this; };
		operator long (void)			{ return (long) *(String*)this; };
		operator float (void)			{ return (double) *(String*)this; };
		operator double (void)			{ return (double) *(String*)this; };
};

