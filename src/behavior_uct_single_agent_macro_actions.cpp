// Copyright (c) 2019 fortiss GmbH
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "src/behavior_uct_single_agent_macro_actions.hpp"

#include "modules/models/behavior/idm/idm_classic.hpp"
#include "modules/models/behavior/motion_primitives/macro_actions.hpp"
#include "modules/models/dynamic/single_track.hpp"

namespace modules {
namespace models {
namespace behavior {

using modules::models::behavior::primitives::Primitive;
using modules::models::behavior::primitives::PrimitiveConstAccChangeToLeft;
using modules::models::behavior::primitives::PrimitiveConstAccChangeToRight;
using modules::models::behavior::primitives::PrimitiveConstAccStayLane;
using modules::models::dynamic::Input;
using modules::models::dynamic::SingleTrackModel;
using modules::world::prediction::PredictionSettings;

PredictionSettings BehaviorUCTSingleAgentMacroActions::SetupPredictionSettings(
    const commons::ParamsPtr& params) {
  // Setup prediction models for ego agent and other agents
  auto prediction_params_ego = params->AddChild("EgoVehicle");
  DynamicModelPtr dyn_model(new SingleTrackModel(prediction_params_ego));
  BehaviorModelPtr ego_prediction_model(
      new BehaviorMPMacroActions(dyn_model, prediction_params_ego));

  float cte = params->GetReal("BehaviorUctSingleAgent::CrossTrackError",
                              "Parameter for lat control", 1);
  std::vector<float> acc_vec =
      params->GetListFloat("BehaviorUctSingleAgent::AccelerationInputs",
                           "A list of acceleration ", {0, 1, 4, -1, -8});

  std::vector<std::shared_ptr<Primitive>> prim_vec;

  for (auto& acc : acc_vec) {
    auto primitive = std::make_shared<PrimitiveConstAccStayLane>(
        prediction_params_ego, dyn_model, acc, cte);
    prim_vec.push_back(primitive);
  }

  auto primitive_left = std::make_shared<PrimitiveConstAccChangeToLeft>(
      prediction_params_ego, dyn_model, cte);
  prim_vec.push_back(primitive_left);

  auto primitive_right = std::make_shared<PrimitiveConstAccChangeToRight>(
      prediction_params_ego, dyn_model, cte);
  prim_vec.push_back(primitive_right);

  for (auto& p : prim_vec) {
    auto idx =
        std::dynamic_pointer_cast<BehaviorMPMacroActions>(ego_prediction_model)
            ->AddMotionPrimitive(p);
  }
  auto prediction_params_other = params->AddChild("OtherVehicles");
  BehaviorModelPtr others_prediction_model(
      new BehaviorIDMClassic(prediction_params_other));
  return PredictionSettings(ego_prediction_model, others_prediction_model);
}

}  // namespace behavior
}  // namespace models
}  // namespace modules
