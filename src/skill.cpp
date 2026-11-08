#include "skill.h"

const BYTE snipeShootingAccuracyTable    ['Z'-'A'+1] = {2, 3, 4, 3, 4, 4, 3, 4, 3, 4, 4, 5, 3, 4, 3, 4, 3, 4, 3, 4, 4, 5, 4, 4, 5, 5}; // higher number = higher accuracy
const bool enableSmallSnipesTable        ['Z'-'A'+1] = {0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1};
const bool bouncingBulletTable           ['Z'-'A'+1] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
const BYTE smallSnipeExplosionChanceTable['Z'-'A'+1] = {0x7F, 0x7F, 0x7F, 0x3F, 0x3F, 0x1F, 0x7F, 0x7F, 0x3F, 0x3F, 0x1F, 0x1F, 0x7F, 0x7F, 0x3F, 0x3F, 0x7F, 0x7F, 0x3F, 0x3F, 0x1F, 0x1F, 0x3F, 0x1F, 0x1F, 0x0F}; // lower number = higher chance
const BYTE maxSnipesTable                ['9'-'1'+1] = { 10,  20,  30,  40,  60,  80, 100, 120, 150};
const BYTE numSnipePortalsTable          ['9'-'1'+1] = {  3,   3,   4,   4,   5,   5,   6,   8,  10};
const BYTE numLivesTable                 ['9'-'1'+1] = {  5,   5,   5,   5,   5,   4,   4,   3,   2};

SkillLevelInfo DescribeSkillLevel(Uint letter, Uint number)
{
	SkillLevelInfo info;
	info.snipes          = maxSnipesTable      [number-1];
	info.portals         = numSnipePortalsTable[number-1];
	info.lives           = numLivesTable       [number-1];
	info.aim             = snipeShootingAccuracyTable[letter];
	info.smallSnipes     = enableSmallSnipesTable    [letter];
	info.bouncingBullets = bouncingBulletTable       [letter];
	// These mirror the assignments at the start of each match in Snipes.cpp
	info.electricWalls   = letter >= 'M'-'A';
	info.armoredPortals  = letter >= 'W'-'A';
	return info;
}
