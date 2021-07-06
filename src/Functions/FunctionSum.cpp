
#include <gsplines++/Functions/FunctionExpression.hpp>
#include<iostream>
namespace gsplines {
namespace functions {

FunctionExpression& FunctionExpression::operator+=(const Function& that){

  if (not FunctionBase::same_domain(*this, that)) {
    throw std::invalid_argument("Functions with different domains");
  }
  if (not FunctionBase::same_codomain(*this, that)) {
    throw std::invalid_argument("Functions with different codomains");
  }

    if(type_ != SUM){
        type_ = SUM;

        std::unique_ptr<Function> self_copy = std::make_unique<FunctionExpression>(get_domain(), get_codom_dim(),SUM,
                            std::move(function_array_));

        function_array_.clear();
        function_array_.push_back(std::move(self_copy));

          eval_operation_ = eval_sum_functions;
          deriv_operation_ = deriv_sum_functions;
    }

    function_array_.push_back(that.clone());

    return *this;
}

FunctionExpression& FunctionExpression::operator+=(const FunctionExpression& that){
    printf("llllllllllllllllllllllllllllllll\n");

  if (not FunctionBase::same_domain(*this, that)) {
    throw std::invalid_argument("Functions with different domains");
  }
  if (not FunctionBase::same_codomain(*this, that)) {
    throw std::invalid_argument("Functions with different codomains");
  }

    if(type_ != SUM){
    printf("this is not a sum\n");
        type_ = SUM;

        std::unique_ptr<Function> self_copy = std::make_unique<FunctionExpression>(get_domain(), get_codom_dim(),SUM,
                            std::move(function_array_));

        function_array_.clear();
        function_array_.push_back(std::move(self_copy));

          eval_operation_ = eval_sum_functions;
          deriv_operation_ = deriv_sum_functions;
    }else if(that.type_ == SUM){
    printf("both are sums sum\n");
        for(const std::unique_ptr<Function>& f:that.function_array_){
            function_array_.push_back(f->clone());
        }
        return *this;
    }
    fflush(stdout);

    function_array_.push_back(that.clone());

    return *this;
}



FunctionExpression operator+(const FunctionExpression &_f1, const Function &_f2){
  if (not FunctionBase::same_domain(_f1, _f2)) {
    throw std::invalid_argument("Functions with different domains");
  }
  if (not FunctionBase::same_codomain(_f1, _f2)) {
    throw std::invalid_argument("Functions with different codomains");
  }
  printf("WAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n\n");
  fflush(stdout);

  std::vector<std::unique_ptr<Function>> result_array;
  std::size_t codom_dim = _f1.get_codom_dim();
  std::pair<double, double> domain = _f1.get_domain();

  if(_f1.get_type()==FunctionExpression::SUM){
  for (const std::unique_ptr<Function> &f : _f1.function_array_) {
      result_array.push_back(f->clone());
}
  }
  result_array.push_back(_f2.clone());


  return FunctionExpression(domain, codom_dim, FunctionExpression::Type::SUM,
                            std::move(result_array));
}


FunctionExpression operator+(const Function &_f1, const Function &_f2) {

  if (not FunctionBase::same_domain(_f1, _f2)) {
    throw std::invalid_argument("Functions with different domains");
  }
  if (not FunctionBase::same_codomain(_f1, _f2)) {
    throw std::invalid_argument("Functions with different codomains");
  }

  std::vector<std::unique_ptr<Function>> result_array;
  std::size_t codom_dim = _f1.get_codom_dim();
  std::pair<double, double> domain = _f1.get_domain();

  printf("call clone 1\n");
  result_array.push_back(_f1.clone());
  result_array.push_back(_f2.clone());
  printf("call clone 1\n");
fflush(stdout);

  return FunctionExpression(domain, codom_dim, FunctionExpression::Type::SUM,
                            std::move(result_array));
}

FunctionExpression operator+(FunctionExpression &&_f1, const Function &_f2) {

printf("******************\n");
fflush(stdout);
  if (not FunctionBase::same_domain(_f1, _f2)) {
    throw std::invalid_argument("Functions with different domains");
  }
  if (not FunctionBase::same_codomain(_f1, _f2)) {
    throw std::invalid_argument("Functions with different codomains");
  }

  std::vector<std::unique_ptr<Function>> result_array;
  std::size_t codom_dim = _f1.get_codom_dim();
  std::pair<double, double> domain = _f1.get_domain();


  if(_f1.get_type()==FunctionExpression::SUM){
      printf("simple clone .. \n");
      _f1.function_array_.push_back(_f2.clone());
      return std::move(_f1);
  }
      printf("clone 2 \n");
  result_array.push_back(std::make_unique<FunctionExpression>(std::move(_f1)));
  result_array.push_back(_f2.clone());
      printf("clone 2 \n");

fflush(stdout);

  return FunctionExpression(domain, codom_dim, FunctionExpression::Type::SUM,
                            std::move(result_array));
}


/* -----
 *  Function Evaluation
 * -----*/
Eigen::MatrixXd
eval_sum_functions(std::vector<std::unique_ptr<Function>> &_function_array,
                   const Eigen::Ref<const Eigen::VectorXd> _domain_points) {

  Eigen::MatrixXd result(_domain_points.size(),
                         _function_array[0]->get_codom_dim());
  result.setZero();
  for (std::unique_ptr<Function> &f : _function_array) {
    result += f->value(_domain_points);
  }
  return result;
}


/* -----
 *  Function Derivation
 * -----*/
std::unique_ptr<Function>
deriv_sum_functions(std::vector<std::unique_ptr<Function>> &_function_array,
                    std::size_t _deg) {

  std::vector<std::unique_ptr<Function>> result_array;
  for (std::unique_ptr<Function> &f : _function_array) {
    result_array.push_back(f->deriv(_deg));
  }
  std::size_t codom_dim = _function_array[0]->get_codom_dim();
  std::pair<double, double> domain = _function_array[0]->get_domain();
  return std::make_unique<FunctionExpression>(
      domain, codom_dim, FunctionExpression::Type::SUM, result_array);
}

} // namespace functions
} // namespace gsplines
