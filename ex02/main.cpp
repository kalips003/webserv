
#include "_colors.h"
#include <string>
#include <iostream>

#include <deque>
#include <vector>

#include <ctime>
#include <algorithm>
#include <climits>

///////////////////////////////////////////////////////////////////////////////]
const unsigned int Jacobsthal[] = {0, 1, 3, 5, 11, 21, 43, 85, 171, \
	341, 683, 1365, 2731, 5461, 10923, 21845, 43691, 87381, \
	174763, 349525, 699051, 1398101, 2796203, 5592405, 11184811};
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
int	readNumber(char* numstr) {

	char*	end(NULL);
	long	num = std::strtol(numstr, &end, 10);
	if (*end)
		return -1; // not an int
	if (num > INT_MAX || num < 0)
		return -1; // too big / small
	return num;
}

template <typename Container>
bool	ini(int ac, char** av, Container& stacks) {
	if (ac < 2)
		return false; // need at least one number
	
	// instanciate time?

	for (int i = 1; i < ac; i++) {
		int result = readNumber(av[i]);
		if (result == -1) {
			std::cout << ERR "Bad input: " RESET << av[i] << std::endl;	
			return false;// bad input
		}
		stacks.push_back(result);
	}
	return true;
}

template <typename Container>
void	printContainer(Container& s) {

	for (size_t i = 0; i < s.size(); i++) {
		if (i)
			std::cout << ", ";
		std::cout << s[i];
	}
	std::cout << std::endl;
}

///////////////////////////////////////////////////////////////////////////////]
template <typename Container>
void	insertNum(Container& c, int to_insert) {

	typename Container::iterator it = c.begin();
	while (it != c.end() && *it < to_insert)
		it++;
	c.insert(it, to_insert);
}

template <typename Container>
void ft_merge(Container& left, Container& right) {

	for (size_t i = 0; Jacobsthal[i] < left.size(); i++) 
		insertNum(right, left[Jacobsthal[i]]);
	for (size_t j = 1, k = 1; j < left.size(); j++) {
		if (j == Jacobsthal[k] && k++)
			continue;
		// std::cout << RED "num: " RESET << left[j] << std::endl;
		insertNum(right, left[j]);
	}
}

template <typename Container>
Container	ft_algo(Container& c) {

	if (c.size() < 2)
		return c;

	Container right, left;
	for (size_t i = 0; i + 1 < c.size(); i += 2) {
		if (i + 1 < c.size() && c[i] > c[i+1])//
			std::swap(c[i], c[i+1]);
		// std::cout << C_441 "[" RESET << c[i] << ", " << c[i+1] << "] " RESET;
		
		left.push_back(c[i]);
		right.push_back(c[i+1]);
	}

	if (c.size() % 2 != 0)
		left.push_back(c.back());
	
	right = ft_algo(right);
	ft_merge<Container>(left, right);
	return right; 
}

template <typename Container>
void	helperMain(int ac, char** av, Container& c, const char* string) {

	clock_t clock = std::clock();

	if (ini(ac, av, c)) {
		std::cout << RED "first: " RESET << std::endl;
		printContainer(c);
		c = ft_algo<Container>(c);
		std::cout << RED "final: " RESET << std::endl;
		printContainer(c);
		double time1 = static_cast<double>(std::clock() - clock) / CLOCKS_PER_SEC;
		std::cout << C_524 "final time " << string << ": " RESET << time1 * 1e6 <<  "µs" << std::endl;
	}
}
///////////////////////////////////////////////////////////////////////////////]
// Jacobsthal sequence
// J0​ = 0,J1 ​= 1,
// Jn ​= Jn−1 ​+ 2*Jn−2

// = 0, 1, 3, 5, 11, 21, 43, 85, ...
///////////////////////////////////////////////////////////////////////////////]
//  [7, 3, 8, 1, 5, 2, 6, 4]
// (7,3), (8,1), (5,2), (6,4)
// (3,7), (1,8), (2,5), (4,6)
//  [3,1,2,4]      [7,8,5,6]
// 				 (7,8), (5,6)
///////////////////////////////////////////////////////////////////////////////]
int main(int ac, char** av)
{
	std::deque<int> dq;
	std::vector<int> vect;

	helperMain(ac, av, dq, "deque");
	helperMain(ac, av, vect, "vector");

/////////////////////////]
// print jacobsthal
	// int a = 0;
	// int b = 1;
	// while (a < 10000000) {
	// 	// std::cout << a << " & " << b << " => ";
	// 	int j = b + 2 * a;
	// 	a = b;
	// 	b = j;
	// 	std::cout << a << ", ";
	// 	// std::cout << b << std::endl;
	// }
/////////////////////////]

	return 0;
}