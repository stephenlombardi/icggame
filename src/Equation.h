#ifndef _Equation_h
#define _Equation_h

#include <cmath>

class Equation {
public:
	virtual float operator()( float x ) const = 0;
};

class EquationConstant : public Equation {
public:
	EquationConstant( float _constant );
	float operator()( float x ) const;
private:
	float constant;
};

class EquationInverseQuadratic : public Equation {
public:
	EquationInverseQuadratic( float _scale, float _dropoff );
	float operator()( float x ) const;
private:
	float scale, dropoff;
};

class EquationGaussian : public Equation {
public:
	EquationGaussian( float _scale, float _stdev );
	float operator()( float x ) const;
private:
	float scale, stdev;
};

#endif