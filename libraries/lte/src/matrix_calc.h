// Copyright (c) 2001-2015, SCALABLE Network Technologies, Inc.  All Rights Reserved.
//                          600 Corporate Pointe
//                          Suite 1200
//                          Culver City, CA 90230
//                          info@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the QualNet
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

/*
 */

#ifndef _MATRIX_CALC_H_
#define _MATRIX_CALC_H_

#include <assert.h>

#include <complex>
#include <valarray>

///////////////////////////////////////////////////////////////
// typedef
///////////////////////////////////////////////////////////////
typedef std::complex < double > Dcomp;
typedef std::valarray < Dcomp > Cmat;

///////////////////////////////////////////////////////////////
// typedef enum
///////////////////////////////////////////////////////////////

/// Matrix Index(2x2 only)
typedef enum
{
    m11,
    m12,
    m21,
    m22,
    numberOfElements2x2
}MatrixIndex2x2;


///////////////////////////////////////////////////////////////
// default value
///////////////////////////////////////////////////////////////
const Dcomp LTE_DEFAULT_DCOMP(0.0, 0.0);

//--------------------------------------------------------------------------
//  Utility functions for calculation of 2x2 complex matrix
//--------------------------------------------------------------------------

/// Get transposed matrix
///
/// \param org  Matrix to transpose
///
/// \return Transposed matrix
Cmat GetTransposeMatrix(const Cmat& org);

/// Get Conjugate transposed matrix
///
/// \param org  Matrix to transpose
///
/// \return Conjugate transposed matrix
Cmat GetConjugateTransposeMatrix(const Cmat& org);

/// Multiply 2 matrices
///
/// \param m1  Left term matrix
/// \param m2  Right term matrix
///
/// \return Multiplied matrix
Cmat MulMatrix(const Cmat& m1, const Cmat& m2);

/// Sum 2 matrices
///
/// \param m1  Left term matrix
/// \param m2  Right term matrix
///
/// \return Multiplied matrix
Cmat SumMatrix(const Cmat& m1, const Cmat& m2);

/// Get inverted matrix
///
/// \param org  Matrix to invert
///
/// \return Inverted matrix
Cmat GetInvertMatrix(const Cmat& org);

/// Get diagonal matrix
///
/// \param e1  (0,0) element
/// \param e2  (1,1) element
///
/// \return Diagonal matrix
Cmat GetDiagMatrix(const Dcomp& e1, const Dcomp& e2);

/// Get diagonal matrix
///
/// \param e1  (0,0) element
/// \param e2  (1,1) element
///
/// \return Diagonal matrix
Cmat GetDiagMatrix(double e1, double e2);


#endif /* _MATRIX_CALC_H_ */
