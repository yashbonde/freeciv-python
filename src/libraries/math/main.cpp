#include <iostream>
#include <nlp_vector.h>
using namespace std;

typedef Vector<double>	double_Vec_t;


int main (void)
{
	double_Vec_t	vec_Nums;
	vec_Nums.Resize (10, 0);
	cout << '1' << endl;
	vec_Nums.Resize (20, 1);
	cout << '2' << endl;
}
