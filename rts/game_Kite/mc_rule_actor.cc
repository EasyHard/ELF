/**
* Copyright (c) 2017-present, Facebook, Inc.
* All rights reserved.
*
* This source code is licensed under the BSD-style license found in the
* LICENSE file in the root directory of this source tree. An additional grant
* of patent rights can be found in the PATENTS file in the same directory.
*/

#include "mc_rule_actor.h"

bool MCRuleActor::ActByState2(const GameEnv &env, const vector<int>& state, string *state_string, AssignedCmds *assigned_cmds) {
    assigned_cmds->clear();
    *state_string = "";

    RegionHist hist;

    // Then loop over all my troops to run.
    const auto& all_my_troops = _preload.AllMyTroops();
    for (const Unit *u : all_my_troops) {
        // Get the bin id.
        act_per_unit(env, u, &state[0], &hist, state_string, assigned_cmds);
    }

    return true;
}

bool MCRuleActor::ActByState(const GameEnv &env, const vector<int>& state, string *state_string, AssignedCmds *assigned_cmds) {
    // Each unit can only have one command. So we have this map.
    // cout << "Enter ActByState" << endl << flush;
    assigned_cmds->clear();
    *state_string = "NOOP";
    const auto& my_troops = _preload.MyTroops();

    const auto& enemy_troops = _preload.EnemyTroops();
    const auto& enemy_troops_in_range = _preload.EnemyTroopsInRange();
    const auto& all_my_troops = _preload.AllMyTroops();

    if (state[KITESTATE_ATTACK_IN_RANGE]) {
      *state_string = "Attack enemy in range..NOOP";
      if (! enemy_troops_in_range.empty()) {
        *state_string = "Attack enemy in range..Success";
        auto cmd = _A(enemy_troops_in_range[0]->GetId());
        batch_store_cmds(my_troops[KMELEE_ATTACKER], cmd, assigned_cmds);
        batch_store_cmds(my_troops[KRANGE_ATTACKER], cmd, assigned_cmds);
      }
    }
	
    if (state[KITESTATE_ATTACK]) {
        *state_string = "Attack..Normal";
        // Then let's scout out the map.        
		if (! enemy_troops.empty())
		{
	        auto cmd = _A(enemy_troops[0]->GetId());
	        batch_store_cmds(my_troops[KMELEE_ATTACKER], cmd, false, assigned_cmds);
	        batch_store_cmds(my_troops[KRANGE_ATTACKER], cmd, false, assigned_cmds);
		}
    }

    return true;
}

bool MCRuleActor::GetActSimpleState(vector<int>* state) {
    vector<int> &_state = *state;
	_state[KITESTATE_ATTACK_IN_RANGE] = 1;
	_state[KITESTATE_ATTACK] = 1;
	
    return true;
}

bool MCRuleActor::ActWithMap(const GameEnv &env, const vector<vector<vector<int>>>& action_map, string *state_string, AssignedCmds *assigned_cmds) {
    assigned_cmds->clear();
    *state_string = "";

    vector<vector<RegionHist>> hist(action_map.size());
    for (size_t i = 0; i < hist.size(); ++i) {
        hist[i].resize(action_map[i].size());
    }

    const int x_size = env.GetMap().GetXSize();
    const int y_size = env.GetMap().GetYSize();
    const int rx = action_map.size();
    const int ry = action_map[0].size();

    // Then loop over all my troops to run.
    const auto& all_my_troops = _preload.AllMyTroops();
    for (const Unit *u : all_my_troops) {
        // Get the bin id.
        const PointF& p = u->GetPointF();
        int x = static_cast<int>(std::round(p.x / x_size * rx));
        int y = static_cast<int>(std::round(p.y / y_size * ry));
        // [REGION_MAX_RANGE_X][REGION_MAX_RANGE_Y][REGION_RANGE_CHANNEL]
        act_per_unit(env, u, &action_map[x][y][0], &hist[x][y], state_string, assigned_cmds);
    }

    return true;
}
