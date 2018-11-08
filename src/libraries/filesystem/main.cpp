#include <nlp_filesystem.h>
#include <nlp_string.h>
#include <nlp_macros.h>
#include <iostream>
using namespace std;

int main (int argc, char* argv [])
{
	{
		File file (argv [1]);
		String sLine = " ";
		file.ReadLastLine (sLine);
		cout << sLine << endl;
		return 0;

		int iBytes = 100;

		file.seekg (0, ios::end);
		long lFileEnd = file.tellg ();
		while (true)
		{
			file.seekg (0, ios::end);
			cout << iBytes << endl;
			file.seekg (- iBytes, ios::cur);

			std::getline (file, sLine);
			cout << file.tellg () << " <> " << lFileEnd << endl;
			if (file.tellg () < lFileEnd)
			{
				cout << sLine << endl;
				while (file.tellg () < lFileEnd)
				{
					cout << file.tellg () << " < " << lFileEnd << endl;
					std::getline (file, sLine);
				}
				cout << sLine << endl;
				break;
			}
			
			iBytes += 100;
		}
		return 0;
	}

	// cout << "->" << argv [1] << "<-" << endl;
	// cout << "->" << Path::GetCanonicalPath (argv [1]) << "<-" << endl;

	// File file (argv [1]);
	// String sLine;
	// while (file.ReadLine (sLine))
		// cout << sLine << endl;

	unsigned long ulValues = 0;
	{
		CsvFile csv (argv [1]);
		CsvFile out (argv [2], ios_base::out);
		out.SetSparseFileFormat (true);

		// csv.SetLineValueCount (10);

		// cout << csv.GetFirstLineValueCount () << endl;
		int_dq_t	dqValues;
		while (csv.ReadLine (dqValues, &ulValues))
		{
			// cout << dqValues.size () << endl;
			out.WriteLine (dqValues);
			ITERATE (int_dq_t, dqValues, ite)
				cout << *ite << ", ";
			cout << endl << endl;
		}
		out.flush ();
	}
	// return 0;

	{
		CsvFile csv (argv [2]);
		CsvFile out (argv [3], ios_base::out|ios_base::app);

		cout << csv.IsSparseFileFormat () << endl;
		ulValues = csv.GetFirstLineValueCount ();
		cout << ulValues << endl;
		int pValues [ulValues];
		while (csv.ReadLine (pValues, &ulValues))
		{
			for (unsigned long i = 0; i < ulValues; ++ i)
				cout << pValues [i] << ", ";
			cout << endl << endl;
			out.WriteLine (pValues, ulValues);
		}
		cout << csv.GetFirstLineValueCount () << endl;

	}
}
