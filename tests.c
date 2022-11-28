#include "teacut.h"

TEST_FUNCTION(main(),

	TEST_SUITE(first_suite)

	TEST(basic_test0,
		ASSERT(5 == 5);
		//ASSERT(5*1 NE 6*2);
		//ASSERT(5 GE 5);
		)

	TEST(basic_test1,
		//ASSERT(1 LE 4);
		ASSERT(3 LE 9);
		EXPECT(1 == 2);
		)

	TEST_SUITE(second_suite)

	TEST(basic_test_from_second_suite,
		//ASSERT(3 LE 6);
		//ASSERT(8 NE 5);
		ASSERT(5 == 5);
		ASSERT(3*5 EQ 5+10);
		)

	TEST_SUITE(third_suite)
	
	TEST(basic_test3,
		//ASSERT(1 EQ 1);
		)
)

