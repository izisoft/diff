#include <string>

#include <gtest/gtest.h>

#include <internal/algorithm.h>

using namespace izi::diff;


TEST(algorithm, equal)
{
	std::string aString1("This is example string");
	EXPECT_TRUE(detail::equal(aString1.begin(), aString1.end(), aString1.begin(), aString1.end()));

	std::string aString2(aString1);
	EXPECT_TRUE(detail::equal(aString1.begin(), aString1.end(), aString2.begin(), aString2.end()));

	aString2[5] = 'X';
	EXPECT_FALSE(detail::equal(aString1.begin(), aString1.end(), aString2.begin(), aString2.end()));

	std::string aString3(aString1);
	aString3.append("X");
	EXPECT_FALSE(detail::equal(aString1.begin(), aString1.end(), aString3.begin(), aString3.end()));

	std::string aString4;
	EXPECT_FALSE(detail::equal(aString1.begin(), aString1.end(), aString4.begin(), aString4.end()));
}

TEST(algorithm, starts_with)
{
	std::string aTest("This is example string");
	std::string aPfx1("This is ex");
	std::string aPfx2("Th");
	std::string aPfx3("This ix");
	std::string aPfx4("This is example strings");
	std::string aPfx5;

	EXPECT_TRUE(detail::starts_with(aTest.begin(), aTest.end(), aPfx1.begin(), aPfx1.end()));
	EXPECT_TRUE(detail::starts_with(aTest.begin(), aTest.end(), aPfx2.begin(), aPfx2.end()));
	EXPECT_FALSE(detail::starts_with(aTest.begin(), aTest.end(), aPfx3.begin(), aPfx3.end()));
	EXPECT_FALSE(detail::starts_with(aTest.begin(), aTest.end(), aPfx4.begin(), aPfx4.end()));
	EXPECT_TRUE(detail::starts_with(aTest.begin(), aTest.end(), aPfx5.begin(), aPfx5.end()));
}

TEST(algorithm, ends_with)
{
	std::string aTest("This is example string");
	std::string aSfx1(" example string");
	std::string aSfx2("ing");
	std::string aSfx3("example saring");
	std::string aSfx4("And This is example string");
	std::string aSfx5;

	EXPECT_TRUE(detail::ends_with(aTest.begin(), aTest.end(), aSfx1.begin(), aSfx1.end()));
	EXPECT_TRUE(detail::ends_with(aTest.begin(), aTest.end(), aSfx2.begin(), aSfx2.end()));
	EXPECT_FALSE(detail::ends_with(aTest.begin(), aTest.end(), aSfx3.begin(), aSfx3.end()));
	EXPECT_FALSE(detail::ends_with(aTest.begin(), aTest.end(), aSfx4.begin(), aSfx4.end()));
	EXPECT_TRUE(detail::ends_with(aTest.begin(), aTest.end(), aSfx5.begin(), aSfx5.end()));
}

TEST(algorithm, common_prefix)
{
	std::string aString1("This is example string");
	std::string aString2("This is an example string");

	std::string::const_iterator aPfxIt = detail::common_prefix(aString1.begin(), aString1.end(), aString2.begin(), aString2.end());
	EXPECT_EQ(aPfxIt, aString1.begin() + 8);

	aString2 = "That is example string";
	aPfxIt = detail::common_prefix(aString1.begin(), aString1.end(), aString2.begin(), aString2.end());
	EXPECT_EQ(aPfxIt, aString1.begin() + 2);

	aString2 = "Ahis is example string";
	aPfxIt = detail::common_prefix(aString1.begin(), aString1.end(), aString2.begin(), aString2.end());
	EXPECT_EQ(aPfxIt, aString1.begin());

	aString2.clear();
	aPfxIt = detail::common_prefix(aString1.begin(), aString1.end(), aString2.begin(), aString2.end());
	EXPECT_EQ(aPfxIt, aString1.begin());

	aString2 = "This is example strings";
	aPfxIt = detail::common_prefix(aString1.begin(), aString1.end(), aString2.begin(), aString2.end());
	EXPECT_EQ(aPfxIt, aString1.end());
}
