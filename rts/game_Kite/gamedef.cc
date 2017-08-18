/**
* Copyright (c) 2017-present, Facebook, Inc.
* All rights reserved.
*
* This source code is licensed under the BSD-style license found in the
* LICENSE file in the root directory of this source tree. An additional grant
* of patent rights can be found in the PATENTS file in the same directory.
*/

#include "../engine/gamedef.h"
#include "../engine/game_env.h"
#include "../engine/rule_actor.h"

#include "../engine/cmd.gen.h"
#include "../engine/cmd_specific.gen.h"
#include "cmd_specific.gen.h"

int GameDef::GetNumUnitType() {
    return NUM_KITE_UNITTYPE;
}

int GameDef::GetNumAction() {
  return NUM_KITESTATE;
}

bool GameDef::IsUnitTypeBuilding(UnitType) const{
    return false;
}

bool GameDef::HasBase() const{ return false; }

bool GameDef::CheckAddUnit(RTSMap *_map, UnitType, const PointF& p) const{
    return _map->CanPass(p, INVALID);
}

void GameDef::InitUnits() {
    _units.assign(GetNumUnitType(), UnitTemplate());
    _units[KMELEE_ATTACKER] = _C(100, 100, 1, 0.1, 15, 1, 3, vector<int>{0, 15, 0, 0}, vector<CmdType>{MOVE, ATTACK});
    _units[KRANGE_ATTACKER] = _C(100, 50, 0, 0.2, 10, 5, 5, vector<int>{0, 10, 0, 0}, vector<CmdType>{MOVE, ATTACK});
    reg_engine();
    reg_engine_specific();
    reg_minirts_specific();
}

vector<pair<CmdBPtr, int> > GameDef::GetInitCmds(const RTSGameOptions&) const{
      vector<pair<CmdBPtr, int> > init_cmds;
      init_cmds.push_back(make_pair(CmdBPtr(new CmdGenerateMap(INVALID, 0, 200)), 1));
      init_cmds.push_back(make_pair(CmdBPtr(new CmdGameStart(INVALID)), 2));
      init_cmds.push_back(make_pair(CmdBPtr(new CmdGenerateUnit(INVALID)), 3));
      return init_cmds;
}

PlayerId GameDef::CheckWinner(const GameEnv& env, bool /*exceeds_max_tick*/) const {
    return env.CheckLastPlayerHasUnit();
}

void GameDef::CmdOnDeadUnitImpl(GameEnv* env, CmdReceiver* receiver, UnitId /*_id*/, UnitId _target) const{
    Unit *target = env->GetUnit(_target);
    if (target == nullptr) return;
    receiver->SendCmd(CmdIPtr(new CmdRemove(_target)));
}

/*
bool GameDef::ActByStateFunc(RuleActor rule_actor, const GameEnv& env, const vector<int>& state, string *s, AssignedCmds *cmds) const {
    return rule_actor.ActByState(env, state, s, cmds);
}
*/
