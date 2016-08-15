#ifndef EIGEN_MATRIX
#define EIGEN_MATRIX

//#include <boost/serialization/array.hpp>
//#define EIGEN_DENSEBASE_PLUGIN "local-eigen-serialization.h"

#ifdef _MSC_VER
// Silence warning: 'Eigen::Quaternion<T,0>' : multiple assignment operators specified
#pragma warning (push)
#pragma warning (disable : 4522)
#endif

#include <Eigen/Core>
#include <unsupported/Eigen/KroneckerProduct>
#include <Eigen/Cholesky>
#include <Eigen/Eigenvalues>

#ifdef _MSC_VER
#pragma warning (pop)
#endif

static const int MAX_MIMO_SIZE = 8;

#endif
