#pragma once

#include "..\Numeric.h"

inline double uniformRealDistribution(double a = 0.0, double b = 1.0)
{
	double r = qrand() / (double) RAND_MAX;
	r =  (1 - r) * a + r * b;
	return RANGED(a, r, b);
}

inline int uniformDiscreteDistribution(int a, int b)
{
	double r = uniformRealDistribution();
	int n = a + int(r * (b - a));
	return RANGED(a, n, b-1);
}

inline int discreteDistribution(const QVector<double>& posibility)
{
	QVector<double> accPosibility;
	accPosibility.push_back(0);
	foreach (double p, posibility)	
		accPosibility.push_back(accPosibility.last() + p);

	double r = uniformRealDistribution();
	for (int i = 0; i < accPosibility.size()-1; i++)
	{
		if (r >= accPosibility[i] && r <= accPosibility[i+1])
			return i;
	}

	return 0;
}

// Marsaglia polar method
// Wikipedia: http://en.wikipedia.org/wiki/Marsaglia_polar_method
class NormalDistribution
{
public:
	NormalDistribution(){isSpareReady = false;}
	~NormalDistribution(){}
	double generate(double mean, double stdDev ){
		if (isSpareReady) {
			isSpareReady = false;
			return spare * stdDev + mean;
		} else {
			double u, v, s;
			do {
				u = uniformRealDistribution() * 2 - 1;
				v = uniformRealDistribution() * 2 - 1;
				s = u * u + v * v;
			} while (s >= 1 || s == 0);
			double mul = sqrt(-2.0 * log(s) / s);
			spare = v * mul;
			isSpareReady = true;
			return mean + stdDev * u * mul;
		}
	}
private:
	double	spare;
	bool	isSpareReady;
};