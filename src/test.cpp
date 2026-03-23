// This is a test cpp file for educational uses , Its ""NOT"" a part of the project and will be deleted

#include <iostream>
#include <cmath>
#include <complex>
#include <vector>
using namespace std;
template <typename T>
void print(const T& x){
	cout << x << endl;
} // a print function to print a single variable
template <typename T>
void printv(const vector<T>& v){
	for ( T x : v){
		cout << x << " ";
	}
	cout << endl; 
} // a vector print function to print a single vector
int main() {
   	print("Hello world");
   	print(2);
  	print(sizeof(int));
	print(314e-2);
	print(0b0100110); // 0b prefix indicates a binary (base 2)
	print(0xBAD12CE3); // 0x prefix indicates a hexadecimal (base16)
	print(0334); // A 0 prefix indicates an octal (base 8)
	for (double i = 0; i <= 15; ++i){
		string triangle (i, '*');
		print(triangle);
	} // Printing a triangle with our printing function! 
	complex<double> z;
	print(z);
	vector<int> v {1, 2, 3, 4};
	printv(v);  // Printing a vector with our printv function! 
	return 0;               
} 
