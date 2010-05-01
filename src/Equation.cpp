#include "Equation.h"

EquationConstant::EquationConstant( float _constant ) : constant( _constant ) { }

float EquationConstant::operator ()( float x ) const {
	return constant;
}

EquationInverseQuadratic::EquationInverseQuadratic( float _scale, float _dropoff ) : scale( _scale ), dropoff( _dropoff ) { }

// scale / 2 = scale / ( 1 + dropoff * x * x )
// 1 / 2 = 1 / ( 1 + dropoff * x * x )
// 2 = 1 + dropoff * x * x
// 1 = dropoff * x * x
// dropoff = 1 / ( x * x )
// x is the distance that the function's power drops by 1/2
float EquationInverseQuadratic::operator()( float x ) const {
	return scale / ( 1.0f + dropoff * x * x );
}

EquationGaussian::EquationGaussian( float _scale, float _stdev ) : scale( _scale ), stdev( _stdev ) { }

float EquationGaussian::operator()( float x ) const {
	return scale * exp( -x * x / ( 2.0f * stdev * stdev ) );
}
