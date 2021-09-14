#include <gsplines/Optimization/ipopt_solver.hpp>

#include <gsplines/Basis/BasisLegendre.hpp>
#include <gsplines/Optimization/ipopt_solver.hpp>
#include <iostream>

int main() {
  std::size_t number_of_wp = 3;
  std::size_t codom_dim = 7;
  std::size_t nglp = 10;

  double exec_time = (double)number_of_wp - 1.0;
  double ti = 0.5 * exec_time;

  Eigen::MatrixXd wp(Eigen::MatrixXd::Random(number_of_wp, codom_dim));

  gsplines::GSpline trj = gsplines::optimization::optimal_sobolev_norm(
      wp, gsplines::basis::BasisLegendre(6), {{1.0, 3}}, exec_time);

  gsplines::GSpline trj_2 = gsplines::optimization::broken_lines_path(wp);
  gsplines::GSpline trj_3 =
      gsplines::optimization::minimum_acceleration_path(wp);
  gsplines::GSpline trj_4 = gsplines::optimization::minimum_jerk_path(wp);
  gsplines::GSpline trj_5 = gsplines::optimization::minimum_snap_path(wp);
  gsplines::GSpline trj_6 = gsplines::optimization::minimum_crackle_path(wp);
}
