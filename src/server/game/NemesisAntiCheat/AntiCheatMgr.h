/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
Nemesis Anti Cheat
Inspired by the old passive anti cheat (https://github.com/TrinityCore/TrinityCoreCustomChanges/tree/3.3.5-passive_anticheat)

Authors:
https://github.com/sveN295
https://github.com/Jinnaix
With feedback and support from https://github.com/acidmanifesto
*/

#ifndef SC_ACMGR_H
#define SC_ACMGR_H

#include "Common.h"
#include "SharedDefines.h"
#include "ScriptMgr.h"
#include "Chat.h"
#include "Player.h"
#include "World.h"

class Player;

class TC_GAME_API AntiCheatMgr
{
public:
    AntiCheatMgr(Player* player);
    ~AntiCheatMgr();

    void HackDetection(MovementInfo movementInfo, uint32 opcode);
    void SetAllowedMovement(bool allowed);
    void SetKnockback(bool value);

private:
    void CheckForSpeedHack(MovementInfo movementInfo);
    void CheckForFlyHack();
    void CheckForTeleportHack();
    void CheckForJumpHack(uint32 opcode);
    void CheckForGravityHack(MovementInfo movementInfo);
    void CheckForClimbHack(MovementInfo movementInfo, uint32 opcode);
    void CheckForZAxisHack(MovementInfo movementInfo);
    void CheckForWaterwalkHack();
    void CheckForAntiswimHack(MovementInfo movementInfo, uint32 opcode);
    void CheckForIgnoreControlHack();
    void CheckForAntiKnockbackHack(MovementInfo movementInfo);

    bool CheckForTransport();

    float CalculateSpeedRate(MovementInfo movementInfo, MovementInfo lastMovementInfo);
    float CalculateSlope(MovementInfo movementInfo, MovementInfo lastMovementInfo);

    void CreateCheatReport(std::string hacktype, std::string description);
    void CreateCheatReport(std::string hacktype);

    Player* _player;

    uint32 _hackCounter = 0;
    uint32 _timeDiff = 0;
    uint32 const _resetTimer = sWorld->getIntConfig(CONFIG_ANTICHEAT_RESET_TIMER); // How much time in ms is supposed to pass before we reset the hack counter
    uint32 const _hackTolerance = sWorld->getIntConfig(CONFIG_ANTICHEAT_HACK_COUNTER); // How many hack detections before we start reporting

    float _deltaX = .0f;
    float _deltaY = .0f;
    float _deltaZ = .0f;

    bool _allowedMovement = false;
    bool _knockback = false;

    uint32 _lastOpcode = 0;
    MovementInfo _lastMovementInfo;
    ZLiquidStatus _lastLiquidStatus;
    float _lastAllowedSpeedRate = .0f;
};

#endif