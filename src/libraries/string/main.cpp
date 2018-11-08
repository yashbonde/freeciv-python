#include <iostream>
#include <fstream>
#include <nlp_macros.h>
#include <nlp_string.h>
using namespace std;


int main (void)
{
	{
		{
			String s ("hello how are you .");
			String_dq_t dq;
			s.Split (dq);
			cout << dq << endl;
		}
		{
			String s ("hello how are you  ");
			String_dq_t dq;
			s.Split (dq);
			cout << dq << endl;
		}
		return 0;
	}

	{
		stemmer* pStemmer = PorterStemmer::CreateStemmer ();
		{
			char zWord [10];
			strcpy (zWord, "the");
			int x = PorterStemmer::Stem (pStemmer, zWord, strlen (zWord));
			cout << zWord << " : " << strlen (zWord) << " = " << x << endl;
		}
		{
			char zWord [10];
			strcpy (zWord, "house");
			int x = PorterStemmer::Stem (pStemmer, zWord, strlen (zWord));
			cout << zWord << " : " << strlen (zWord) << " = " << x << endl;
		}
		{
			char zWord [10];
			strcpy (zWord, "of");
			int x = PorterStemmer::Stem (pStemmer, zWord, strlen (zWord));
			cout << zWord << " : " << strlen (zWord) << " = " << x << endl;
		}
		{
			char zWord [10];
			strcpy (zWord, "a");
			int x = PorterStemmer::Stem (pStemmer, zWord, strlen (zWord));
			cout << zWord << " : " << strlen (zWord) << " = " << x << endl;
		}
		return 0;
	}

	{
		String a ("the house of representatives on friday approved a major package of tax breaks and safety-net spending for the unemployed but only after the democratic leadership pared the plan back substantially to win over rank-and-file");
		String_dq_t b;
		a.Split (b);

		String::InitPorterStemmer ();
		ITERATE (String_dq_t, b, ite)
		{
			cout << (*ite) << "   " << (*ite).GetPorterStem () << endl;
		}
		String::DestroyPorterStemmer ();

		return 0;
	}

	{
		String a ("[delay response] -1");
		cout << a.StartsWith ("[delay response]") << endl;
		if (true == a.StartsWith ("[delay response]"))
			cout << "ok!" << endl;
		return 0;
	}

	{
		String a ("settings");
		cout << a.StartsWith ("set") << endl;
		cout << a.EndsWith ("ings") << endl;
		cout << a.Has ("tti") << endl;
		cout << a.StartsWith ("setx") << endl;
		cout << a.EndsWith ("service") << endl;
		return 0;
	}

	{
		char z [100];
		strcpy (z, "abc_def_ghi_jkl");

		zchar_dq_t dq1 = String::DestructiveSplit (z, '_', 1);
		ITERATE (zchar_dq_t, dq1, ite)
			cout << *ite << "  ";
		cout << endl;

		zchar_dq_t dq2 = String::DestructiveSplit (dq1[1], '_', 1);
		ITERATE (zchar_dq_t, dq2, ite)
			cout << *ite << "  ";
		cout << endl;

		return 0;
	}


	{
		String x ("abcdefgh");
		cout << x << endl;

		String y;
		y = "asdfajdsl";
		y.Set ((const char*)x, 3);
		cout << 3 << endl;
		cout << y << endl;

		y.Set ((const char*)x + 1, 7);
		cout << 7 << endl;
		cout << y << endl;

		y.Set ((const char*)x + 1, 8);
		cout << 8 << endl;
		cout << y << endl;

		y.Set ((const char*)x, 0);
		cout << 0 << endl;
		cout << y << endl;

		y = "asdfajdsl";
		y.Set ((const char*)NULL, 0);
		cout << 0 << endl;
		cout << y << endl;

		cout << -1 << endl;
		y.Set ((const char*)NULL, -1);
	}


	{
		// String x ("a" "\x03" "b" "\x03" "c" "\x03" "" "\x03" "e" "\x03");
		String x ("a b c  e ");
		String_dq_t dq;
		x.Split (dq, ' ');
		cout << dq << endl;
		return 0;
	}


	{
		String s ("Applications");
		if (true == s.Has ("p" + string ("lic")))
			cout << "has ok" << endl;
		if (true == s.StartsWith ("A" + string ("ppli")))
			cout << "starts ok" << endl;
		if (true == s.EndsWith ("ions"))
			cout << "ends ok" << endl;
		else
			cout << "ends not ok" << endl;
		if (true != s.EndsWith ("ion"))
			cout << "ends ok" << endl;
		else
			cout << "ends not ok" << endl;

		return 0;
	}


	/*
	String s ("Click the <b>Foreign Users</b> tab, type the DN of the top object of the tree in the <b>Foreign user's organizational unit</b> box   (for example, type <b>dc=com</b> if your naming context is similar to <b>dc=microsoft,dc=com</b>).");
	cout << s << endl;
	s.ReplacePattern ("<b>([^,<]*),([^<]*)</b>", "<b>\\1\x02\\2</b>");
	cout << s << endl;
	cout << endl;

	String t ("From the File menu, choose New, and press ENTER.");
	cout << t << endl;
	t.ReplacePattern ("([Ff]rom the [^,]*),", "\\1#");
	cout << t << endl;
	cout << endl;

	String r ("choose Start (ALT, R, S) to");
	cout << r << endl;
	r.ReplacePattern ("\\(([^,<]*),([^<]*)\\)", "(\\1\x02\\2)");
	cout << r << endl;
	return 0;


	String text ("the_DT large_JJ ble_JJS crowd_NN is_X very_JJS noisy_NNP blah_NNPS");
	String_dq_t match;
	text.FindAllPattern ("([^ ]+_DT )?([^ ]+_JJ[SP]? )*([^ ]+_NN[PS]?[S]?[ ]?)+", match);
	cout << match << endl;
	return 0;


	String a (" 	ABS DF	  ");
	String b = a;
	b.Strip ();

	cout << "->" << a << "<-" << endl;
	cout << "->" << b << "<-" << endl;
	return 0;

	ifstream file ("txt");
	while (false == file.eof ())
	{
		char zLine [10000];
		file.getline (zLine, 10000);

		String sText (zLine);

		cout << sText << endl;


		//------------------------------------------------------------------
		if (true)
		{
			sText.Replace (">th ", "> ");
			sText.Replace (" ax ", " ");
			sText.Replace (" py ", " ");
			sText.ReplacePattern ("\\&.*\\&", " <exp0> ");
			sText.ReplacePattern ("\\{.*\\}", " <exp1> ");
			sText.Replace ("<expression>", "<exp>");
			sText.Replace ("<figure>", "<exp>");
			sText.ReplacePattern ("([A-Za-z0-9]+[\\\\\\/\\+\\=\\%\\^\\*_]+[A-Za-z0-9]*)|(([\\\\\\/\\+\\=\\%\\^\\*_]+[A-Za-z0-9]+)[\\\\\\/\\+\\=\\%\\^\\*_]*)", "");
			sText.Replace ("\\", "");
			sText.ReplacePattern ("(\\([^a-zA-Z0-9]*\\))|(\\[[^a-zA-Z0-9]*\\])|(\\<[^a-zA-Z0-9]*\\>)", "");
			sText.ReplacePattern ("([,.?\\*!@\\$%\\^&])[\t ]*([,.?\\*!@\\$%\\^&])", "\\2");
			sText.ReplacePattern (">[>]+", ">");

			sText.ReplacePattern ("[ \t]+$", "");
			char ch = sText [sText.length () - 1];
			if (NULL == strchr (".,!?;:", ch))
				sText << " .";
		}
		//------------------------------------------------------------------
	
		//------------------------------------------------------------------
		sText.Replace ("...", " ... ");

		sText.ReplacePattern ("([,;:@#$%&])", " \\1 ");
		sText.ReplacePattern ("([^.])([.])([]\\)}>\"\']*)[ \t]*$", "\\1 \\2\\3 ");
		sText.ReplacePattern ("([?!])", " \\1 ");
		sText.ReplacePattern ("([\\]\\[\\(\\)\\{\\}])", " \\1 ");

		sText.Replace ("<", " <");
		sText.Replace (">", "> ");
		sText.Replace ("--", " -- ");
		sText.Replace ("-", " ");
		sText.Replace ("(", "-LRB-");
		sText.Replace (")", "-RRB-");
		sText.Replace ("[", "-LSB-");
		sText.Replace ("]", "-RSB-");
		sText.Replace ("{", "-LCB-");
		sText.Replace ("}", "-RCB-");

		String s (" ");
		s << sText << ' ';
		sText = s;
		sText.Replace ("\"", " '' ");
		sText.ReplacePattern ("([^'])' ", "\\1 ' ");
		sText.ReplacePattern ("'([sSmMdD])", " '\\1 ");

		sText.Replace ("'ll ", " 'll ");
		sText.Replace ("'LL ", " 'LL ");
		sText.Replace ("'re ", " 're ");
		sText.Replace ("'RE ", " 'RE ");
		sText.Replace ("'ve ", " 've ");
		sText.Replace ("'VE ", " 'VE ");
		sText.Replace ("n't ", " n't ");
		sText.Replace ("N'T ", " N'T ");

		sText.ReplacePattern (" ([Cc])annot ", " \\1an not ");
		sText.ReplacePattern (" ([Dd])'ye ", " \\1id you ");
		sText.ReplacePattern (" ([Gg])imme ", " \\1ive me ");

		sText.ReplacePattern ("^[ \t]+", "");
		sText.ReplacePattern ("[ \t]+$", "");
		sText.ReplacePattern ("[ \t]+", " ");

		//------------------------------------------------------------------

		cout << sText << endl;
	}
*/
}
