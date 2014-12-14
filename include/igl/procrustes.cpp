// This file is part of libigl, a simple c++ geometry processing library.
// 
// Copyright (C) 2014 Stefan Brugger <stefanbrugger@gmail.com>
// 
// This Source Code Form is subject to the terms of the Mozilla Public License 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.
#include "procrustes.h"
#include "polar_svd.h"
#include "polar_dec.h"
 
template <typename DerivedV, typename Scalar, typename DerivedR, typename DerivedT>
IGL_INLINE void igl::procrustes(
    const Eigen::PlainObjectBase<DerivedV>& X,
    const Eigen::PlainObjectBase<DerivedV>& Y,
    bool includeScaling,
    bool includeReflections,
    Scalar& scale,
    Eigen::PlainObjectBase<DerivedR>& R,
    Eigen::PlainObjectBase<DerivedT>& t)
{
   using namespace Eigen;
   assert (X.rows() == Y.rows() && "Same number of points");
   assert(X.cols() == Y.cols() && "Points have same dimensions");

   // Center data
   const VectorXd Xmean = X.colwise().mean();      
   const VectorXd Ymean = Y.colwise().mean();      
   MatrixXd XC = X.rowwise() - Xmean.transpose();
   MatrixXd YC = Y.rowwise() - Ymean.transpose();

   // Scale
   scale = 1.;
   if (includeScaling)
   {
       double scaleX = XC.norm() / XC.rows();
       double scaleY = YC.norm() / YC.rows();
       scale = scaleY/scaleX;
       XC *= scale;
       assert (abs(XC.norm() / XC.rows() - scaleY) < 1e-8);
   }

   // Rotation 
   MatrixXd S = XC.transpose() * YC; 
   MatrixXd T;
   if (includeReflections)
     polar_dec(S,R,T);
   else
     polar_svd(S,R,T);
   R.transposeInPlace();

   // Translation
   t = Ymean - scale*R.transpose()*Xmean;
}

template <typename DerivedV, typename DerivedR, typename DerivedT>
IGL_INLINE void igl::procrustes(
    const Eigen::PlainObjectBase<DerivedV>& X,
    const Eigen::PlainObjectBase<DerivedV>& Y,
    bool includeScaling,
    bool includeReflections,
    Eigen::PlainObjectBase<DerivedR>& S,
    Eigen::PlainObjectBase<DerivedT>& t)
{
  double scale;
  procrustes(X,Y,includeScaling,includeReflections,scale,S,t);
  S *= scale;
}


template <typename DerivedV, typename Scalar, int DIM, int TType>
IGL_INLINE void igl::procrustes(
    const Eigen::PlainObjectBase<DerivedV>& X,
    const Eigen::PlainObjectBase<DerivedV>& Y,
    bool includeScaling,
    bool includeReflections,
    Eigen::Transform<Scalar,DIM,TType>& T)
{
  double scale;
  MatrixXd R;
  VectorXd t;
  procrustes(X,Y,includeScaling,includeReflections,scale,R,t);

   // Combine
   T = Translation<Scalar,DIM>(t) * R * Scaling(scale);
}

template <typename DerivedV, typename DerivedR, typename DerivedT>
IGL_INLINE void igl::procrustes(
    const Eigen::PlainObjectBase<DerivedV>& X,
    const Eigen::PlainObjectBase<DerivedV>& Y,
    Eigen::PlainObjectBase<DerivedR>& R,
    Eigen::PlainObjectBase<DerivedT>& t)
{
  double scale;
  procrustes(X,Y,false,false,R,t);
}

template <typename DerivedV, typename Scalar, typename DerivedT>
IGL_INLINE void igl::procrustes(
    const Eigen::PlainObjectBase<DerivedV>& X,
    const Eigen::PlainObjectBase<DerivedV>& Y,
    Eigen::Rotation2D<Scalar>& R,
    Eigen::PlainObjectBase<DerivedT>& t)
{
  assert (X.cols() == 2 && Y.cols() == 2 && "Points must have dimension 2");
  Matrix2d Rmat;
  procrustes(X,Y,false,false,Rmat,t);
  R.fromRotationMatrix(Rmat);
}