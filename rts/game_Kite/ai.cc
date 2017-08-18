/**
* Copyright (c) 2017-present, Facebook, Inc.
* All rights reserved.
*
* This source code is licensed under the BSD-style license found in the
* LICENSE file in the root directory of this source tree. An additional grant
* of patent rights can be found in the PATENTS file in the same directory.
*/

#include "ai.h"
#include "../engine/game_env.h"
#include "../engine/unit.h"

void AIBase::save_structured_state(const GameEnv &env, Data *data) const {
    GameState *game = &data->newest();
    game->tick = _receiver->GetTick();
    game->winner = env.GetWinnerId();
    game->terminal = env.GetTermination() ? 1 : 0;
    game->player_id = _player_id;

    const int n_type = env.GetGameDef().GetNumUnitType();
    const int n_additional = 3;
    const int resource_grid = 50;
    const int res_pt = NUM_RES_SLOT;
    const int total_channel = n_type + n_additional;

    const auto &m = env.GetMap();

    // [Channel, width, height]
    const int sz = total_channel * m.GetXSize() * m.GetYSize();
    game->s.resize(sz);
    std::fill(game->s.begin(), game->s.end(), 0.0);

    game->res.resize(env.GetNumOfPlayers() * res_pt);
    std::fill(game->res.begin(), game->res.end(), 0.0);

#define _OFFSET(_c, _x, _y) (((_c) * m.GetYSize() + (_y)) * m.GetXSize() + (_x))
#define _F(_c, _x, _y) game->s[_OFFSET(_c, _x, _y)]

    // Extra data.
    game->ai_start_tick = 0;

    PlayerId visibility_check = _respect_fow ? _player_id : INVALID;

    auto unit_iter = env.GetUnitIterator(visibility_check);

    std::vector<int> quantized_r(env.GetNumOfPlayers(), 0);

    while (! unit_iter.end()) {
        const Unit &u = *unit_iter;
        int x = int(u.GetPointF().x);
        int y = int(u.GetPointF().y);
        float hp_level = u.GetProperty()._hp;
        UnitType t = u.GetUnitType();

        _F(t, x, y) = 1.0;
        _F(n_type, x, y) = u.GetPlayerId() + 1;
        _F(n_type + 1, x, y) = hp_level;
		_F(n_type + 2, x, y) = u.GetProperty().CD(CD_ATTACK).Passed(game->tick) ? 1 : -1;
       }

        ++ unit_iter;
    }

    for (int i = 0; i < env.GetNumOfPlayers(); ++i) {
        // Omit player signal from other player's perspective.
        if (visibility_check != INVALID && visibility_check != i) continue;
        const auto &player = env.GetPlayer(i);
        quantized_r[i] = min(int(player.GetResource() / resource_grid), res_pt - 1);
        game->res[i * res_pt + quantized_r[i]] = 1.0;
    }

    game->last_r = 0.0;
    int winner = env.GetWinnerId();

    if (winner != INVALID) {
        if (winner == _player_id) game->last_r = 1.0;
        else game->last_r = -1.0;
    }
}


bool TrainedAI2::on_act(const GameEnv &env) {
    // Get the current action from the queue.
	int h = 0;
	h = gs.a;
    const GameState& gs = _ai_comm->info().data.newest();
	if (_receiver->GetUseCmdComment()) {
		string s = to_string(h);
		SendComment(s);
	}
	
	return gather_decide(env, [&](const GameEnv &e, string *s, AssignedCmds *assigned_cmds) {
        return _mc_rule_actor.ActWithUnit(e, h, s, assigned_cmds);
    });
}

///////////////////////////// Simple AI ////////////////////////////////
bool SimpleAI::on_act(const GameEnv &env) {
    return gather_decide(env, [&](const GameEnv &e, string *s, AssignedCmds *assigned_cmds) {
        return _mc_rule_actor.ActSimpleAI(e, s, assigned_cmds);
    });
}

bool HitAndRunAI::on_act(const GameEnv &env) {
    _state.resize(NUM_AISTATE, 0);
    std::fill (_state.begin(), _state.end(), 0);
    return gather_decide(env, [&](const GameEnv &e, string *s, AssignedCmds *assigned_cmds) {
        _mc_rule_actor.GetActHitAndRunState(&_state);
        return _mc_rule_actor.ActByState(e, _state, s, assigned_cmds);
    });
}
