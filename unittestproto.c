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

#define TEACUT_RED(STR_LITERAL)			"\033[0;31m"			STR_LITERAL "\033[0m"
#define TEACUT_GREEN(STR_LITERAL)		"\033[0;92m"			STR_LITERAL "\033[0m"
#define TEACUT_CYAN(STR_LITERAL)		"\033[0;96m"			STR_LITERAL "\033[0m"
#define TEACUT_WHITE_BG(STR_LITERAL)	"\033[0;107m\033[30m"	STR_LITERAL "\033[0m"

struct teacut_TestAndSuiteData
{
	int testErrors, suiteErrors;
	char *testName, *suiteName;
	bool testDefined, suiteDefined;
};

struct teacut_TestAndSuiteData teacut_data = {.testName = "", .suiteName = ""};

void teacut_firstSet()
{
	static bool setted = false;
	if ( ! setted)
	{
		printf("\n\nStarting tests\n\n");
		setted = true;
	}
}

void teacut_setTest(char* name)
{
	teacut_firstSet();

	if (name != NULL)
	{
		teacut_data.testName = name;
		teacut_data.testDefined = true;
	}
	else
	{
		teacut_data.testName = "";
		teacut_data.testDefined = false;
		teacut_data.testErrors = 0;
	}
}

void teacut_setSuite(char* name)
{
	teacut_firstSet();

	if (name != NULL)
	{
		teacut_data.suiteName = name;
		teacut_data.suiteDefined = true;
	}
	else
	{
		teacut_data.suiteName = "";
		teacut_data.suiteDefined = false;
		teacut_data.suiteErrors = 0;
	}
}

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

	// Expands to
	
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

// SIIVOO TÄÄ YLÖS
void teacut_printResult(const char*, const char*, int);

void teacut_printFailMessage(struct teacut_ExpectationData* expectation)
{
	const char* finalTestName = teacut_data.testDefined  ? teacut_data.testName  :
							 	teacut_data.suiteDefined ? teacut_data.suiteName :
								expectation->func;

	fprintf(stderr,
			"\nTest \"%s\" " TEACUT_RED("[FAILED]") " in %s " TEACUT_WHITE_BG("line %i") "\n", 
			finalTestName, __FILE__, expectation->line);

	fprintf(stderr, TEACUT_CYAN("%s"), expectation->str_a);
	if (expectation->operation != TEACUT_NO_OP)
		fprintf(stderr, TEACUT_CYAN(" %s %s"), expectation->str_operator, expectation->str_b);
	fprintf(stderr, " evaluated to " TEACUT_RED("%g"), expectation->a);
	if (expectation->operation != TEACUT_NO_OP)
		fprintf(stderr, TEACUT_RED(" %s %g"), expectation->str_operator, expectation->b);
	fprintf(stderr, ".\n");

	if (expectation->isAssertion && teacut_data.testDefined)
		teacut_printResult("Test", teacut_data.testName, teacut_data.testErrors);
	if (expectation->isAssertion && teacut_data.suiteDefined)
		teacut_printResult("Suite", teacut_data.suiteName, teacut_data.suiteErrors);

	//if(expectation->isAssertion)
		//teacut_printSuiteFailMessage(expectation);
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

	if ( ! passed)
	{
		if (teacut_data.testDefined)
			teacut_data.testErrors++;
		if (teacut_data.suiteDefined)
			teacut_data.suiteErrors++;

		teacut_printFailMessage(&expectation);

		if(expectation.isAssertion)
			exit(EXIT_FAILURE);
		else 
			return TEACUT_FAILURE;
	}

	return 0;
}

void teacut_printResult(const char* testOrSuite, const char* testOrSuiteName, int errors)
{

	if ( ! errors)
	{
		printf("\n%s \"%s\" " TEACUT_GREEN("[PASSED]") " \n", testOrSuite, testOrSuiteName);
	}
	else
	{
		fprintf(stderr, "\n%s \"%s\" " TEACUT_RED("[FAILED]") " \n",
				testOrSuite, testOrSuiteName);
	}
}

#define TEST_SUITE(SUITE_NAME)											\
	teacut_setSuite(#SUITE_NAME);										\
	auto void teacut_suite_##SUITE_NAME ();								\
	teacut_suite_##SUITE_NAME ();										\
	teacut_printResult("Suite", #SUITE_NAME , teacut_data.suiteErrors);	\
	teacut_setSuite(NULL);												\
	void teacut_suite_##SUITE_NAME ()

#define TEST(TEST_NAME)													\
	teacut_setTest(#TEST_NAME);											\
	auto void teacut_test_##TEST_NAME ();								\
	teacut_test_##TEST_NAME ();											\
	teacut_printResult("Test", #TEST_NAME , teacut_data.testErrors);	\
	teacut_setTest(NULL);												\
	void teacut_test_##TEST_NAME ()

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
	TEST_SUITE(factorial)
	{
		TEST(zero)
		{
			ASSERT(factorial(0) EQ 1);
		}

		TEST(expectation)
		{
			EXPECT(factorial(3) EQ -1);
		}

		TEST(positiveNumbers)
		{
			ASSERT(factorial(1) EQ 1);
			ASSERT(factorial(2) EQ 2);
			ASSERT(factorial(3) EQ 6);
			ASSERT(factorial(12) EQ 479001600);
		}
	}

	return 0;
}
