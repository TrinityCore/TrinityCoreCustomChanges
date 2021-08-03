SET
@Entry = 190000,
@NAME = "Arena 1v1",
@Subname = "TrinityCore",
@IconName = "Speak";

DELETE FROM `world`.`creature_template` WHERE `entry`=@Entry;
INSERT INTO `world`.`creature_template` (`entry`, `modelid1`, `modelid2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `scale`, `rank`, `dmgschool`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `AIName`, `MovementType`, `HoverHeight`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ScriptName`) VALUES
(@Entry, 21249, 0, @Name, @Subname, @IconName, 0, 80, 80, 2, 35, 3, 1, 0, 0, 2000, 0, 1, 0, 7, 138936390, 0, 0, 0, '', 0, 1, 0, 0, 1, 0, 0, 'npc_1v1arena');
