#ifndef __PORTER_STEMMER__
#define __PORTER_STEMMER__

#include <stdlib.h>  /* for malloc, free */
#include <string.h>  /* for memcmp, memmove */




/* stemmer is a structure for a few local bits of data,
*/
/* Member b is a buffer holding a word to be stemmed. The letters are in
   b[0], b[1] ... ending at b[z->k]. Member k is readjusted downwards as
   the stemming progresses. Zero termination is not in fact used in the
   algorithm.

   Note that only lower case sequences are stemmed. Forcing to lower case
   should be done before stem(...) is called.


   Typical usage is:

       struct stemmer * z = create_stemmer();
       char b[] = "pencils";
       int res = stem(z, b, 6);
           /- stem the 7 characters of b[0] to b[6]. The result, res,
              will be 5 (the 's' is removed). -/
       free_stemmer(z);
*/

struct stemmer 
{
   char * b;       /* buffer for word to be stemmed */
   int k;          /* offset to the end of the string */
   int j;          /* a general offset into the string */
};



//
class PorterStemmer
{
private:
	static int stem (struct stemmer* z, char * b, int k);
	static int cons (struct stemmer* z, int i);
	static int m (struct stemmer* z);
	static int vowelinstem (struct stemmer* z);
	static int doublec (struct stemmer* z, int j);
	static int cvc (struct stemmer* z, int i);
	static int ends (struct stemmer* z, const char* s);
	static void setto (struct stemmer* z, const char* s);
	static void r (struct stemmer* z, const char* s);
	static void step1ab (struct stemmer* z);
	static void step1c (struct stemmer* z);
	static void step2 (struct stemmer* z);
	static void step3 (struct stemmer* z); 
	static void step4 (struct stemmer* z);
	static void step5 (struct stemmer* z);

public:
	static struct stemmer* CreateStemmer (void);
	static void FreeStemmer (struct stemmer* z);

	static int Stem (char* _zWord, int _iLength = 0);
	static int Stem (struct stemmer* _pStemmer, char* _zWord, int _iLength = 0);
};


#endif
