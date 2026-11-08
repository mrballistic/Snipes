// Unit tests for the platform-independent logic behind the start screen and
// the in-game bar. Build and run with `make test`.

#include <stdio.h>
#include <string.h>
#include "../skill.h"

static int failures = 0, checks = 0;

#define CHECK(cond) \
	do { \
		checks++; \
		if (!(cond)) { failures++; fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); } \
	} while (0)

static Uint L(char letter) { return (Uint)(letter - 'A'); }

static void TestDescribeSkillLevel()
{
	SkillLevelInfo a1 = DescribeSkillLevel(L('A'), 1);
	CHECK(a1.snipes == 10); CHECK(a1.portals == 3); CHECK(a1.lives == 5);
	CHECK(!a1.smallSnipes); CHECK(!a1.bouncingBullets); CHECK(!a1.electricWalls); CHECK(!a1.armoredPortals);

	SkillLevelInfo e5 = DescribeSkillLevel(L('E'), 5);
	CHECK(e5.snipes == 60); CHECK(e5.portals == 5); CHECK(e5.lives == 5);
	CHECK(e5.smallSnipes); CHECK(!e5.bouncingBullets); CHECK(!e5.electricWalls);

	CHECK(DescribeSkillLevel(L('M'), 4).electricWalls);
	CHECK(!DescribeSkillLevel(L('M'), 4).armoredPortals);
	CHECK(!DescribeSkillLevel(L('L'), 4).electricWalls);

	SkillLevelInfo w9 = DescribeSkillLevel(L('W'), 9);
	CHECK(w9.armoredPortals); CHECK(w9.snipes == 150); CHECK(w9.portals == 10); CHECK(w9.lives == 2);
	CHECK(!DescribeSkillLevel(L('V'), 9).armoredPortals);

	CHECK(DescribeSkillLevel(L('G'), 1).bouncingBullets);
	CHECK(DescribeSkillLevel(L('A'), 1).aim == 2);
}

int main()
{
	TestDescribeSkillLevel();
	printf("%d checks, %d failures\n", checks, failures);
	return failures ? 1 : 0;
}
