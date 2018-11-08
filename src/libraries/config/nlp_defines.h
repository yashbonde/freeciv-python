#include <iostream>
#include <fstream>
#include <map>
#include <nlp_string.h>

#define ITERATE_COLLECTION(type,lst,ite)	\
					for (type::iterator ite = lst.begin (), type::iterator ite##_end = lst.end (); \
						 ite != ite##_end; \
						 ++ ite)

// #define ITERATE_COLLECTION(type,lst,ite)	type::iterator ite; type::iterator ite##_end; for (ite = lst.begin (), ite##_end = lst.end (); ite != ite##_end; ++ ite)

#define ITERATE_COLLECTION_POINTER(type,plst,ite)	type::iterator ite; type::iterator ite##_end; for (ite = plst->begin (), ite##_end = plst->end (); ite != ite##_end; ++ ite)



class Config
{

        typedef std::map <String, String>    Key_to_Config_map_t;
        static Key_to_Config_map_t map_KeytoConfig;

		static void SetCommandLine (const char* _zCommandLine);
		static void PreprocessConfigs (void);
		static String PreprocessConfig (Key_to_Config_map_t::iterator& _iteConfig);
		static String PreprocessConfig (const char* _zConfig);

    public:
		static void Config::SetCommandLine (int argc, const char* argv[]);
		static bool LoadConfig (const char* _zFileName);
		static void Clear (void);
		
		static void Set (const char* _zKey, const char* _zValue);
		static String Get (const char* _zKey);
		static String GetTempFileName (const char* _zKey);
};


class config_t : public String
{
	config_t (const char* _zKey);

	operator bool (void);
	operator int (void);
	operator long (void);
	operator float (void);
	operator double (void);
	operator String (void);
};
