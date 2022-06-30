-- Basistabelle fuer Anticheat System
DROP TABLE IF EXISTS `nemesis_anticheat`;
CREATE TABLE `nemesis_anticheat` (
    `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT 'Itendifier',
    `time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'Detection Time',
    `realm_id` INT(10) NOT NULL COMMENT 'Realm ID',
    `account_id` INT(10) NOT NULL COMMENT 'Account ID',
    `account_name` VARCHAR(32) NOT NULL COMMENT 'Account Name' COLLATE 'utf8_general_ci',
    `character_guid` INT(10) NOT NULL COMMENT 'Character GUID',
    `character_name` VARCHAR(12) NOT NULL COMMENT 'Character Name' COLLATE 'utf8_general_ci',
    `character_class` VARCHAR(20) NOT NULL COMMENT 'Character Class' COLLATE 'utf8_general_ci',
    `detected_hack` VARCHAR(50) NOT NULL COMMENT 'Hack Type' COLLATE 'utf8_general_ci',
    `detected_value` VARCHAR(255) NOT NULL COMMENT 'Detailed Data for Hacktype' COLLATE 'utf8_general_ci',
    `map_info` VARCHAR(255) NOT NULL COMMENT 'Location Info' COLLATE 'utf8_general_ci',
    `gm_teleport` VARCHAR(255) NOT NULL COMMENT 'Teleport Data' COLLATE 'utf8_general_ci',
    PRIMARY KEY (`id`) USING BTREE
)
COMMENT='Overview of all triggered anti cheat data'
COLLATE='utf8_general_ci'
ENGINE=InnoDB
AUTO_INCREMENT = 0
;

-- Cheat Report View
DROP VIEW IF EXISTS `nemesis_anticheat_report`;
CREATE VIEW nemesis_anticheat_report (account_name, account_id, c_speedhack, c_flyhack, c_teleporthack, c_jumphack, c_climbhack, c_zaxishack, c_waterwalkhack, c_antiswimhack, c_ignorecontrolhack) AS SELECT account_name, account_id,
          COUNT(CASE WHEN detected_hack = 'SpeedHack' THEN detected_hack END),
          COUNT(CASE WHEN detected_hack = 'FlyHack' THEN detected_hack END),
          COUNT(CASE WHEN detected_hack = 'TeleportHack' THEN detected_hack END),
          COUNT(CASE WHEN detected_hack = 'JumpHack' THEN detected_hack END),
          COUNT(CASE WHEN detected_hack = 'ClimbHack' THEN detected_hack END),
          COUNT(CASE WHEN detected_hack = 'ZAxisHack' THEN detected_hack END),
          COUNT(CASE WHEN detected_hack = 'WaterwalkHack' THEN detected_hack END),
          COUNT(CASE WHEN detected_hack = 'AntiswimHack' THEN detected_hack END),
          COUNT(CASE WHEN detected_hack = 'IgnoreControlHack' THEN detected_hack END)
FROM nemesis_anticheat
GROUP BY account_name;