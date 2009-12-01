// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2009 Gael Guennebaud <g.gael@free.fr>
// Copyright (C) 2006-2008 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#ifndef EIGEN_PARTIAL_REDUX_H
#define EIGEN_PARTIAL_REDUX_H

/** \array_module \ingroup Array_Module
  *
  * \class PartialReduxExpr
  *
  * \brief Generic expression of a partially reduxed matrix
  *
  * \param MatrixType the type of the matrix we are applying the redux operation
  * \param MemberOp type of the member functor
  * \param Direction indicates the direction of the redux (Vertical or Horizontal)
  *
  * This class represents an expression of a partial redux operator of a matrix.
  * It is the return type of some VectorwiseOp functions,
  * and most of the time this is the only way it is used.
  *
  * \sa class VectorwiseOp
  */

template< typename MatrixType, typename MemberOp, int Direction>
class PartialReduxExpr;

template<typename MatrixType, typename MemberOp, int Direction>
struct ei_traits<PartialReduxExpr<MatrixType, MemberOp, Direction> >
{
  typedef typename MemberOp::result_type Scalar;
  typedef typename MatrixType::Scalar InputScalar;
  typedef typename ei_nested<MatrixType>::type MatrixTypeNested;
  typedef typename ei_cleantype<MatrixTypeNested>::type _MatrixTypeNested;
  enum {
    RowsAtCompileTime = Direction==Vertical   ? 1 : MatrixType::RowsAtCompileTime,
    ColsAtCompileTime = Direction==Horizontal ? 1 : MatrixType::ColsAtCompileTime,
    MaxRowsAtCompileTime = Direction==Vertical   ? 1 : MatrixType::MaxRowsAtCompileTime,
    MaxColsAtCompileTime = Direction==Horizontal ? 1 : MatrixType::MaxColsAtCompileTime,
    Flags = (unsigned int)_MatrixTypeNested::Flags & HereditaryBits,
    TraversalSize = Direction==Vertical ? RowsAtCompileTime : ColsAtCompileTime
  };
  #if EIGEN_GNUC_AT_LEAST(3,4)
  typedef typename MemberOp::template Cost<InputScalar,int(TraversalSize)> CostOpType;
  #else
  typedef typename MemberOp::template Cost<InputScalar,TraversalSize> CostOpType;
  #endif
  enum {
    CoeffReadCost = TraversalSize * ei_traits<_MatrixTypeNested>::CoeffReadCost + int(CostOpType::value)
  };
};

template< typename MatrixType, typename MemberOp, int Direction>
class PartialReduxExpr : ei_no_assignment_operator,
  public MatrixBase<PartialReduxExpr<MatrixType, MemberOp, Direction> >
{
  public:

    EIGEN_GENERIC_PUBLIC_INTERFACE(PartialReduxExpr)
    typedef typename ei_traits<PartialReduxExpr>::MatrixTypeNested MatrixTypeNested;
    typedef typename ei_traits<PartialReduxExpr>::_MatrixTypeNested _MatrixTypeNested;

    PartialReduxExpr(const MatrixType& mat, const MemberOp& func = MemberOp())
      : m_matrix(mat), m_functor(func) {}

    int rows() const { return (Direction==Vertical   ? 1 : m_matrix.rows()); }
    int cols() const { return (Direction==Horizontal ? 1 : m_matrix.cols()); }

    const Scalar coeff(int i, int j) const
    {
      if (Direction==Vertical)
        return m_functor(m_matrix.col(j));
      else
        return m_functor(m_matrix.row(i));
    }

    const Scalar coeff(int index) const
    {
      if (Direction==Vertical)
        return m_functor(m_matrix.col(index));
      else
        return m_functor(m_matrix.row(index));
    }

  protected:
    const MatrixTypeNested m_matrix;
    const MemberOp m_functor;
};

#define EIGEN_MEMBER_FUNCTOR(MEMBER,COST)                           \
  template <typename ResultType>                                    \
  struct ei_member_##MEMBER EIGEN_EMPTY_STRUCT {                    \
    typedef ResultType result_type;                                 \
    template<typename Scalar, int Size> struct Cost                 \
    { enum { value = COST }; };                                     \
    template<typename Derived>                                      \
    inline ResultType operator()(const MatrixBase<Derived>& mat) const     \
    { return mat.MEMBER(); } \
  }

EIGEN_MEMBER_FUNCTOR(squaredNorm, Size * NumTraits<Scalar>::MulCost + (Size-1)*NumTraits<Scalar>::AddCost);
EIGEN_MEMBER_FUNCTOR(norm, (Size+5) * NumTraits<Scalar>::MulCost + (Size-1)*NumTraits<Scalar>::AddCost);
EIGEN_MEMBER_FUNCTOR(stableNorm, (Size+5) * NumTraits<Scalar>::MulCost + (Size-1)*NumTraits<Scalar>::AddCost);
EIGEN_MEMBER_FUNCTOR(blueNorm, (Size+5) * NumTraits<Scalar>::MulCost + (Size-1)*NumTraits<Scalar>::AddCost);
EIGEN_MEMBER_FUNCTOR(hypotNorm, (Size-1) * ei_functor_traits<ei_scalar_hypot_op<Scalar> >::Cost );
EIGEN_MEMBER_FUNCTOR(sum, (Size-1)*NumTraits<Scalar>::AddCost);
EIGEN_MEMBER_FUNCTOR(mean, (Size-1)*NumTraits<Scalar>::AddCost + NumTraits<Scalar>::MulCost);
EIGEN_MEMBER_FUNCTOR(minCoeff, (Size-1)*NumTraits<Scalar>::AddCost);
EIGEN_MEMBER_FUNCTOR(maxCoeff, (Size-1)*NumTraits<Scalar>::AddCost);
EIGEN_MEMBER_FUNCTOR(all, (Size-1)*NumTraits<Scalar>::AddCost);
EIGEN_MEMBER_FUNCTOR(any, (Size-1)*NumTraits<Scalar>::AddCost);
EIGEN_MEMBER_FUNCTOR(count, (Size-1)*NumTraits<Scalar>::AddCost);
EIGEN_MEMBER_FUNCTOR(prod, (Size-1)*NumTraits<Scalar>::MulCost);


/** \internal */
template <typename BinaryOp, typename Scalar>
struct ei_member_redux {
  typedef typename ei_result_of<
                     BinaryOp(Scalar)
                   >::type  result_type;
  template<typename _Scalar, int Size> struct Cost
  { enum { value = (Size-1) * ei_functor_traits<BinaryOp>::Cost }; };
  ei_member_redux(const BinaryOp func) : m_functor(func) {}
  template<typename Derived>
  inline result_type operator()(const MatrixBase<Derived>& mat) const
  { return mat.redux(m_functor); }
  const BinaryOp m_functor;
};

/** \array_module \ingroup Array_Module
  *
  * \class VectorwiseOp
  *
  * \brief Pseudo expression providing partial reduction operations
  *
  * \param ExpressionType the type of the object on which to do partial reductions
  * \param Direction indicates the direction of the redux (Vertical or Horizontal)
  *
  * This class represents a pseudo expression with partial reduction features.
  * It is the return type of MatrixBase::colwise() and MatrixBase::rowwise()
  * and most of the time this is the only way it is used.
  *
  * Example: \include MatrixBase_colwise.cpp
  * Output: \verbinclude MatrixBase_colwise.out
  *
  * \sa MatrixBase::colwise(), MatrixBase::rowwise(), class PartialReduxExpr
  */
template<typename ExpressionType, int Direction> class VectorwiseOp
{
  public:

    typedef typename ei_traits<ExpressionType>::Scalar Scalar;
    typedef typename ei_meta_if<ei_must_nest_by_value<ExpressionType>::ret,
        ExpressionType, const ExpressionType&>::ret ExpressionTypeNested;

    template<template<typename _Scalar> class Functor> struct ReturnType
    {
      typedef PartialReduxExpr<ExpressionType,
                               Functor<typename ei_traits<ExpressionType>::Scalar>,
                               Direction
                              > Type;
    };

    template<typename BinaryOp> struct ReduxReturnType
    {
      typedef PartialReduxExpr<ExpressionType,
                               ei_member_redux<BinaryOp,typename ei_traits<ExpressionType>::Scalar>,
                               Direction
                              > Type;
    };

    enum {
      IsVertical   = (Direction==Vertical) ? 1 : 0,
      IsHorizontal = (Direction==Horizontal) ? 1 : 0
    };

  protected:

    /** \internal
      * \returns the i-th subvector according to the \c Direction */
    typedef typename ei_meta_if<Direction==Vertical,
                               typename ExpressionType::ColXpr,
                               typename ExpressionType::RowXpr>::ret SubVector;
    SubVector subVector(int i)
    {
      return SubVector(m_matrix.derived(),i);
    }

    /** \internal
      * \returns the number of subvectors in the direction \c Direction */
    int subVectors() const
    { return Direction==Vertical?m_matrix.cols():m_matrix.rows(); }

    template<typename OtherDerived> struct ExtendedType {
      typedef Replicate<OtherDerived,
                        Direction==Vertical   ? 1 : ExpressionType::RowsAtCompileTime,
                        Direction==Horizontal ? 1 : ExpressionType::ColsAtCompileTime> Type;
    };

    /** \internal
      * Replicates a vector to match the size of \c *this */
    template<typename OtherDerived>
    typename ExtendedType<OtherDerived>::Type
    extendedTo(const MatrixBase<OtherDerived>& other) const
    {
      EIGEN_STATIC_ASSERT_VECTOR_ONLY(OtherDerived);
      return typename ExtendedType<OtherDerived>::Type
                      (other.derived(),
                       Direction==Vertical   ? 1 : m_matrix.rows(),
                       Direction==Horizontal ? 1 : m_matrix.cols());
    }

  public:

    inline VectorwiseOp(const ExpressionType& matrix) : m_matrix(matrix) {}

    /** \internal */
    inline const ExpressionType& _expression() const { return m_matrix; }

    /** \returns a row or column vector expression of \c *this reduxed by \a func
      *
      * The template parameter \a BinaryOp is the type of the functor
      * of the custom redux operator. Note that func must be an associative operator.
      *
      * \sa class VectorwiseOp, MatrixBase::colwise(), MatrixBase::rowwise()
      */
    template<typename BinaryOp>
    const typename ReduxReturnType<BinaryOp>::Type
    redux(const BinaryOp& func = BinaryOp()) const
    { return typename ReduxReturnType<BinaryOp>::Type(_expression(), func); }

    /** \returns a row (or column) vector expression of the smallest coefficient
      * of each column (or row) of the referenced expression.
      *
      * Example: \include PartialRedux_minCoeff.cpp
      * Output: \verbinclude PartialRedux_minCoeff.out
      *
      * \sa MatrixBase::minCoeff() */
    const typename ReturnType<ei_member_minCoeff>::Type minCoeff() const
    { return _expression(); }

    /** \returns a row (or column) vector expression of the largest coefficient
      * of each column (or row) of the referenced expression.
      *
      * Example: \include PartialRedux_maxCoeff.cpp
      * Output: \verbinclude PartialRedux_maxCoeff.out
      *
      * \sa MatrixBase::maxCoeff() */
    const typename ReturnType<ei_member_maxCoeff>::Type maxCoeff() const
    { return _expression(); }

    /** \returns a row (or column) vector expression of the squared norm
      * of each column (or row) of the referenced expression.
      *
      * Example: \include PartialRedux_squaredNorm.cpp
      * Output: \verbinclude PartialRedux_squaredNorm.out
      *
      * \sa MatrixBase::squaredNorm() */
    const typename ReturnType<ei_member_squaredNorm>::Type squaredNorm() const
    { return _expression(); }

    /** \returns a row (or column) vector expression of the norm
      * of each column (or row) of the referenced expression.
      *
      * Example: \include PartialRedux_norm.cpp
      * Output: \verbinclude PartialRedux_norm.out
      *
      * \sa MatrixBase::norm() */
    const typename ReturnType<ei_member_norm>::Type norm() const
    { return _expression(); }


    /** \returns a row (or column) vector expression of the norm
      * of each column (or row) of the referenced expression, using
      * blue's algorithm.
      *
      * \sa MatrixBase::blueNorm() */
    const typename ReturnType<ei_member_blueNorm>::Type blueNorm() const
    { return _expression(); }


    /** \returns a row (or column) vector expression of the norm
      * of each column (or row) of the referenced expression, avoiding
      * underflow and overflow.
      *
      * \sa MatrixBase::stableNorm() */
    const typename ReturnType<ei_member_stableNorm>::Type stableNorm() const
    { return _expression(); }


    /** \returns a row (or column) vector expression of the norm
      * of each column (or row) of the referenced expression, avoiding
      * underflow and overflow using a concatenation of hypot() calls.
      *
      * \sa MatrixBase::hypotNorm() */
    const typename ReturnType<ei_member_hypotNorm>::Type hypotNorm() const
    { return _expression(); }

    /** \returns a row (or column) vector expression of the sum
      * of each column (or row) of the referenced expression.
      *
      * Example: \include PartialRedux_sum.cpp
      * Output: \verbinclude PartialRedux_sum.out
      *
      * \sa MatrixBase::sum() */
    const typename ReturnType<ei_member_sum>::Type sum() const
    { return _expression(); }

    /** \returns a row (or column) vector expression of the mean
    * of each column (or row) of the referenced expression.
    *
    * \sa MatrixBase::mean() */
    const typename ReturnType<ei_member_mean>::Type mean() const
    { return _expression(); }

    /** \returns a row (or column) vector expression representing
      * whether \b all coefficients of each respective column (or row) are \c true.
      *
      * \sa MatrixBase::all() */
    const typename ReturnType<ei_member_all>::Type all() const
    { return _expression(); }

    /** \returns a row (or column) vector expression representing
      * whether \b at \b least one coefficient of each respective column (or row) is \c true.
      *
      * \sa MatrixBase::any() */
    const typename ReturnType<ei_member_any>::Type any() const
    { return _expression(); }

    /** \returns a row (or column) vector expression representing
      * the number of \c true coefficients of each respective column (or row).
      *
      * Example: \include PartialRedux_count.cpp
      * Output: \verbinclude PartialRedux_count.out
      *
      * \sa MatrixBase::count() */
    const PartialReduxExpr<ExpressionType, ei_member_count<int>, Direction> count() const
    { return _expression(); }

    /** \returns a row (or column) vector expression of the product
      * of each column (or row) of the referenced expression.
      *
      * Example: \include PartialRedux_prod.cpp
      * Output: \verbinclude PartialRedux_prod.out
      *
      * \sa MatrixBase::prod() */
    const typename ReturnType<ei_member_prod>::Type prod() const
    { return _expression(); }


    /** \returns a matrix expression
      * where each column (or row) are reversed.
      *
      * Example: \include PartialRedux_reverse.cpp
      * Output: \verbinclude PartialRedux_reverse.out
      *
      * \sa MatrixBase::reverse() */
    const Reverse<ExpressionType, Direction> reverse() const
    { return Reverse<ExpressionType, Direction>( _expression() ); }

    const Replicate<ExpressionType,Direction==Vertical?Dynamic:1,Direction==Horizontal?Dynamic:1>
    replicate(int factor) const;

    /** \nonstableyet
      * \return an expression of the replication of each column (or row) of \c *this
      *
      * Example: \include DirectionWise_replicate.cpp
      * Output: \verbinclude DirectionWise_replicate.out
      *
      * \sa VectorwiseOp::replicate(int), MatrixBase::replicate(), class Replicate
      */
    // NOTE implemented here because of sunstudio's compilation errors
    template<int Factor> const Replicate<ExpressionType,(IsVertical?Factor:1),(IsHorizontal?Factor:1)>
    replicate(int factor = Factor) const
    {
      return Replicate<ExpressionType,Direction==Vertical?Factor:1,Direction==Horizontal?Factor:1>
          (_expression(),Direction==Vertical?factor:1,Direction==Horizontal?factor:1);
    }

/////////// Artithmetic operators ///////////

    /** Copies the vector \a other to each subvector of \c *this */
    template<typename OtherDerived>
    ExpressionType& operator=(const MatrixBase<OtherDerived>& other)
    {
      EIGEN_STATIC_ASSERT_VECTOR_ONLY(OtherDerived)
      //ei_assert((m_matrix.isNull()) == (other.isNull())); FIXME
      for(int j=0; j<subVectors(); ++j)
        subVector(j) = other;
      return const_cast<ExpressionType&>(m_matrix);
    }

    /** Adds the vector \a other to each subvector of \c *this */
    template<typename OtherDerived>
    ExpressionType& operator+=(const MatrixBase<OtherDerived>& other)
    {
      EIGEN_STATIC_ASSERT_VECTOR_ONLY(OtherDerived)
      for(int j=0; j<subVectors(); ++j)
        subVector(j) += other;
      return const_cast<ExpressionType&>(m_matrix);
    }

    /** Substracts the vector \a other to each subvector of \c *this */
    template<typename OtherDerived>
    ExpressionType& operator-=(const MatrixBase<OtherDerived>& other)
    {
      EIGEN_STATIC_ASSERT_VECTOR_ONLY(OtherDerived)
      for(int j=0; j<subVectors(); ++j)
        subVector(j) -= other;
      return const_cast<ExpressionType&>(m_matrix);
    }

    /** Returns the expression of the sum of the vector \a other to each subvector of \c *this */
    template<typename OtherDerived>
    CwiseBinaryOp<ei_scalar_sum_op<Scalar>,
                  ExpressionType,
                  typename ExtendedType<OtherDerived>::Type>
    operator+(const MatrixBase<OtherDerived>& other) const
    {
      EIGEN_STATIC_ASSERT_VECTOR_ONLY(OtherDerived);
      return m_matrix + extendedTo(other).nestByValue();
    }

    /** Returns the expression of the difference between each subvector of \c *this and the vector \a other */
    template<typename OtherDerived>
    CwiseBinaryOp<ei_scalar_difference_op<Scalar>,
                  ExpressionType,
                  typename ExtendedType<OtherDerived>::Type>
    operator-(const MatrixBase<OtherDerived>& other) const
    {
      EIGEN_STATIC_ASSERT_VECTOR_ONLY(OtherDerived);
      return m_matrix - extendedTo(other).nestByValue();
    }

/////////// Geometry module ///////////

    const Homogeneous<ExpressionType,Direction> homogeneous() const;

    typedef typename ExpressionType::PlainMatrixType CrossReturnType;
    template<typename OtherDerived>
    const CrossReturnType cross(const MatrixBase<OtherDerived>& other) const;

    enum {
      HNormalized_Size = Direction==Vertical ? ei_traits<ExpressionType>::RowsAtCompileTime
                                             : ei_traits<ExpressionType>::ColsAtCompileTime,
      HNormalized_SizeMinusOne = HNormalized_Size==Dynamic ? Dynamic : HNormalized_Size-1
    };
    typedef Block<ExpressionType,
                  Direction==Vertical   ? int(HNormalized_SizeMinusOne)
                                        : int(ei_traits<ExpressionType>::RowsAtCompileTime),
                  Direction==Horizontal ? int(HNormalized_SizeMinusOne)
                                        : int(ei_traits<ExpressionType>::ColsAtCompileTime)>
            HNormalized_Block;
    typedef Block<ExpressionType,
                  Direction==Vertical   ? 1 : int(ei_traits<ExpressionType>::RowsAtCompileTime),
                  Direction==Horizontal ? 1 : int(ei_traits<ExpressionType>::ColsAtCompileTime)>
            HNormalized_Factors;
    typedef CwiseBinaryOp<ei_scalar_quotient_op<typename ei_traits<ExpressionType>::Scalar>,
                HNormalized_Block,
                Replicate<HNormalized_Factors,
                  Direction==Vertical   ? HNormalized_SizeMinusOne : 1,
                  Direction==Horizontal ? HNormalized_SizeMinusOne : 1> >
            HNormalizedReturnType;

    const HNormalizedReturnType hnormalized() const;

  protected:
    ExpressionTypeNested m_matrix;

  private:
    VectorwiseOp& operator=(const VectorwiseOp&);
};

/** \array_module
  *
  * \returns a VectorwiseOp wrapper of *this providing additional partial reduction operations
  *
  * Example: \include MatrixBase_colwise.cpp
  * Output: \verbinclude MatrixBase_colwise.out
  *
  * \sa rowwise(), class VectorwiseOp
  */
template<typename Derived>
inline const VectorwiseOp<Derived,Vertical>
MatrixBase<Derived>::colwise() const
{
  return derived();
}

/** \array_module
  *
  * \returns a writable VectorwiseOp wrapper of *this providing additional partial reduction operations
  *
  * \sa rowwise(), class VectorwiseOp
  */
template<typename Derived>
inline VectorwiseOp<Derived,Vertical>
MatrixBase<Derived>::colwise()
{
  return derived();
}

/** \array_module
  *
  * \returns a VectorwiseOp wrapper of *this providing additional partial reduction operations
  *
  * Example: \include MatrixBase_rowwise.cpp
  * Output: \verbinclude MatrixBase_rowwise.out
  *
  * \sa colwise(), class VectorwiseOp
  */
template<typename Derived>
inline const VectorwiseOp<Derived,Horizontal>
MatrixBase<Derived>::rowwise() const
{
  return derived();
}

/** \array_module
  *
  * \returns a writable VectorwiseOp wrapper of *this providing additional partial reduction operations
  *
  * \sa colwise(), class VectorwiseOp
  */
template<typename Derived>
inline VectorwiseOp<Derived,Horizontal>
MatrixBase<Derived>::rowwise()
{
  return derived();
}

#endif // EIGEN_PARTIAL_REDUX_H
