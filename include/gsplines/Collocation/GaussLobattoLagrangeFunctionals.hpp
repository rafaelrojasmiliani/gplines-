#include <algorithm>
#include <gsplines/Collocation/GaussLobattoLagrange.hpp>
#include <gsplines/FunctionalAnalysis/Sobolev.hpp>
#include <ifopt/constraint_set.h>
#include <ifopt/cost_term.h>
#include <ifopt/variable_set.h>
#include <stdexcept>
#ifndef GAUSSLOBATTOLAGRANGEFUNCTIONALS_H
#define GAUSSLOBATTOLAGRANGEFUNCTIONALS_H

namespace gsplines {
namespace collocation {

class LinearFunctionalBase;
class Functional {
private:
  std::size_t codom_dim_;

public:
  Functional(std::size_t _codom_dim) : codom_dim_(_codom_dim){};

  virtual Eigen::VectorXd
  operator()(const GaussLobattoLagrangeSpline &_in) const = 0;

  virtual std::shared_ptr<LinearFunctionalBase>
  differential(const GaussLobattoLagrangeSpline &_in) const = 0;

  std::size_t get_codom_dim() const { return codom_dim_; }

  virtual ~Functional() = default;
};

class LinearFunctionalBase : public Functional {
public:
  using Functional::Functional;

  LinearFunctionalBase(const LinearFunctionalBase &_that) = default;

  virtual Eigen::MatrixXd to_dense_matrix() const = 0;

  virtual Eigen::SparseMatrix<double> to_sparse_matrix() const = 0;

  ~LinearFunctionalBase() = default;
};

template <typename T> class LinearFunctional : public LinearFunctionalBase {
  template <typename M>
  friend LinearFunctional<M> operator*(double, const LinearFunctional<M> &);
  template <typename M>
  friend LinearFunctional<M> &&operator*(double, LinearFunctional<M> &&);

private:
  LinearFunctional<T> &operator=(const LinearFunctional<T> &that) = delete;

public:
  T mat_;
  LinearFunctional(const T &_args)
      : LinearFunctionalBase(_args.rows()), mat_(_args) {}

  LinearFunctional(T &&_args)
      : LinearFunctionalBase(_args.rows()), mat_(std::move(_args)) {}

  LinearFunctional(const LinearFunctional<T> &_that)
      : LinearFunctionalBase(_that.get_codom_dim()), mat_(_that.mat_) {}

  LinearFunctional(LinearFunctional<T> &&_that)
      : LinearFunctionalBase(_that.get_codom_dim()),
        mat_(std::move(_that.mat_)) {}

  LinearFunctional(long _rows, long _cols)
      : LinearFunctionalBase((std::size_t)_rows), mat_(_rows, _cols) {}

  const T &to_matrix() const { return mat_; }

  template <typename M> void set(LinearFunctional<M> &&_other) {
    if constexpr (std::is_same_v<T, M>) {
      mat_ = _other.mat_;
      return;
    } else if constexpr (not std::is_same_v<Eigen::MatrixXd, T> and
                         not std::is_same_v<Eigen::MatrixXd, M>) {
      mat_ = _other.mat_;
      return;
    } else if constexpr (std::is_same_v<Eigen::MatrixXd, T> and
                         not std::is_same_v<Eigen::MatrixXd, M>) {
      mat_ = _other.mat_.toDense();
      return;
    } else if constexpr (std::is_same_v<Eigen::MatrixXd, T> and
                         not std::is_same_v<Eigen::MatrixXd, M>) {
      mat_ = _other.mat_.toDense();
      return;
    }
    throw std::runtime_error("This part of the code is not implement. An error "
                             "on expected types: LinearFunctional");
  }

  template <typename M> void set(const LinearFunctional<M> &_other) {
    if constexpr (std::is_same_v<T, M>) {
      mat_ = _other.mat_;
      return;
    } else if constexpr (not std::is_same_v<Eigen::MatrixXd, T> and
                         not std::is_same_v<Eigen::MatrixXd, M>) {
      mat_ = _other.mat_;
      return;
    } else if constexpr (std::is_same_v<Eigen::MatrixXd, T> and
                         not std::is_same_v<Eigen::MatrixXd, M>) {
      mat_ = _other.mat_.toDense();
      return;
    } else if constexpr (std::is_same_v<Eigen::MatrixXd, T> and
                         not std::is_same_v<Eigen::MatrixXd, M>) {
      mat_ = _other.mat_.toDense();
      return;
    }

    throw std::runtime_error("This part of the code is not implement. An error "
                             "on expected types: LinearFunctional");
  }

  template <typename M>
  inline auto operator*(const LinearFunctional<M> &_rhs) const {
    return LinearFunctional(to_matrix() * _rhs.to_matrix());
  }

  template <typename M>
  inline auto operator+(const LinearFunctional<M> &_rhs) const {
    return LinearFunctional(to_matrix() + _rhs.to_matrix());
  }
  template <typename M>
  inline auto operator-(const LinearFunctional<M> &_rhs) const {
    return LinearFunctional(to_matrix() - _rhs.to_matrix());
  }

  inline LinearFunctional<T> operator-() const {
    LinearFunctional<T> result(*this);
    result.mat_ *= -1.0;
    return result;
  }

  Eigen::VectorXd
  operator()(const GaussLobattoLagrangeSpline &_in) const override {
    return mat_ * _in.get_coefficients();
  }

  GaussLobattoLagrangeSpline
  operator*(const GaussLobattoLagrangeSpline &_in) const {

    Eigen::VectorXd coeff = to_matrix() * _in.get_coefficients();

    std::size_t codom_dim = to_matrix().rows() / _in.get_number_of_intervals() /
                            _in.get_basis().get_dim();

    return GaussLobattoLagrangeSpline(
        _in.get_domain(), codom_dim, _in.get_number_of_intervals(),
        _in.get_basis().get_dim(), to_matrix() * _in.get_coefficients(),
        _in.get_interval_lengths());
  }

  Eigen::MatrixXd to_dense_matrix() const override {
    if constexpr (std::is_same_v<Eigen::MatrixXd, T>) {
      return mat_;
    } else {
      return mat_.toDense();
    }
    throw std::runtime_error("This part of the code is not implement. An error "
                             "on expected types: LinearFunctional");
    return Eigen::MatrixXd();
  }

  Eigen::SparseMatrix<double> to_sparse_matrix() const override {

    if constexpr (std::is_same_v<Eigen::MatrixXd, T>) {
      return mat_.sparseView();
    } else {
      Eigen::SparseMatrix<double> result(mat_);
      return result;
    }
    throw std::runtime_error("This part of the code is not implement. An error "
                             "on expected types: LinearFunctional");
    return Eigen::SparseMatrix<double>();
  }

  std::shared_ptr<LinearFunctionalBase>
  differential(const GaussLobattoLagrangeSpline & /*_in*/) const override {
    return std::make_shared<LinearFunctional<T>>(*this);
  }

  ~LinearFunctional() = default;
};

template <typename T>
LinearFunctional<T> operator*(double _num, const LinearFunctional<T> &_lm) {
  LinearFunctional<T> result(_lm);
  result.mat_ *= _num;
  return result;
}

template <typename T>
LinearFunctional<T> &&operator*(double _num, LinearFunctional<T> &&_lm) {
  _lm.mat_ *= _num;
  return std::move(_lm);
}

typedef LinearFunctional<Eigen::SparseMatrix<double>> LinearFunctionalSparse;

typedef LinearFunctional<Eigen::MatrixXd> LinearFunctionalDense;

class Derivative : public LinearFunctionalSparse {

private:
  gsplines::basis::BasisLagrangeGaussLobatto basis_;
  const std::size_t nglp_;
  const std::size_t n_intervals_;
  const std::size_t codom_dim_;
  const std::size_t deg_;

public:
  Derivative(std::pair<double, double> _domain, std::size_t _codom_dim,
             std::size_t _n_glp, std::size_t _n_intervals, std::size_t _deg = 1)
      : Derivative(_codom_dim, _n_glp, _n_intervals,
                   Eigen::VectorXd::Ones(_n_intervals) *
                       (_domain.second - _domain.first) / _n_intervals,
                   _deg) {}

  Derivative(std::size_t _codom_dim, std::size_t _n_glp,
             std::size_t _n_intervals, const Eigen::VectorXd &_int_lengs,
             std::size_t _deg = 1)
      : LinearFunctionalSparse(_n_glp * _n_intervals * _codom_dim,
                               _n_glp * _n_intervals * _codom_dim),
        basis_(_n_glp), nglp_(_n_glp), n_intervals_(_n_intervals),
        codom_dim_(_codom_dim), deg_(_deg) {

    const Eigen::MatrixXd &d_mat = basis_.get_derivative_matrix_block(deg_);
    std::size_t total_size = _n_glp * _n_intervals * _codom_dim;
    // ---
    for (std::size_t uici = 0; uici < total_size; uici += _n_glp) {
      for (std::size_t i = 0; i < _n_glp; i++) {
        for (std::size_t j = 0; j < _n_glp; j++) {
          mat_.block(uici, uici, _n_glp, _n_glp).coeffRef(i, j) =
              2 * d_mat(i, j) / _int_lengs(0);
        }
      }
    }
    // ---
    mat_.makeCompressed();
  }

  Derivative(const GaussLobattoLagrangeSpline &_that)
      : Derivative(_that.get_codom_dim(), _that.get_basis().get_dim(),
                   _that.get_number_of_intervals(),
                   _that.get_interval_lengths()) {}

  void update(const Eigen::VectorXd &_int_lengs) {

    const Eigen::MatrixXd &d_mat = basis_.get_derivative_matrix_block();
    std::size_t total_size = nglp_ * n_intervals_ * codom_dim_;
    // ---
    for (std::size_t uici = 0; uici < total_size; uici += nglp_) {
      for (std::size_t i = 0; i < nglp_; i++) {
        for (std::size_t j = 0; j < nglp_; j++) {
          mat_.block(uici, uici, nglp_, nglp_).coeffRef(i, j) =
              2 * d_mat(i, j) / _int_lengs(0);
        }
      }
    }
    // ---
    mat_.makeCompressed();
  }
};

class TransposeLeftMultiplication : public LinearFunctionalSparse {
private:
  const std::size_t nglp_;
  const std::size_t n_inter_;
  const std::size_t codom_dim_;

public:
  TransposeLeftMultiplication(std::size_t _codom_dim, std::size_t _n_glp,
                              std::size_t _n_intervals)
      : LinearFunctionalSparse(_n_glp * _n_intervals,
                               _n_glp * _n_intervals * _codom_dim),
        nglp_(_n_glp), n_inter_(_n_intervals), codom_dim_(_codom_dim) {}

  TransposeLeftMultiplication(const GaussLobattoLagrangeSpline &_that)
      : LinearFunctionalSparse(
            _that.get_basis().get_dim() * _that.get_number_of_intervals(),
            _that.get_basis().get_dim() * _that.get_number_of_intervals() *
                _that.get_codom_dim()),
        nglp_(_that.get_basis().get_dim()),
        n_inter_(_that.get_number_of_intervals()),
        codom_dim_(_that.get_codom_dim()) {

    std::size_t n_glp = _that.get_basis().get_dim();
    std::size_t n_inter = _that.get_number_of_intervals();
    std::size_t codom_dim = _that.get_codom_dim();

    const Eigen::VectorXd &vec = _that.get_coefficients();
    // ---
    for (std::size_t uic_interval = 0; uic_interval < n_inter; uic_interval++) {
      for (std::size_t uic_coor = 0; uic_coor < codom_dim; uic_coor++) {
        std::size_t i0 = uic_interval * n_glp;
        std::size_t j0 = uic_coor * n_glp + uic_interval * n_glp * codom_dim;

        for (std::size_t i = 0; i < n_glp; i++) {

          mat_.coeffRef(i0 + i, j0 + i) =
              vec(uic_interval * n_glp * codom_dim + uic_coor * n_glp + i);
        }
      }
    }
    mat_.makeCompressed();
  }

  TransposeLeftMultiplication &
  operator=(const gsplines::functions::FunctionBase &_that) {
    if (_that.get_codom_dim() != codom_dim_)
      throw std::invalid_argument("");

    const Eigen::VectorXd vec =
        GaussLobattoLagrangeSpline::approximate(_that, nglp_, n_inter_)
            .get_coefficients();
    // ---
    for (std::size_t uic_interval = 0; uic_interval < n_inter_;
         uic_interval++) {
      for (std::size_t uic_coor = 0; uic_coor < codom_dim_; uic_coor++) {
        std::size_t i0 = uic_interval * nglp_;
        std::size_t j0 = uic_coor * nglp_ + uic_interval * nglp_ * codom_dim_;

        for (std::size_t i = 0; i < nglp_; i++) {

          mat_.coeffRef(i0 + i, j0 + i) =
              vec(uic_interval * nglp_ * codom_dim_ + uic_coor * nglp_ + i);
        }
      }
    }
    mat_.makeCompressed();
    return *this;
  }

  TransposeLeftMultiplication &
  operator=(const GaussLobattoLagrangeSpline &_that) {
    if (_that.get_codom_dim() != codom_dim_)
      throw std::invalid_argument("");

    const Eigen::VectorXd &vec = _that.get_coefficients();
    // ---
    for (std::size_t uic_interval = 0; uic_interval < n_inter_;
         uic_interval++) {
      for (std::size_t uic_coor = 0; uic_coor < codom_dim_; uic_coor++) {
        std::size_t i0 = uic_interval * nglp_;
        std::size_t j0 = uic_coor * nglp_ + uic_interval * nglp_ * codom_dim_;

        for (std::size_t i = 0; i < nglp_; i++) {

          mat_.coeffRef(i0 + i, j0 + i) =
              vec(uic_interval * nglp_ * codom_dim_ + uic_coor * nglp_ + i);
        }
      }
    }
    mat_.makeCompressed();
    return *this;
  }
};
class MatrixLeftMultiplication : public LinearFunctionalSparse {
private:
  const std::size_t nglp_;
  const std::size_t n_inter_;
  const long matrix_cols_;
  const long matrix_rows_;

public:
  MatrixLeftMultiplication(std::size_t _matrix_dom_dim, std::size_t _n_glp,
                           std::size_t _n_intervals,
                           std::size_t _matrix_codom_dim)
      : LinearFunctionalSparse(_n_glp * _n_intervals * _matrix_codom_dim,
                               _n_glp * _n_intervals * _matrix_dom_dim),
        nglp_(_n_glp), n_inter_(_n_intervals), matrix_cols_(_matrix_dom_dim),
        matrix_rows_(_matrix_codom_dim) {

    for (std::size_t interval = 0; interval < n_inter_; interval++) {

      for (long col = 0; col < matrix_cols_; col++) {

        for (long row = 0; row < matrix_rows_; row++) {

          std::size_t j0 = col * nglp_ + interval * nglp_ * matrix_cols_;
          std::size_t i0 = row * nglp_ + interval * nglp_ * matrix_rows_;

          for (std::size_t i = 0; i < nglp_; i++) {

            mat_.coeffRef(i0 + i, j0 + i) = 1.0;
          }
        }
      }
    }
    mat_.makeCompressed();
  }

  template <typename IteratorType, typename Function>
  void update(const IteratorType &_begin, const IteratorType &_end,
              Function _fun) {

    if (std::distance(_begin, _end) != nglp_ * n_inter_) {
      throw std::runtime_error("wrong number of elements");
    }

    for (auto it = _begin; it != _end; ++it) {
      decltype(auto) mat = _fun(*it);

      long index = std::distance(_begin, it);
      long interval = index / nglp_;

      for (long col = 0; col < matrix_cols_; col++) {
        for (long row = 0; row < matrix_cols_; row++) {

          std::size_t j0 =
              col * nglp_ + interval * nglp_ * matrix_cols_ + index % nglp_;
          std::size_t i0 =
              row * nglp_ + interval * nglp_ * matrix_rows_ + index % nglp_;
          mat_.coeffRef(i0, j0) = mat(col, row);
        }
      }
    }
  }
  void update(const Eigen::MatrixXd &_mat) {

    if (_mat.rows() != matrix_rows_ || _mat.cols() != matrix_cols_) {
      throw std::runtime_error("wrong number of elements");
    }

    for (std::size_t index = 0; index < n_inter_ * nglp_; index++) {

      long interval = index / nglp_;

      for (long col = 0; col < matrix_cols_; col++) {
        for (long row = 0; row < matrix_cols_; row++) {

          std::size_t j0 =
              col * nglp_ + interval * nglp_ * matrix_cols_ + index % nglp_;
          std::size_t i0 =
              row * nglp_ + interval * nglp_ * matrix_rows_ + index % nglp_;
          mat_.coeffRef(i0, j0) = _mat(col, row);
        }
      }
    }
  }
};

class ContinuityError
    : public LinearFunctional<Eigen::SparseMatrix<double, Eigen::RowMajor>> {

public:
  ContinuityError(const GaussLobattoLagrangeSpline &_that, std::size_t _deg)
      : LinearFunctional(_that.get_basis().continuity_matrix(
            _that.get_number_of_intervals(), _that.get_codom_dim(), _deg,
            _that.get_interval_lengths())) {}
};

class Integral : public LinearFunctionalDense {
  Eigen::MatrixXd glw_;

public:
  Integral(std::tuple<double, double> _domain, std::size_t _nglp,
           std::size_t _n_intervals);
  Integral(const GaussLobattoLagrangeSpline &_in);
  void update(std::tuple<double, double> _domain, std::size_t _nglp,
              std::size_t _n_intervals);
};

class SobolevDistance : public Functional {
private:
  GaussLobattoLagrangeSpline approx_;
  GaussLobattoLagrangeSpline approx_d_;
  Integral int_;
  Derivative der_;

  std::shared_ptr<LinearFunctionalDense> diff_;

public:
  SobolevDistance(const gsplines::functions::FunctionBase &_fun,
                  std::size_t _nglp, std::size_t _n_inter, std::size_t _deg);
  Eigen::VectorXd
  operator()(const GaussLobattoLagrangeSpline &_in) const override;

  std::shared_ptr<LinearFunctionalBase>
  differential(const GaussLobattoLagrangeSpline & /*_in*/) const override;
};

class GLLSplineVariable : public ifopt::VariableSet,
                          public GaussLobattoLagrangeSpline {
private:
  ifopt::Component::Component::VecBound bounds_;

public:
  GLLSplineVariable(const GaussLobattoLagrangeSpline &_in);
  virtual ~GLLSplineVariable() = default;
  void SetVariables(const Eigen::VectorXd &_vec) override;
  Eigen::VectorXd GetValues() const override;
  ifopt::Component::VecBound GetBounds() const override { return bounds_; };
};

class __Counter {
protected:
  static int counter__;
  __Counter() { counter__++; }
};

template <typename T> class CostWrapper : public T, public ifopt::CostTerm {

  static int counter__;

public:
  template <typename... Ts>
  CostWrapper(Ts &&... args)
      : T(args...),
        CostTerm("cost_" + std::to_string(CostWrapper<T>::counter__)) {}

  double GetCost() const override {

    std::shared_ptr<GaussLobattoLagrangeSpline> spline =
        std::dynamic_pointer_cast<GaussLobattoLagrangeSpline>(
            GetVariables()->GetComponent("GLL"));

    if (not spline)
      throw std::logic_error("could not cast GLL into GLLSpline");

    Eigen::VectorXd result = T::operator()(*spline);

    return result(0);
  }

  void FillJacobianBlock(std::string _set_name,
                         Jacobian &_jac_block) const override {
    std::shared_ptr<GaussLobattoLagrangeSpline> spline =
        std::dynamic_pointer_cast<GaussLobattoLagrangeSpline>(
            GetVariables()->GetComponent("GLL"));
    if (not spline)
      throw std::logic_error("could not cast GLL into GLLSpline");

    if (_set_name == "GLL") {
      _jac_block = T::differential(*spline)->to_sparse_matrix();
    } else {
      throw std::logic_error("The unique variable implement is GLL");
    }
  }
};

template <typename T>
class ConstraintWrapper : public T, public ifopt::ConstraintSet {
private:
  ifopt::Component::VecBound bounds_;
  static int counter__;

public:
  template <typename... Ts>
  ConstraintWrapper(double _min, double _max, Ts &&... args)
      : T(args...),
        ConstraintSet(T::get_codom_dim(),
                      "Wrapper_" +
                          std::to_string(ConstraintWrapper<T>::counter__)) {

    counter__++;
    ifopt::Bounds default_bound(_min, _max);
    bounds_ = ifopt::Component::VecBound(GetRows(), default_bound);
  }

  Eigen::VectorXd GetValues() const override {

    std::shared_ptr<GaussLobattoLagrangeSpline> spline =
        std::dynamic_pointer_cast<GaussLobattoLagrangeSpline>(
            GetVariables()->GetComponent("GLL"));
    if (not spline)
      throw std::logic_error("could not cast GLL into GLLSpline");

    return T::operator()(*spline);
  }

  ifopt::Component::VecBound GetBounds() const override { return bounds_; }

  void FillJacobianBlock(std::string _set_name,
                         Jacobian &_jac_block) const override {
    std::shared_ptr<GaussLobattoLagrangeSpline> spline =
        std::dynamic_pointer_cast<GaussLobattoLagrangeSpline>(
            GetVariables()->GetComponent("GLL"));
    if (not spline)
      throw std::logic_error("could not cast GLL into GLLSpline");
    if (_set_name == "GLL") {
      _jac_block = T::differential(*spline)->to_sparse_matrix();
    } else {
      throw std::logic_error("The unique cost implement is GLL");
    }
  }
};
template <typename T> int ConstraintWrapper<T>::counter__ = 0;
template <typename T> int CostWrapper<T>::counter__ = 0;
} // namespace collocation
} // namespace gsplines
#endif /* GAUSSLOBATTOLAGRANGEFUNCTIONALS_H */
