// Delegate for heuristic corrections based upon coupled surface/subsurface water.


#ifndef AMANZI_MPC_DELEGATE_WATER_HH_
#define AMANZI_MPC_DELEGATE_WATER_HH_

#include "Teuchos_RCP.hpp"
#include "Teuchos_ParameterList.hpp"

#include "VerboseObject.hh"
#include "TreeVector.hh"
#include "CompositeVector.hh"
#include "State.hh"

namespace Amanzi {

class MPCDelegateWater {

 public:
  MPCDelegateWater(const Teuchos::RCP<Teuchos::ParameterList>& plist,
                   int i_domain, int i_surf);

  void
  set_states(const Teuchos::RCP<const State>& S,
             const Teuchos::RCP<State>& S_inter,
             const Teuchos::RCP<State>& S_next) {
    S_= S;
    S_inter_ = S_inter;
    S_next_ = S_next;
  }


  bool
  ModifyPredictor_Heuristic(double h, const Teuchos::RCP<TreeVector>& u);
  bool
  ModifyPredictor_WaterSpurtDamp(double h, const Teuchos::RCP<TreeVector>& u);

  // bool
  // ModifyCorrection(double h, Teuchos::RCP<const TreeVector> res,
  //         Teuchos::RCP<const TreeVector> u, Teuchos::RCP<TreeVector> du);

  int
  ModifyCorrection_WaterFaceLimiter(double h, Teuchos::RCP<const TreeVector> res,
          Teuchos::RCP<const TreeVector> u, Teuchos::RCP<TreeVector> du);

  double
  ModifyCorrection_WaterSpurtDamp(double h, Teuchos::RCP<const TreeVector> res,
          Teuchos::RCP<const TreeVector> u, Teuchos::RCP<TreeVector> du);

  int
  ModifyCorrection_WaterSpurtCap(double h, Teuchos::RCP<const TreeVector> res,
                                    Teuchos::RCP<const TreeVector> u, Teuchos::RCP<TreeVector> du, double damping);

 protected:
  Teuchos::RCP<Teuchos::ParameterList> plist_;
  Teuchos::RCP<VerboseObject> vo_;
  
  // states
  Teuchos::RCP<State> S_next_;
  Teuchos::RCP<State> S_inter_;
  Teuchos::RCP<const State> S_;

  // predictor fixes
  bool modify_predictor_heuristic_;
  bool modify_predictor_spurt_damping_;
  
  // Preconditioned correction alteration
  // -- control
  bool cap_the_spurt_;
  bool damp_the_spurt_;

  // -- parameters
  double cap_size_;
  double face_limiter_;

  // indices into the TreeVector
  int i_surf_;
  int i_domain_;

};

} // namespace



#endif 
