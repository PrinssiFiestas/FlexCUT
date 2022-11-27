#include "teacut.h"

TEST_FUNCTION(main(),

	TEST_SUITE(first_suite)

	TEST(basic_test0,
		ASSERT(5 == 5);
		ASSERT(5*1 EQ 6*2);
		ASSERT(6==5);
		)

	TEST(basic_test1,
		ASSERT(1 GE 4);
		)

	TEST_SUITE(second_suite)
	{
		TEST(basic_test_from_second_suite,
			ASSERT(3 LE 6);
			ASSERT(8 NE 5);
			ASSERT(3*5 EQ 3);
			)
	} // optional curly braces for structuring and namespacing
)

