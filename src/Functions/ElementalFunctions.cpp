#include <gsplines++/Functions/ElementalFunctions.hpp>
#include <iostream>
namespace gsplines {
namespace functions {

ConstFunction::ConstFunction(std::pair<double, double> _domain,
                             Eigen::Ref<Eigen::VectorXd> _values)
    : Function(_domain, _values.size(), "ConstFunction"), values_(_values) {}

ConstFunction::ConstFunction(std::pair<double, double> _domain,
                             std::size_t _codom_dim, double _value)
    : Function(_domain, _codom_dim, "ConstFunction"),
      values_(Eigen::VectorXd::Ones(_codom_dim) * _value) {}

ConstFunction::ConstFunction(const ConstFunction &_that)
    : Function(_that), values_(_that.values_) {}

void Exponential::value(const Eigen::Ref<const Eigen::VectorXd> _domain_points,
                        Eigen::Ref<Eigen::MatrixXd> _result) const {
  // std::cout << "Exp \n" << Eigen::exp(_domain_points.array()).matrix() <<
  // '\n';
  _result = Eigen::exp(_domain_points.array()).matrix();
  // std::cout << "................\n";
};

void Cos::value(const Eigen::Ref<const Eigen::VectorXd> _domain_points,
                Eigen::Ref<Eigen::MatrixXd> _result) const {
  // std::cout << "Cos \n" << Eigen::cos(_domain_points.array()).matrix() <<
  // '\n';
  _result = Eigen::cos(_domain_points.array()).matrix();
  // std::cout << "................\n";
};

std::unique_ptr<FunctionExpression> Cos::deriv(int _deg) const {
  printf("DERIVATING COS\n\n");
  return std::make_unique<FunctionExpression>(
      ConstFunction(get_domain(), 1, -1.0) * Sin(get_domain()));
}

void Sin::value(const Eigen::Ref<const Eigen::VectorXd> _domain_points,
                Eigen::Ref<Eigen::MatrixXd> _result) const {
  // std::cout << "Sin \n" << Eigen::sin(_domain_points.array()).matrix() <<
  // '\n';
  _result = Eigen::sin(_domain_points.array()).matrix();
  // std::cout << "................\n";
};

std::unique_ptr<FunctionExpression> Sin::deriv(int _deg) const {
  return std::make_unique<Cos>(get_domain());
}

void CanonicPolynomial::value(
    const Eigen::Ref<const Eigen::VectorXd> _domain_points,
    Eigen::Ref<Eigen::MatrixXd> _result) const {

  _result.setConstant(coefficients_.tail(0)(0));
  for (std::size_t uici = 1; uici < coefficients_.size(); uici++) {
    _result.array() *= _domain_points.array();
    _result.array() += coefficients_.tail(uici)(0);
  }
}

std::unique_ptr<FunctionExpression> CanonicPolynomial::deriv(int _deg) const {

  Eigen::VectorXd result(coefficients_);
  for (std::size_t uici = 0; uici < coefficients_.size(); uici++) {
    result(uici) *= (double)uici;
  }
  return std::make_unique<CanonicPolynomial>(get_domain(), std::move(result));
}

} // namespace functions
} // namespace gsplines
