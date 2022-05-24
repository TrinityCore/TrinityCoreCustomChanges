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

#include "AntiCheatMgr.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "Log.h"
#include "Map.h"
#include "Realm.h"
#include "WorldSession.h"

AntiCheatMgr::AntiCheatMgr(Player* player)
{
    _player = player;
}

AntiCheatMgr::~AntiCheatMgr() { }

void AntiCheatMgr::HackDetection(MovementInfo movementInfo, uint32 opcode)
{
    if (!sWorld->getBoolConfig(CONFIG_ANTICHEAT_ENABLED))
        return;

    // GM Commands persists when you turn of GM State, we check for Account RBAC
    if (_player->CanBeGameMaster())
        return;

    // We use this variable to determine if we should look for hacks or not
    // This makes the code more readably since we have to run through the whole function to store the current info for the next check at the end of the function
    bool checkForHacks = true;

    // We calculate the coordinate changes one by one to later only check for the hacks that are actually affected
    // For example we dont need to check for SpeedHack if only Z coordinate changes
    _deltaX = fabs(_lastMovementInfo.pos.m_positionX - movementInfo.pos.m_positionX);
    _deltaY = fabs(_lastMovementInfo.pos.m_positionY - movementInfo.pos.m_positionY);
    _deltaZ = fabs(_lastMovementInfo.pos.m_positionZ - movementInfo.pos.m_positionZ);

    // If the player hasnt changed his position we dont have to check for hacks
    if (_deltaX == 0 && _deltaY == 0 && _deltaZ == 0)
        checkForHacks = false;

    // If LastMovementInfo is all 0 the player just logged in and we dont need to check for hacks yet
    if (_lastMovementInfo.pos.m_positionX == 0 &&
        _lastMovementInfo.pos.m_positionY == 0 &&
        _lastMovementInfo.pos.m_positionZ == 0)
        checkForHacks = false;

    // If the player is using a transport, taxi or vehicle, don't check for hacks
    if (_player->IsInFlight() || _player->GetTransport() || _player->GetVehicle())
        checkForHacks = false;

    // If the player is on our transport list (Deeprun, Ships...), don't check for hacks
    if (CheckForTransport())
        checkForHacks = false;

    if (sWorld->getBoolConfig(CONFIG_ANTICHEAT_ANTIKNOCKBACK_HACK))
        CheckForAntiKnockbackHack(movementInfo);

    // If the movement is allowed by the core (Taxi, Charge, Legit Teleports...), don't check for hacks
    if (_allowedMovement)
        checkForHacks = false;

    if (checkForHacks)
    {
        // We only need to check for certain hacks if the player has moved on x or y axis. If the player moved 50 units or more the teleport hack takes over
        if (_deltaX + _deltaY > 0 && _deltaX + _deltaY < 50.0f)
        {
            if (sWorld->getBoolConfig(CONFIG_ANTICHEAT_SPEED_HACK))
                CheckForSpeedHack(movementInfo);

            if (sWorld->getBoolConfig(CONFIG_ANTICHEAT_IGNORECONTROL_HACK))
                CheckForIgnoreControlHack();

            // Only need to check for ClimbHack if the Z Coordinate changed
            if (sWorld->getBoolConfig(CONFIG_ANTICHEAT_CLIMB_HACK))
                if (_deltaZ > 0)
                    CheckForClimbHack(movementInfo, opcode);

            // We want to check for Water Walk if the player has LiquidStatus Waterwalk
            // Otherwise, we want to check for ZAxis Hack
            if (_player->GetLiquidStatus() == LIQUID_MAP_WATER_WALK)
            {
                if (sWorld->getBoolConfig(CONFIG_ANTICHEAT_WATERWALK_HACK))
                    CheckForWaterwalkHack();
            }
            else
            {
                if (sWorld->getBoolConfig(CONFIG_ANTICHEAT_ZAXIS_HACK))
                    CheckForZAxisHack(movementInfo);

                if (sWorld->getBoolConfig(CONFIG_ANTICHEAT_ANTISWIM_HACK))
                    CheckForAntiswimHack(movementInfo, opcode);
            }
        }

        // Only check for Teleport Hack if the player moved 50 units or more
        if (_deltaX + _deltaY >= 50.0f)
            if (sWorld->getBoolConfig(CONFIG_ANTICHEAT_TELEPORT_HACK))
                CheckForTeleportHack();

        // Only need to check for JumpHack if the player actually jumps
        if (sWorld->getBoolConfig(CONFIG_ANTICHEAT_JUMP_HACK))
            CheckForJumpHack(opcode);

        if (sWorld->getBoolConfig(CONFIG_ANTICHEAT_GRAVITY_HACK))
            CheckForGravityHack(movementInfo);

        // Only need to check for FlyHack if the player is actually flying
        // With our tested Cheat Tool we need to check for Ascending/Descending since MOVEMENTFLAG_FLYING is not triggered
        if (sWorld->getBoolConfig(CONFIG_ANTICHEAT_FLY_HACK))
            if (movementInfo.HasMovementFlag(MOVEMENTFLAG_FLYING) ||
                movementInfo.HasMovementFlag(MOVEMENTFLAG_CAN_FLY) ||
                movementInfo.HasMovementFlag(MOVEMENTFLAG_ASCENDING) ||
                movementInfo.HasMovementFlag(MOVEMENTFLAG_DESCENDING))
                CheckForFlyHack();
    }

    // To decrease the amount of false positives we get, we allow a certain number of positive detections (_hackTolerance)
    // in a certain mount of time (_timeDiff)
    _timeDiff += getMSTimeDiff(_lastMovementInfo.time, movementInfo.time);

    if (_timeDiff >= _resetTimer)
    {
        _hackCounter = 0;
        _timeDiff = 0;
    }

    // Now we store the movement info, opcode and liqui status in case we need them for the next check
    _lastMovementInfo = movementInfo;
    _lastOpcode = opcode;
    _lastLiquidStatus = _player->GetLiquidStatus();

    // We also reset the allowed movement for legit teleports etc.
    _allowedMovement = false;
}

// SpeedHack Detection
// Compare Distance between positions and the time it took to travel it
// False Positive: Dying and teleporting to Ghost healer
void AntiCheatMgr::CheckForSpeedHack(MovementInfo movementInfo)
{
    // To prevent false positives when entering water we skip detection if the liquid status changed recently
    if (_player->GetLiquidStatus() != _lastLiquidStatus)
        return;

    // We need to determine how the player is moving
    uint8 movementFlag = 0;

    if (movementInfo.HasMovementFlag(MOVEMENTFLAG_SWIMMING))
        movementFlag = MOVE_SWIM;
    else if (movementInfo.HasMovementFlag(MOVEMENTFLAG_FLYING))
        movementFlag = MOVE_FLIGHT;
    else if (movementInfo.HasMovementFlag(MOVEMENTFLAG_WALKING))
        movementFlag = MOVE_WALK;
    else
        movementFlag = MOVE_RUN;

    // Now we can calculate the maximum allowed travel speed depending on the movement type
    // We increase the allowed speed by a tolerance to minimize false positives (Default +7.5%)
    float const tolerance = sWorld->getFloatConfig(CONFIG_ANTICHEAT_TOLERANCE);
    float allowedSpeedRate = _player->GetSpeed(UnitMoveType(movementFlag)) * tolerance;

    // If the allowed speedrate was just reduced, we want to skip detection to prevent a false positive
    if (allowedSpeedRate < _lastAllowedSpeedRate)
    {
        _lastAllowedSpeedRate = allowedSpeedRate;
        return;
    }

    // We calculate the client speedrate accordint to his movementInfos
    float clientSpeedRate = CalculateSpeedRate(movementInfo, _lastMovementInfo);

    // We check the last MovementInfo for the falling flag since falling down a hill and sliding a bit triggered a false positive
    if ((clientSpeedRate > allowedSpeedRate) && !(_lastMovementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING) || movementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING)))
    {
        _hackCounter++;

        if (_hackCounter >= _hackTolerance)
            CreateCheatReport("SpeedHack", "Speed Difference: " + std::to_string(clientSpeedRate - allowedSpeedRate));
    }

    _lastAllowedSpeedRate = allowedSpeedRate;

    return;
}

// FlyHack Detection
// Detection relies on the player having the movement flag MOVEMENTFLAG_ASCENDING/DESCENDING since the flying flags dont trigger from the cheat
void AntiCheatMgr::CheckForFlyHack()
{
    // Is the player allowed to fly because of an aura?
    if (_player->HasAuraType(SPELL_AURA_FLY) ||
        _player->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) ||
        _player->HasAuraType(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED))
        return;

    // Ascending/Descending flag is okay if the player is swimming
    if (_player->IsInWater())
        return;

    _hackCounter++;

    CreateCheatReport("FlyHack");

    return;
}

// TeleportHack Detection
void AntiCheatMgr::CheckForTeleportHack()
{
    // Since teleport hack is not used repeatedly, we ignore _hackCounter and immediately create a report
    if (!_allowedMovement)
        CreateCheatReport("TeleportHack");

    _hackCounter++;

    return;
}

// JumpHack Detection
void AntiCheatMgr::CheckForJumpHack(uint32 opcode)
{
    if (opcode != MSG_MOVE_JUMP)
        return;

    // If player sends the JUMP opcode twice in a row we know there is something fishy going on
    if (_lastOpcode == MSG_MOVE_JUMP && opcode == MSG_MOVE_JUMP)
        CreateCheatReport("JumpHack");

    _hackCounter++;

    return;
}

// GravityHack
// We check if the player has an abnormal jump speed
void AntiCheatMgr::CheckForGravityHack(MovementInfo movementInfo)
{
    // We only want to check if the player jumped to not cause false positives on knockbacks
    if (_lastOpcode != MSG_MOVE_JUMP)
        return;

    // If player has a slow fall aura the gravity manipulation is legit
    if (_player->HasAuraType(SPELL_AURA_FEATHER_FALL))
        return;

    if (!_player->HasUnitState(MOVEMENTFLAG_DISABLE_GRAVITY) && movementInfo.jump.zspeed < -10.0f)
        CreateCheatReport("GravityHack", "Jumpspeed: " + std::to_string(movementInfo.jump.zspeed));
}

// Climbhack Detection
// We check if the slope of the players movement is steeper than the server allows
void AntiCheatMgr::CheckForClimbHack(MovementInfo movementInfo, uint32 opcode)
{
    // These flags already get checked in other cheat detection functions so we can skip these
    // We check if the player jumped while in water to catch him using low gravity hack there
    if ((_player->IsInWater() && opcode != MSG_MOVE_JUMP) ||
        _player->IsFlying() ||
        _player->IsFalling())
        return;

    // If the player jumped, we dont want to check for climb hack
    // This can lead to false positives for climbing game objects legit
    if (opcode == MSG_MOVE_JUMP)
        return;

    // To further reduce false positives, we only care for movements where the z coordinate changed by more than 2 units
    if (_deltaZ <= 2.0f)
        return;

    // In Maps we have walkableSlopeAngleNotSteep at <= 55 degrees
    // Our calculation is in radians and 1 radian = 57.296 degrees
    // So 55 degrees are 0.9599 radians (Default)
    float const maxAngle = sWorld->getFloatConfig(CONFIG_ANTICHEAT_MAXSLOPE);

    // Calculates the slope of the movement
    float slope = CalculateSlope(movementInfo, _lastMovementInfo);

    // If the slope of the players movement is bigger than the maximum slope allowed and he is not falling he must be hacking
    if ((slope > maxAngle) &&
        (!_lastMovementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING) || !movementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING)))
    {
        _hackCounter++;

        if (movementInfo.pos.GetPositionZ() < _lastMovementInfo.pos.GetPositionZ() && _deltaZ > 14.57f)
            // If the player moved more than 14.57f DOWNWARDS and doesn't have MOVEMENTFLAG_FALLING he probably has a NoFallDamage Hack
            CreateCheatReport("ClimbHack", "Possible No Fall Damage Hack - Slope: " + std::to_string(slope) + " DeltaZ: " + std::to_string(_deltaZ));
        else
            CreateCheatReport("ClimbHack", "Slope: " + std::to_string(slope) + " DeltaZ: " + std::to_string(_deltaZ));
    }

    return;
}

// ZAxisHack Detection
// We compare the Z Coordinate of the map to the Z Coordinate of the player. If he is not allowed to fly he must be hacking if he is not on the ground
// Also kind of cateches some WaterwalkHacking
void AntiCheatMgr::CheckForZAxisHack(MovementInfo movementInfo)
{
    // If he is flying we dont need to check
    if (movementInfo.HasMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FLYING))
        return;

    // If the player is allowed to waterwalk (or he is dead because he automatically waterwalks then) we dont need to check any further
    // We also stop if the player is in water, because otherwise you get a false positive for swimming
    if (movementInfo.HasMovementFlag(MOVEMENTFLAG_WATERWALKING) || _player->IsInWater() || !_player->IsAlive())
        return;

    // We want to exclude this LiquidStatus from detection because it leads to false positives on boats, docks etc.
    // Basically everytime you stand on a game object in water
    if (_player->GetLiquidStatus() == LIQUID_MAP_ABOVE_WATER)
        return;

    // If the Z coordinate hasnt changed and the player is higher than the map ground position plus tolerance, he must be hacking
    if (_lastMovementInfo.pos.GetPositionZ() == movementInfo.pos.GetPositionZ()
        && _player->GetPositionZ() >= _player->GetFloorZ() + 2.5f)
    {
        _hackCounter++;

        if (_hackCounter >= _hackTolerance)
            CreateCheatReport("ZAxisHack", "LiquidStatus: " + std::to_string(_player->GetLiquidStatus()));
    }

    return;
}

// WaterwalkHack Detection
// We use the LiquidState because our test hack cant seem to fake this
void AntiCheatMgr::CheckForWaterwalkHack()
{
    // if we are a ghost we can walk on water
    if (!_player->IsAlive())
        return;

    // It counts as Liquid Status Waterwalking if youre flying directly over the water
    if (_player->IsFlying())
        return;

    // If the liquid status changed recently we skip the detection
    if (_player->GetLiquidStatus() != _lastLiquidStatus)
        return;

    // Player has an aura that allows him to water walk
    if (_player->HasAuraType(SPELL_AURA_WATER_WALK) ||
        _player->HasAuraType(SPELL_AURA_FEATHER_FALL) ||
        _player->HasAuraType(SPELL_AURA_SAFE_FALL))
        return;

    _hackCounter++;

    if (_hackCounter >= _hackTolerance)
        CreateCheatReport("WaterwalkHack");

    return;
}

// Anti-Swim Hack Detection
// If the player is underwater and not swimming, he probably has a anti-swim hack
void AntiCheatMgr::CheckForAntiswimHack(MovementInfo movementInfo, uint32 opcode)
{
    // We only need to check for this hack if the player is under water
    if (_player->GetLiquidStatus() != LIQUID_MAP_UNDER_WATER)
        return;

    // If the player is under water and swimming its fine
    // We also check if he has the falling movement flag to prevent false positives while jumping in water
    if (movementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_SWIMMING))
        return;

    // Again we want to exclude false positives from jumping in water
    if (opcode == MSG_MOVE_JUMP)
        return;

    // If the liquid status changed recently we skip the detection
    if (_player->GetLiquidStatus() != _lastLiquidStatus)
        return;

    CreateCheatReport("AntiswimHack");
}

// IgnoreControlHack Detection
// Cant be tested atm since we lack a cheat tool that can enable this
void AntiCheatMgr::CheckForIgnoreControlHack()
{
    // Players with high latency can trigger a false positive here when the aura durations dont match up because of the latency
    if (_player->GetSession()->GetLatency() >= 200)
        return;

    // If the player is rooted and moved anyway, if he is not in a Vehicle he is cheating
    if (_player->HasUnitState(UNIT_STATE_ROOT) && !_player->GetVehicle())
    {
        _hackCounter++;

        if (_hackCounter >= _hackTolerance)
            CreateCheatReport("IgnoreControlHack", "Latency: " + std::to_string(_player->GetSession()->GetLatency()));
    }

    return;
}

// Anti-Knockback Detection
// We set _knockback to true when the core sends a knockback opcode
// If the clients next movement originates from the same point he was knocked back from we can assume he ignored the knockback with a hack
void AntiCheatMgr::CheckForAntiKnockbackHack(MovementInfo movementInfo)
{
    if (!_knockback)
        return;

    if (movementInfo.pos == _lastMovementInfo.pos)
        CreateCheatReport("KnockbackHack");

    _knockback = false;
}

// Allowed Movement means while the movement might trigger hack detection, it is legit and we can skip the hack detection
void AntiCheatMgr::SetAllowedMovement(bool allowed)
{
    _allowedMovement = allowed;
}

void AntiCheatMgr::SetKnockback(bool value)
{
    _knockback = value;
}

// Calculates the speed rate of the player based on the distance between his last movements and the time passed
float AntiCheatMgr::CalculateSpeedRate(MovementInfo movementInfo, MovementInfo lastMovementInfo)
{
    // Distance between current and last position
    float deltaXY = movementInfo.pos.GetExactDist2d(lastMovementInfo.pos);

    // Time passed between current and last position
    uint32 timeDiff = getMSTimeDiff(lastMovementInfo.time, movementInfo.time);

    // To prevent divisions by 0 and other fuckery with the timeDiff, we force it to be at least 1
    if (int32(timeDiff) < 1)
        timeDiff = 1;

    // Here we calculate the actual speed rate of the player
    return deltaXY * 1000 / timeDiff;
}

// Calculates the slope of the players climb
float AntiCheatMgr::CalculateSlope(MovementInfo movementInfo, MovementInfo lastMovementInfo)
{
    // We calculate the distance the player moved exactly like in the speed hack detection
    float deltaXY = movementInfo.pos.GetExactDist2d(lastMovementInfo.pos);
    float deltaZ = fabs(movementInfo.pos.GetPositionZ() - lastMovementInfo.pos.GetPositionZ());

    // We don't want to divide by something smaller than 1
    // For example 2 / 0.2 = 10 -> A lot of false positives
    if (deltaXY < 1)
        deltaXY = 1;

    // With that information we can calculate the slope of the players movement
    return atan(deltaZ / deltaXY);
}

bool AntiCheatMgr::CheckForTransport()
{
    // On transports, the player may move way faster than allowed. We check for the specific MapIds so a hack cant fake the MovementFlag
    if (_lastMovementInfo.HasMovementFlag(MOVEMENTFLAG_ONTRANSPORT) && _player->GetMapId())
    {
        switch (_player->GetMapId())
        {
            case 369: // Transport: Deeprun Tram
            case 607: // Transport: Strands of the Ancients
            case 582: // Transport: Rut'theran to Auberdine
            case 584: // Transport: Menethil to Theramore
            case 586: // Transport: Exodar to Auberdine
            case 587: // Transport: Feathermoon Ferry
            case 588: // Transport: Menethil to Auberdine
            case 589: // Transport: Orgrimmar to Grom'Gol
            case 590: // Transport: Grom'Gol to Undercity
            case 591: // Transport: Undercity to Orgrimmar
            case 592: // Transport: Borean Tundra Test
            case 593: // Transport: Booty Bay to Ratchet
            case 594: // Transport: Howling Fjord Sister Mercy (Quest)
            case 596: // Transport: Naglfar
            case 610: // Transport: Tirisfal to Vengeance Landing
            case 612: // Transport: Menethil to Valgarde
            case 613: // Transport: Orgrimmar to Warsong Hold
            case 614: // Transport: Stormwind to Valiance Keep
            case 620: // Transport: Moa'ki to Unu'pe
            case 621: // Transport: Moa'ki to Kamagua
            case 622: // Transport: Orgrim's Hammer
            case 623: // Transport: The Skybreaker
            case 641: // Transport: Alliance Airship BG
            case 642: // Transport: Horde Airship BG
            case 647: // Transport: Orgrimmar to Thunder Bluff
            case 672: // Transport: The Skybreaker (Icecrown Citadel Raid)
            case 673: // Transport: Orgrim's Hammer (Icecrown Citadel Raid)
            case 712: // Transport: The Skybreaker (Icecrown Citadel Dungeon)
            case 713: // Transport: Orgrim's Hammer (Icecrown Citadel Dungeon)
            case 718: // Transport: The Mighty Wind (Icecrown Citadel Raid)
                return true;
        }
    }

    return false;
}

void AntiCheatMgr::CreateCheatReport(std::string hacktype)
{
    CreateCheatReport(hacktype, "");
}

void AntiCheatMgr::CreateCheatReport(std::string hacktype, std::string description)
{
    std::string playerName = _player->GetName();
    uint32 accountId = _player->GetSession()->GetAccountId();
    std::string accountName = _player->GetSession()->GetAccountName();
    uint32 playerGUID = _player->GetGUID();
    std::string playerClass = (std::string)GetClassName(_player->GetClass(), 0);
    std::string reportLocation = _player->GetMapAreaAndZoneString();

    // If the player is mindcontrolled, we want to log the controlling player instead of the controlled player
    if (Player* controller = _player->GetControllingPlayer())
        if (controller != _player)
        {
            playerName = controller->GetName();
            accountId = controller->GetSession()->GetAccountId();
            accountName = controller->GetSession()->GetAccountName();
            playerGUID = controller->GetGUID();
            playerClass = (std::string)GetClassName(controller->GetClass(), 0);
            reportLocation = controller->GetMapAreaAndZoneString();
        }

    // We want to store a copy-pastable .go xyz command to be able to check out the position the hack was detected ingame
    // We add 1.0f to the Z Coordinate to prevent clipping through the map
    std::string goXYZ = ".go xyz " + std::to_string(_player->GetPositionX()) + " " + std::to_string(_player->GetPositionY()) + " " + std::to_string(_player->GetPositionZ() + 1.0f) + " " + std::to_string(_player->GetMap()->GetId()) + " " + std::to_string(_player->GetOrientation());

    // A message in the worldserver console for logging
    TC_LOG_INFO("anticheat", "[AntiCheat] Possible %s detected: Player %s at %s - Description: %s", hacktype.c_str(), playerName.c_str(), reportLocation.c_str(), description.c_str());

    // Here we build our SQL Query to insert the cheat report into the table
    // Columns: (id), (timestamp), Account ID, Playername, Description, Area information, go. xyz Command
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_ACCOUNT_ANTICHEAT);

    stmt->setUInt32(0, realm.Id.Realm); // realm_id
    stmt->setUInt32(1, accountId); // account_id
    stmt->setString(2, accountName); // account_name
    stmt->setUInt32(3, playerGUID); // character_guid
    stmt->setString(4, playerName); // character_name
    stmt->setString(5, playerClass); // character_class
    stmt->setString(6, hacktype); // detected_hack
    stmt->setString(7, description);
    stmt->setString(8, _player->GetMapAreaAndZoneString()); // map_info
    stmt->setString(9, goXYZ); // gm_teleport

    LoginDatabase.Execute(stmt);
}