

#ifndef IPOPT_INTERFACE_H
#define IPOPT_INTERFACE_H
#include <cstddef>
#include <eigen3/Eigen/Core>
#include <gsplines/FunctionalAnalysis/Sobolev.hpp>
#include <ifopt/constraint_set.h>
#include <ifopt/cost_term.h>
#include <ifopt/variable_set.h>

namespace gsplines {
namespace optimization {

class TimeSegmentLenghtsVar : public ifopt::VariableSet {
private:
  double exec_time_;
  Eigen::VectorXd values_;
  ifopt::Component::Component::VecBound bounds_;

public:
  TimeSegmentLenghtsVar(std::size_t _num_intervals, double _exec_time);
  virtual ~TimeSegmentLenghtsVar();
  void SetVariables(const Eigen::VectorXd &_vec) override;
  Eigen::VectorXd GetValues() const override;
  ifopt::Component::VecBound GetBounds() const override;
};

class ExecTimeConstraint : public ifopt::ConstraintSet {
public:
  ExecTimeConstraint(std::size_t _num_intervals, double _exec_time);

  Eigen::VectorXd GetValues() const override;
  ifopt::Component::VecBound GetBounds() const override;

  void FillJacobianBlock(std::string _set_name,
                         Jacobian &_jac_block) const override;

  virtual ~ExecTimeConstraint() = default;

private:
  Eigen::VectorXd values_;
  double exec_time_;
  ifopt::Component::VecBound bounds_;
};

class SobolevNorm : public ifopt::CostTerm {
public:
  SobolevNorm(std::string _name,
              const Eigen::Ref<const Eigen::MatrixXd> _waypoints,
              const gsplines::basis::Basis &_basis,
              std::vector<std::pair<std::size_t, double>> _weights);

  double GetCost() const override;
  void FillJacobianBlock(std::string var_set, Jacobian &jac) const override;
  virtual ~SobolevNorm() = default;

private:
  /* data */
  std::unique_ptr<gsplines::basis::Basis> basis_;
  std::vector<std::pair<std::size_t, double>> weights_;
  Eigen::MatrixXd waypoints_;
  mutable ::gsplines::functional_analysis::SobolevNorm sobol_norm_;
  mutable Eigen::VectorXd buff_1_;
  mutable Eigen::VectorXd buff_2_;
};

} // namespace optimization
} // namespace gsplines
#endif /* IPOPT_INTERFACE_H */
