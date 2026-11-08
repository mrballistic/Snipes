#pragma once

#include "types.h"

// Per-skill-level tables, indexed by skill letter (0 = A ... 25 = Z) or by
// skill number minus one (0 = 1 ... 8 = 9).
extern const BYTE snipeShootingAccuracyTable    ['Z'-'A'+1]; // higher number = higher accuracy
extern const bool enableSmallSnipesTable        ['Z'-'A'+1];
extern const bool bouncingBulletTable           ['Z'-'A'+1];
extern const BYTE smallSnipeExplosionChanceTable['Z'-'A'+1]; // lower number = higher chance
extern const BYTE maxSnipesTable                ['9'-'1'+1];
extern const BYTE numSnipePortalsTable          ['9'-'1'+1];
extern const BYTE numLivesTable                 ['9'-'1'+1];

// What a skill level means, for display on the start screen.
struct SkillLevelInfo
{
	Uint snipes, portals, lives, aim;
	bool smallSnipes, bouncingBullets, electricWalls, armoredPortals;
};

SkillLevelInfo DescribeSkillLevel(Uint letter /* 0-25 */, Uint number /* 1-9 */);
