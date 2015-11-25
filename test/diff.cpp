#include <string>
#include <iostream>

#include <gtest/gtest.h>

#include <diff.h>

using namespace izi::diff;


TEST(diff, result)
{
	std::string aText1("This is first string");
	std::string aText2("This is second string");

	result<std::string> aDiff;
	aDiff.calculate(aText1, aText2);

	for(result<std::string>::const_iterator aDiffIt = aDiff.begin(); aDiffIt != aDiff.end(); ++aDiffIt)
	{
		std::cout << aDiffIt->first.value() << " : \"" << aDiffIt->second << "\"" << std::endl;
	}

	EXPECT_EQ(aDiff.size(), 3u);
}
