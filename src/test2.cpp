// This is another test file and will be removed later!
#include<iostream>
#include<vector>
#include<cmath>
#include<complex>
#include<variant>
#include<string>
#include<torch/torch.h>

namespace ez{
	template<typename T>
	void print(const T &x){
		std::cout << x << std::endl;
	}
	template<typename T>
	void input( T &x){
		std::cin >> x;
	}

} 

int main() {
    	// LibTorch test
    	torch::Tensor t = torch::rand({3, 3}).cuda();
    	std::cout << "LibTorch tensor on GPU:\n" << t << std::endl;
    	torch::Tensor v = torch::rand({2, 1}).cuda();
    	std::cout << "LibTorch vector on GPU:\n" << v << std::endl;
	torch::Tensor ones = torch::ones({5, 5}).cuda();
	std::cout << "The ones tensor is : \n" << ones << std::endl;
	torch::Tensor zeros = at::zeros({5, 5}).cuda();
	std::cout << "The zeros tensor is : \n" << zeros << std::endl;
	int k;
	ez::print("Enter a number: ");
	ez::input(k);
	ez::print(k);
	std::cerr << "ERROR YOU ARE SO FAT" << std::endl;
	ez::print(pow(2 , pow(2 , pow(2, 2))));
    	return 0;
}
