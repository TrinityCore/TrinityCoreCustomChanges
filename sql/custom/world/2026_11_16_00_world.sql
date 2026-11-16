DELETE FROM `gameobject_template` WHERE `entry` = 250000;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `AIName`, `ScriptName`) VALUES
(250000, 34, 7627, 'Bóveda de Componentes', '', '', '', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '');

DELETE FROM `gameobject_template_locale` WHERE `entry` = 250000 AND `locale` IN ('esES', 'esMX');
INSERT INTO `gameobject_template_locale` (`entry`, `locale`, `name`, `castBarCaption`, `VerifiedBuild`) VALUES
(250000, 'esES', 'Bóveda de Componentes', '', 0),
(250000, 'esMX', 'Bóveda de Componentes', '', 0);
