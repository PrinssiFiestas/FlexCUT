#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

/*int main()
{
	TEST_SUITE(factorial)
	{
		TEST(zero)
		{
			ASSERT(factorial(0) EQ 1);
		}

		TEST(positive)
		{
			ASSERT(factorial(1) EQ 1);
			ASSERT(factorial(2) EQ 2);
			ASSERT(factorial(3) EQ 6);
		}
	}
	return 0;
}*/

//--------------------------------------------------

#define TEACUT_RED 				"\033[0;31m"
#define TEACUT_GREEN 			"\033[0;92m"
#define TEACUT_CYAN 			"\033[0;96m"
#define TEACUT_WHITE_BG 		"\033[0;107m\033[30m"
#define TEACUT_DEFAULT_COLOR 	"\033[0m"

int teacut_colorfprintf(const char* color, FILE* file, const char* format, ...)
{
	fprintf(file, "%s", color);

	va_list arg;
	int done;

	va_start(arg, format);
	done = vfprintf(file, format, arg);
	va_end(arg);

	fprintf(file, TEACUT_DEFAULT_COLOR);

	return done;
}

int teacut_colorprintf(const char* color, const char* format, ...)
{
	printf("%s", color);

	va_list arg;
	int done;

	va_start(arg, format);
	done = vfprintf(stdout, format, arg);
	va_end(arg);

	printf(TEACUT_DEFAULT_COLOR);

	return done;
}


static int teacut_errors = 0;
static char* teacut_testName  = "";
static char* teacut_suiteName = "";

#define OP_TABLE	\
	X(_EQ, ==)		\
	X(_NE, !=)		\
	X(_GT, >)		\
	X(_LT, <)		\
	X(_GE, >=)		\
	X(_LE, <=)		\

enum teacut_BooleanOperators
{
	TEACUT_NO_OP = -1,

#define X(OP, DUMMY) TEACUT##OP,
	OP_TABLE
#undef X

// Evaluates to

/*	TEACUT_EQ,
	TEACUT_NE,
	TEACUT_GT,
	TEACUT_LT,
	TEACUT_GE,
	TEACUT_LE,*/

	TEACUT_OPS_LENGTH
};

#define EQ ,TEACUT_EQ,
#define NE ,TEACUT_NE,
#define GT ,TEACUT_GT,
#define LT ,TEACUT_LT,
#define GE ,TEACUT_GE,
#define LE ,TEACUT_LE,

//const char TEACUT_STR_OPERATORS[TEACUT_OPS_LENGTH][3] = {"==", "!=", ">", "<", ">=", "<="};
const char TEACUT_STR_OPERATORS[TEACUT_OPS_LENGTH][3] = {
#define X(DUMMY, OP) #OP,
	OP_TABLE
#undef X
};

// Boolean operations as a function
// Allows macros EQ, NE, etc. to be used like operators
bool teacut_compare(double a, int operation, double b)
{
	switch(operation)
	{
		case TEACUT_NO_OP:
			return a;

	#define X(OP_ENUM, OP) 		\
		case TEACUT##OP_ENUM:	\
			return a OP b;

		OP_TABLE

	#undef X

	// Evaluates to
	
	/*	case TEACUT_EQ:
			return a == b;
		case TEACUT_NE:
			return a != b;
		// etc...
	*/
	}

	return 0&&(a+b); // Gets rid of pointless compiler warnings
}

struct teacut_ExpectationData
{
	double a, b;
	const char *str_a, *str_b, *str_operator, *func;
	enum teacut_BooleanOperators operation;
	int line;
	bool isAssertion;
};

void teacut_printFailMessage(struct teacut_ExpectationData* expectation)
{
	bool testNameIsEmpty  = !strcmp(teacut_testName,  "");
	bool suiteNameIsEmpty = !strcmp(teacut_suiteName, "");

	const char* finalTestName = ! testNameIsEmpty  ? teacut_testName  :
							 	! suiteNameIsEmpty ? teacut_suiteName :
								expectation->func;

	fprintf(stderr, "\nTest \"%s\" ", finalTestName);
	teacut_colorfprintf(TEACUT_RED, stderr, "[FAILED]");
	fprintf(stderr, " in %s ", __FILE__);
	teacut_colorfprintf(TEACUT_WHITE_BG, stderr, "line %i", expectation->line);
	fprintf(stderr, "\n\n");

	teacut_colorfprintf(TEACUT_CYAN, stderr, "%s", expectation->str_a);
	if (expectation->operation != TEACUT_NO_OP)
		teacut_colorfprintf(TEACUT_CYAN, stderr, " %s %s",
				expectation->str_operator, expectation->str_b);
	fprintf(stderr, " evaluated to ");
	teacut_colorfprintf(TEACUT_RED, stderr, "%g", expectation->a);
	if (expectation->operation != TEACUT_NO_OP)
		teacut_colorfprintf(TEACUT_RED, stderr, " %s %g",
				expectation->str_operator, expectation->b);
	fprintf(stdout, ".\n\n");

	if ( ! testNameIsEmpty && ! suiteNameIsEmpty)
	{
		fprintf(stderr, "Suite \"%s\" ", teacut_suiteName);
		teacut_colorfprintf(TEACUT_RED, stderr, "[FAILED]\n");
	}

	if (testNameIsEmpty && ! suiteNameIsEmpty)
	{
		fprintf(stderr, "Testing in \"%s\" ", expectation->func);
		teacut_colorfprintf(TEACUT_RED, stderr, "[FAILED]\n");
	}

	if ( ! testNameIsEmpty && suiteNameIsEmpty)
	{
		fprintf(stderr, "Suite \"%s\" ", expectation->func);
		teacut_colorfprintf(TEACUT_RED, stderr, "[FAILED]\n");
	}
}

#define TEACUT_ASSERT(ASS) 					\
	teacut_assert							\
	(										\
		(struct teacut_ExpectationData)		\
		{									\
			.a 			 	= ASS,			\
			.str_a		 	= #ASS,			\
			.operation	 	= TEACUT_NO_OP,	\
			.line 		 	= __LINE__,		\
			.isAssertion 	= true,			\
			.func			= __func__		\
		}									\
	)

#define TEACUT_ASSERT_CMP(A, OP, B) 					\
	teacut_assert										\
	(													\
	 	(struct teacut_ExpectationData)					\
		{												\
			.a 	   			= A,						\
			.b 				= B,						\
			.str_a 			= #A,						\
			.str_b 			= #B,						\
			.str_operator 	= TEACUT_STR_OPERATORS[OP],	\
			.operation		= OP,						\
			.line 			= __LINE__,					\
			.isAssertion	= true,						\
			.func			= __func__					\
		}												\
	)

#define GET_MACRO_NAME(DUMMY1, DUMMY2, SUMMY3, NAME, ...) NAME
#define ASSERT(...)		\
	GET_MACRO_NAME(__VA_ARGS__, TEACUT_ASSERT_CMP, DUMMY, TEACUT_ASSERT)(__VA_ARGS__)

#define TEACUT_EXPECT(EXP) 					\
	teacut_assert							\
	(										\
		(struct teacut_ExpectationData)		\
		{									\
			.a 			 	= EXP,			\
			.str_a		 	= #EXP,			\
			.operation	 	= TEACUT_NO_OP,	\
			.line 		 	= __LINE__,		\
			.isAssertion 	= false,		\
			.func			= __func__		\
		}									\
	)

#define TEACUT_EXPECT_CMP(A, OP, B) 					\
	teacut_assert										\
	(													\
	 	(struct teacut_ExpectationData)					\
		{												\
			.a 	   		  	= A,						\
			.b 			  	= B,						\
			.str_a 		  	= #A,						\
			.str_b 		  	= #B,						\
			.str_operator 	= TEACUT_STR_OPERATORS[OP],	\
			.operation	  	= OP,						\
			.line 		  	= __LINE__,					\
			.isAssertion  	= false,					\
			.func			= __func__					\
		}												\
	)

#define GET_MACRO_NAME(DUMMY1, DUMMY2, SUMMY3, NAME, ...) NAME
#define EXPECT(...)		\
	GET_MACRO_NAME(__VA_ARGS__, TEACUT_EXPECT_CMP, DUMMY, TEACUT_EXPECT)(__VA_ARGS__)

#define TEACUT_FAILURE 1

int teacut_assert(struct teacut_ExpectationData expectation)
{
	bool passed = teacut_compare(expectation.a,
								 expectation.operation,
								 expectation.b);

	if( ! passed)
	{
		teacut_errors += 1;
		teacut_printFailMessage(&expectation);

		if(expectation.isAssertion)
			exit(EXIT_FAILURE);
		else 
			return TEACUT_FAILURE;
	}

	return 0;
}

#define TEST_SUITE(SUITE_NAME)		\
	teacut_##SUITE_NAME = gvfdsg

//-------------------------------------------------------

int factorial(int x)
{
	int y = 1;
	for(int i = 1; i <= x; i++)
		y *= i;
	return y;
}

int main()
{
	/*ME*/teacut_suiteName = "factorial";
	/*ME*/auto void teacut_test_factorial();
	/*ME*/teacut_test_factorial();
	/*ME*///printSuiteResult();
	/*ME*/teacut_suiteName = "";
	/*ME*/void teacut_test_factorial()
	{
		/*ME*/teacut_testName = "zero";
		/*ME*/auto void teacut_test_zero();
		/*ME*/teacut_test_zero();
		/*ME*///printTestResult();
		/*ME*/teacut_testName = "";
		/*ME*/void teacut_test_zero()
		{
			ASSERT(factorial(0) EQ 0);
			ASSERT(0 == 0);
		}
	}

	return 0;
}
