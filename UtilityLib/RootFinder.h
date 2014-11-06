// Polynomial Root-Finder
// Copyright (c) 2003, by Per Vognsen.
//
// This software is free for any use.

#pragma once

#include <iostream>
#include <iterator> 
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <limits>

#include <cassert>
#include <cstdlib>

#include <QVector>
	

namespace RootFinder
{
	const double pos_inf = std::numeric_limits<double>::max();
	const double neg_inf = -std::numeric_limits<double>::max();
	const double error_tolerance = 1e-10;

	template<typename T> int sign(T x)
	{
		if (x > 0)
			return 1;
		else if (x < 0)
			return -1;
		else
			return 0;
	}

	bool is_zero(double x)
	{
		return (std::fabs(x) < error_tolerance);
	}

	std::vector<double> differentiate(const std::vector<double>& coeff)
	{
		assert(!coeff.empty() && !is_zero(coeff[coeff.size()-1]));

		if (coeff.size() < 2)
			return std::vector<double>();

		std::vector<double> deriv;
		deriv.resize(coeff.size()-1);
		for (unsigned int i = 0; i < deriv.size(); i++)
			deriv[i] = (i+1) * coeff[i+1];
		return deriv;
	}

	double evaluate(const std::vector<double>& coeff, double x)
	{
		assert(!coeff.empty());
	
		double lc = coeff[coeff.size()-1];
		if (x == pos_inf)
			return (lc > 0 ? pos_inf : neg_inf);
		if (x == neg_inf)
		{
			if (coeff.size() % 2 == 0)
				return (lc > 0 ? neg_inf : pos_inf);
			else
				return (lc > 0 ? pos_inf : neg_inf);
		}
	
		double value = 0, y = 1;
		for (unsigned int i = 0; i < coeff.size(); i++)
		{
			value += coeff[i] * y;
			y *= x;
		}
		return value;
	}

	double finite_bisect(const std::vector<double>& coeff, double low,
		                 double high)
	{
		assert(low != neg_inf && low != pos_inf &&
			   high != neg_inf && high != pos_inf);

		if (high < low)
			std::swap(low, high);

		const unsigned int max_iterations = 100000;

		for (unsigned int i = 0; i < max_iterations; i++)
		{
			if (is_zero(evaluate(coeff, low)))
				return low;
		
			double mid = 0.5 * low + 0.5 * high;
			if (sign(evaluate(coeff, low)) != sign(evaluate(coeff, mid)))
				high = mid;
			else
				low = mid;
		}
	
		std::cout << "Too many iterations in finite_bisect()." << std::endl;
		return 0.0;
	}
	
	double bisect(const std::vector<double>& coeff, double low, double high)
	{
		assert(!coeff.empty());

		if (high < low)
			std::swap(low, high);
		
		double e = 1.0;
		if (low == neg_inf)
		{
			low = high;
			while (sign(evaluate(coeff, low)) == sign(evaluate(coeff, high)))
			{
				low -= e;
				e *= 2.0;
			}
		}
		else if (high == pos_inf)
		{
			high = low;
			while (sign(evaluate(coeff, high)) == sign(evaluate(coeff, low)))
			{
				high += e;
				e *= 2.0;
			}
		}

		return finite_bisect(coeff, low, high);
	}

	// Generate the quadratic polynomial with roots a and b.
	std::vector<double> quadratic_polynomial(double a, double b)
	{
		std::vector<double> coeff;
		coeff.push_back(a*b);
		coeff.push_back(-(a+b));
		coeff.push_back(1.0);
		return coeff;
	}

	// Generate the cubic polynomial with roots a, b and c.
	std::vector<double> cubic_polynomial(double a, double b, double c)
	{
		std::vector<double> coeff;
		coeff.push_back(-(a*b*c));
		coeff.push_back(a*b+a*c+b*c);
		coeff.push_back(-(a+b+c));
		coeff.push_back(1.0);
		return coeff;
	}

	// Generate the quartic polynomial with roots a, b, c and d.
	std::vector<double> quartic_polynomial(double a, double b, double c,
		                                   double d)
	{
		std::vector<double> coeff;
		coeff.push_back(a*b*c*d);
		coeff.push_back(-(a*b*d+a*c*d+b*c*d+a*b*c));
		coeff.push_back(a*b+a*c+b*c+a*d+b*d+c*d);
		coeff.push_back(-(a+b+c+d));
		coeff.push_back(1.0);
		return coeff;
	}

	double random_double(double low, double high)
	{
		assert(high > low);
		
		return (low + (high - low + 1) * std::rand()/double(RAND_MAX + 1.0));
	}

	int random_integer(int low, int high)
	{
		return int(random_double(low, high));
	}

	std::vector<double> random_polynomial(unsigned int degree, double low, double high)
	{
		std::vector<double> coeff;
		for (unsigned int i = 0; i < degree+1; i++)
			coeff.push_back(random_double(low, high));
		return coeff;
	}


std::vector<double> find_roots(const std::vector<double>& coeff)
{
	assert(coeff.size() >= 2 && !is_zero(coeff[coeff.size()-1]));

	if (coeff.size() == 2)
		return std::vector<double>(1, -coeff[0]/coeff[1]);
	
	std::vector<double> deriv_roots(find_roots(differentiate(coeff)));

	if (deriv_roots.empty())
		deriv_roots.push_back(0.0);

	deriv_roots.push_back(neg_inf);
	deriv_roots.push_back(pos_inf);
	
	std::sort(deriv_roots.begin(), deriv_roots.end());
	deriv_roots.erase(std::unique(deriv_roots.begin(), deriv_roots.end()),
					  deriv_roots.end());
	
	std::vector<double> roots;
	for (unsigned int i = 0; i < deriv_roots.size()-1; i++)
	{
		double first_value = evaluate(coeff, deriv_roots[i]),
			   second_value = evaluate(coeff, deriv_roots[i+1]);
		
		if (is_zero(first_value) || sign(first_value) != sign(second_value))
			roots.push_back(bisect(coeff, deriv_roots[i], deriv_roots[i+1]));
	}

	std::sort(roots.begin(), roots.end());
	roots.erase(std::unique(roots.begin(), roots.end()), roots.end());

	return roots;
}

void test_polynomial(std::vector<double> coeff)
{
	std::cout << "Polynomial: ";
	std::copy(coeff.begin(), coeff.end(),
		      std::ostream_iterator<double>(std::cout, " "));
	std::cout << std::endl;

	std::vector<double> roots = find_roots(coeff);
	if (roots.size() > 0)
	{
		for (unsigned int i = 0; i < roots.size(); i++)
			if (!is_zero(evaluate(coeff, roots[i])))
				std::cout << "Invalid root: " << roots[i] << std::endl;
		std::cout << "Roots: ";
		for (unsigned int i = 0; i < roots.size(); i++)
			std::cout << roots[i] << " ";
		std::cout << std::endl;
	}
	else
		std::cout << "No roots." << std::endl;

	std::cout << std::endl;
}

}