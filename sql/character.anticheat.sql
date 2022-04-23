CREATE TABLE IF NOT EXISTS `daily_players_reports` (
  `guid` int unsigned NOT NULL DEFAULT 0,
  `creation_time` int unsigned NOT NULL DEFAULT 0,
  `average` float NOT NULL DEFAULT 0,
  `total_reports` bigint unsigned NOT NULL DEFAULT 0,
  `speed_reports` bigint unsigned NOT NULL DEFAULT 0,
  `fly_reports` bigint unsigned NOT NULL DEFAULT 0,
  `jump_reports` bigint unsigned NOT NULL DEFAULT 0,
  `waterwalk_reports` bigint unsigned NOT NULL DEFAULT 0,
  `teleportplane_reports` bigint unsigned NOT NULL DEFAULT 0,
  `climb_reports` bigint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

ALTER TABLE `daily_players_reports`
	CHANGE COLUMN `climb_reports` `climb_reports` BIGINT UNSIGNED NOT NULL DEFAULT '0' AFTER `teleportplane_reports`,
	ADD COLUMN `teleport_reports` BIGINT UNSIGNED NOT NULL DEFAULT '0' AFTER `climb_reports`,
	ADD COLUMN `ignorecontrol_reports` BIGINT UNSIGNED NOT NULL DEFAULT '0' AFTER `teleport_reports`,
	ADD COLUMN `zaxis_reports` BIGINT UNSIGNED NOT NULL DEFAULT '0' AFTER `ignorecontrol_reports`;

  
CREATE TABLE IF NOT EXISTS `players_reports_status` (
  `guid` int unsigned NOT NULL DEFAULT 0,
  `creation_time` int unsigned NOT NULL DEFAULT 0,
  `average` float NOT NULL DEFAULT 0,
  `total_reports` bigint unsigned NOT NULL DEFAULT 0,
  `speed_reports` bigint unsigned NOT NULL DEFAULT 0,
  `fly_reports` bigint unsigned NOT NULL DEFAULT 0,
  `jump_reports` bigint unsigned NOT NULL DEFAULT 0,
  `waterwalk_reports` bigint unsigned NOT NULL DEFAULT 0,
  `teleportplane_reports` bigint unsigned NOT NULL DEFAULT 0,
  `climb_reports` bigint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

ALTER TABLE `players_reports_status`
	CHANGE COLUMN `climb_reports` `climb_reports` BIGINT UNSIGNED NOT NULL DEFAULT '0' AFTER `teleportplane_reports`,
	ADD COLUMN `teleport_reports` BIGINT UNSIGNED NOT NULL DEFAULT '0' AFTER `climb_reports`,
	ADD COLUMN `ignorecontrol_reports` BIGINT UNSIGNED NOT NULL DEFAULT '0' AFTER `teleport_reports`,
	ADD COLUMN `zaxis_reports` BIGINT UNSIGNED NOT NULL DEFAULT '0' AFTER `ignorecontrol_reports`;
	