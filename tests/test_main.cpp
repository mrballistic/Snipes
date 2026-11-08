// Unit tests for the platform-independent logic behind the start screen and
// the in-game bar. Build and run with `make test`.

#include <stdio.h>
#include <string.h>
#include "../src/skill.h"
#include "../src/sdl/hudformat.h"
#include "../src/sdl/startscreen_logic.h"

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

static bool TimeIs(WORD frame, const char *expected)
{
	char out[8];
	FormatGameTime(frame, out);
	return strcmp(out, expected) == 0;
}

static void TestFormatGameTime()
{
	CHECK(TimeIs(0, "0:00"));
	CHECK(TimeIs(18, "0:00"));
	CHECK(TimeIs(19, "0:01"));
	CHECK(TimeIs(1093, "1:00"));
	CHECK(TimeIs(65535, "59:59")); // frame is a WORD, so this is the largest time shown
}

static StartSelection Sel(char letter, Uint number) { StartSelection s; s.letter = L(letter); s.number = number; return s; }
static bool SelIs(const StartSelection &s, char letter, Uint number) { return s.letter == L(letter) && s.number == number; }

static void TestStartScreenKey()
{
	StartSelection s;
	s = Sel('A', 1); CHECK(StartScreenKey(s, StartKey_Right) == StartAction_None); CHECK(SelIs(s, 'B', 1));
	s = Sel('A', 1); StartScreenKey(s, StartKey_Left);  CHECK(SelIs(s, 'A', 1));
	s = Sel('Z', 1); StartScreenKey(s, StartKey_Right); CHECK(SelIs(s, 'Z', 1));
	s = Sel('C', 1); StartScreenKey(s, StartKey_Left);  CHECK(SelIs(s, 'B', 1));
	s = Sel('A', 1); StartScreenKey(s, StartKey_Up);    CHECK(SelIs(s, 'A', 2));
	s = Sel('A', 9); StartScreenKey(s, StartKey_Up);    CHECK(SelIs(s, 'A', 9));
	s = Sel('A', 1); StartScreenKey(s, StartKey_Down);  CHECK(SelIs(s, 'A', 1));
	s = Sel('A', 5); StartScreenKey(s, StartKey_Down);  CHECK(SelIs(s, 'A', 4));
	s = Sel('A', 1); StartScreenKey(s, StartKey_Char, 'm'); CHECK(SelIs(s, 'M', 1));
	s = Sel('A', 1); StartScreenKey(s, StartKey_Char, 'Q'); CHECK(SelIs(s, 'Q', 1));
	s = Sel('A', 1); StartScreenKey(s, StartKey_Char, '7'); CHECK(SelIs(s, 'A', 7));
	s = Sel('A', 1); CHECK(StartScreenKey(s, StartKey_Char, '0') == StartAction_None); CHECK(SelIs(s, 'A', 1));
	s = Sel('A', 1); StartScreenKey(s, StartKey_Char, '#'); CHECK(SelIs(s, 'A', 1));
	s = Sel('B', 3); CHECK(StartScreenKey(s, StartKey_Return) == StartAction_Start); CHECK(SelIs(s, 'B', 3));
	s = Sel('B', 3); CHECK(StartScreenKey(s, StartKey_Escape) == StartAction_Quit);
}

static void TestSavedLevel()
{
	StartSelection s;
	s = Sel('A', 1); CHECK(ParseSavedLevel("M4", s));   CHECK(SelIs(s, 'M', 4));
	s = Sel('A', 1); CHECK(ParseSavedLevel("m4\n", s)); CHECK(SelIs(s, 'M', 4));
	const char *bad[] = { "", "Z0", "44", "M", "4M", "M10" };
	for (unsigned i = 0; i < sizeof(bad)/sizeof(bad[0]); i++)
	{
		s = Sel('B', 2);
		CHECK(!ParseSavedLevel(bad[i], s));
		CHECK(SelIs(s, 'B', 2));
	}
	s = Sel('B', 2); CHECK(!ParseSavedLevel(NULL, s)); CHECK(SelIs(s, 'B', 2));

	char out[4];
	FormatSavedLevel(Sel('W', 9), out); CHECK(strcmp(out, "W9") == 0);
	s = Sel('A', 1); CHECK(ParseSavedLevel(out, s)); CHECK(SelIs(s, 'W', 9));
}

int main()
{
	TestDescribeSkillLevel();
	TestFormatGameTime();
	TestStartScreenKey();
	TestSavedLevel();
	printf("%d checks, %d failures\n", checks, failures);
	return failures ? 1 : 0;
}
