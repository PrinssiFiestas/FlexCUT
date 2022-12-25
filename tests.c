#include <stdio.h> // for debugging

#include "teacut.h"

int factorial(int x)
{
	int y = 1;
	for(int i = 1; i <= x; i++)
		y *= i;
	return y;
}

int main()
{
	EXPECT(0+0 EQ 1+1);

	TEST_SUITE(factorial)
	{
		TEST(zero)
		{
			ASSERT(factorial(0) EQ 1);
		}

		EXPECT(factorial(3) EQ -1);

		TEST(positiveNumbers)
		{
			ASSERT(factorial(1) EQ 1);
			ASSERT(factorial(2) EQ 2);
			ASSERT(factorial(3) EQ 6);
			ASSERT(factorial(12) EQ 479001600);
		}
	}

	TEST_SUITE(nested)
	{
		TEST(first)
		{
			EXPECT(1 == 1);

			TEST(first_inner)
			{
				EXPECT(0);
			}
		}
		
		TEST(second)
		{
			EXPECT(1 == 1);
		}
	}

	TEST(suiteInTest)
	{
		TEST_SUITE(suite)
		{
			TEST(right)
			{
				ASSERT(1 + 1 EQ 2);
			}
			TEST(wrong)
			{
				EXPECT(1 + 1 NE 2);
			}
		}
	}

	printf("total expectation fails: %i\n", teacut_globalData.expectationFails);
	printf("total test fails: %i\n", teacut_globalData.testFails);
	printf("total suite fails: %i\n", teacut_globalData.suiteFails);
	return 0;
}

