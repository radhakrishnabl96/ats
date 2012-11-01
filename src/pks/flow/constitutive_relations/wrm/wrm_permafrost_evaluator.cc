/* -*-  mode: c++; c-default-style: "google"; indent-tabs-mode: nil -*- */

/*
  This WRM evaluator evaluates saturation of gas, liquid, and ice from the
  constituents, A and B in the permafrost notes.

  Authors: Ethan Coon (ecoon@lanl.gov)
*/

#include "wrm_permafrost_evaluator.hh"


namespace Amanzi {
namespace Flow {
namespace FlowRelations {

#define DEBUG_FLAG 0

WRMPermafrostEvaluator::WRMPermafrostEvaluator(Teuchos::ParameterList& plist) :
    SecondaryVariablesFieldEvaluator(plist) {

  // my keys are for saturation
  s_l_key_ = plist_.get<string>("liquid saturation key", "saturation_liquid");
  my_keys_.push_back(s_l_key_);
  my_keys_.push_back(plist_.get<string>("ice saturation key", "saturation_ice"));
  my_keys_.push_back(plist_.get<string>("gas saturation key", "saturation_gas"));
  setLinePrefix(my_keys_[0]+std::string(" evaluator"));

  // 1/A is the ice-liquid
  one_on_A_key_ = plist_.get<string>("1/A key", "wrm_permafrost_one_on_A");
  dependencies_.insert(one_on_A_key_);

  // 1/B is the gas-liquid
  one_on_B_key_ = plist_.get<string>("1/B key", "wrm_permafrost_one_on_B");
  dependencies_.insert(one_on_B_key_);
}

WRMPermafrostEvaluator::WRMPermafrostEvaluator(const WRMPermafrostEvaluator& other) :
    SecondaryVariablesFieldEvaluator(other),
    one_on_A_key_(other.one_on_A_key_),
    one_on_B_key_(other.one_on_B_key_),
    s_l_key_(other.s_l_key_) {}

Teuchos::RCP<FieldEvaluator>
WRMPermafrostEvaluator::Clone() const {
  return Teuchos::rcp(new WRMPermafrostEvaluator(*this));
}


void WRMPermafrostEvaluator::EvaluateField_(const Teuchos::Ptr<State>& S,
        const std::vector<Teuchos::Ptr<CompositeVector> >& results) {
  // Loop over names in the target and then owned entities in that name,
  // evaluating the evaluator to calculate saturations.
  for (CompositeVector::name_iterator comp=results[0]->begin();
       comp!=results[0]->end(); ++comp) {
    Epetra_MultiVector& sat = *results[0]->ViewComponent(*comp,false);
    Epetra_MultiVector& sat_i = *results[1]->ViewComponent(*comp,false);
    Epetra_MultiVector& sat_g = *results[2]->ViewComponent(*comp,false);

    const Epetra_MultiVector& one_on_A =
        *S->GetFieldData(one_on_A_key_)->ViewComponent(*comp,false);
    const Epetra_MultiVector& one_on_B =
        *S->GetFieldData(one_on_B_key_)->ViewComponent(*comp,false);

    int count = results[0]->size(*comp, false);
    for (int id=0; id!=count; ++id) {
      double s_l = 1.0 / (1.0/one_on_A[0][id] + 1.0/one_on_B[0][id] - 1.0);
      sat[0][id] = s_l;
      sat_i[0][id] = s_l * (1.0/one_on_A[0][id] - 1.0);
      sat_g[0][id] = s_l * (1.0/one_on_B[0][id] - 1.0);
    }
  }
}


void WRMPermafrostEvaluator::EvaluateFieldPartialDerivative_(const Teuchos::Ptr<State>& S,
        Key wrt_key, const std::vector<Teuchos::Ptr<CompositeVector> > & results) {
  if (wrt_key == one_on_A_key_) {
    for (CompositeVector::name_iterator comp=results[0]->begin();
         comp!=results[0]->end(); ++comp) {
      Epetra_MultiVector& dsat = *results[0]->ViewComponent(*comp,false);
      Epetra_MultiVector& dsat_i = *results[1]->ViewComponent(*comp,false);
      Epetra_MultiVector& dsat_g = *results[2]->ViewComponent(*comp,false);

      const Epetra_MultiVector& sat =
          *S->GetFieldData(s_l_key_)->ViewComponent(*comp,false);
      const Epetra_MultiVector& one_on_A =
          *S->GetFieldData(one_on_A_key_)->ViewComponent(*comp,false);
      const Epetra_MultiVector& one_on_B =
          *S->GetFieldData(one_on_B_key_)->ViewComponent(*comp,false);

      int count = results[0]->size(*comp, false);
      for (int id=0; id!=count; ++id) {
        double Ainv = one_on_A[0][id];
        double A = 1.0 / Ainv;
        double dA = - A * A;
        double B = 1.0 / one_on_B[0][id];
        double sl = sat[0][id];

        dsat[0][id] = (- sl * sl) * dA;
        dsat_i[0][id] = sl*dA + (A - 1.0)*dsat[0][id];
        dsat_g[0][id] = (B - 1.0)*dsat[0][id];
      }
    }
  } else if (wrt_key == one_on_B_key_) {
    for (CompositeVector::name_iterator comp=results[0]->begin();
         comp!=results[0]->end(); ++comp) {
      Epetra_MultiVector& dsat = *results[0]->ViewComponent(*comp,false);
      Epetra_MultiVector& dsat_i = *results[1]->ViewComponent(*comp,false);
      Epetra_MultiVector& dsat_g = *results[2]->ViewComponent(*comp,false);

      const Epetra_MultiVector& sat =
          *S->GetFieldData(s_l_key_)->ViewComponent(*comp,false);
      const Epetra_MultiVector& one_on_A =
          *S->GetFieldData(one_on_A_key_)->ViewComponent(*comp,false);
      const Epetra_MultiVector& one_on_B =
          *S->GetFieldData(one_on_B_key_)->ViewComponent(*comp,false);

      int count = results[0]->size(*comp, false);
      for (int id=0; id!=count; ++id) {
        double Binv = one_on_B[0][id];
        double B = 1.0 / Binv;
        double dB = - B * B;
        double A = 1.0 / one_on_A[0][id];
        double sl = sat[0][id];

        dsat[0][id] = (- sl * sl) * dB;
        dsat_i[0][id] = (A - 1.0)*dsat[0][id];
        dsat_g[0][id] = sl*dB + (B - 1.0)*dsat[0][id];
      }
    }
  } else {
    ASSERT(0);
  }
}


} // namespace
} // namespace
} // namespace



