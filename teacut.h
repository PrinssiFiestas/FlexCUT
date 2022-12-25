#ifndef TEACUT_H
#define TEACUT_H

#ifdef __cplusplus
#include <atomic>
extern "C" {
#else
#include <stdatomic.h>
#endif

#include <stdbool.h>

//*************************************************************************************
//
//		PUBLIC API
//
//*************************************************************************************

// Use these macros to define test or suite functions.
// Tests and suites have to be defined in function scope so they can be run automatically.
// Tests and suites are optional: EXPECT() and ASSERT() can be used anywhere in your code.
// Tests and suites can be nested arbitrarily.
#define TEST(NAME)			TEACUT_TEST_OR_SUITE(NAME,test)
#define TEST_SUITE(NAME)	TEACUT_TEST_OR_SUITE(NAME,suite)
// Example use:
/*
int main() // function scope required!
{
	TEST_SUITE(exampleSuite) // optional suite
	{
		TEST(exampleTest) // optional test
		{
			ASSERT(1 + 1 EQ 2);
		}
	}
}
*/

#define TEACUT_FAILURE 1

// Does nothing when expression is true.
// Exits program and prints failure message when expression is false.
// Really a macro.
void ASSERT(bool expression);

// Returns 0 when expression is true.
// Prints failure message and returns TEACUT_FAILURE when expression is false.
// Really a macro.
int EXPECT(bool expression);

// 'Pseudo-operators' to be used in argument for ASSERT() or EXPECT().
// Use ASSERT(A EQ B) instead of ASSERT(A == B) for more info at failure!
#define EQ ,TEACUT_EQ,
#define NE ,TEACUT_NE,
#define GT ,TEACUT_GT,
#define LT ,TEACUT_LT,
#define GE ,TEACUT_GE,
#define LE ,TEACUT_LE,

//*************************************************************************************
//
//		END OF PUBLIC API
//
//		Structs, functions and macros below are not meant to be used by the user.
//		However, they are required for macros to work so here you go I guess.
//
//*************************************************************************************

#define TEACUT_RED(STR_LITERAL)			"\033[0;31m"			STR_LITERAL "\033[0m"
#define TEACUT_GREEN(STR_LITERAL)		"\033[0;92m"			STR_LITERAL "\033[0m"
#define TEACUT_MAGENTA(STR_LITERAL)		"\033[0;95m"			STR_LITERAL "\033[0m"
#define TEACUT_CYAN(STR_LITERAL)		"\033[0;96m"			STR_LITERAL "\033[0m"
#define TEACUT_WHITE_BG(STR_LITERAL)	"\033[0;107m\033[30m"	STR_LITERAL "\033[0m"

struct teacut_TestAndSuiteData
{
#ifdef __cplusplus
	std::atomic<int> testFails, suiteFails, expectationFails/*includes assertion fails*/;
#else
	_Atomic int testFails, suiteFails, expectationFails/*includes assertion fails*/;
#endif
	char *testName, *suiteName;
	bool testDefined, suiteDefined;
	struct teacut_TestAndSuiteData* parent;
} teacut_globalData;

#define OP_TABLE	\
	X(_EQ, ==)		\
	X(_NE, !=)		\
	X(_GT, >)		\
	X(_LT, <)		\
	X(_GE, >=)		\
	X(_LE, <=)		\

enum teacut_BooleanOperator
{
	TEACUT_NO_OP = -1,

#define X(OP, DUMMY) TEACUT##OP,
	OP_TABLE
#undef X

// Expands to

/*	TEACUT_EQ,
	TEACUT_NE,
	TEACUT_GT,
	TEACUT_LT,
	TEACUT_GE,
	TEACUT_LE,*/

	TEACUT_OPS_LENGTH
};

struct teacut_ExpectationData
{
	double a, b;
	const char *str_a, *str_b, *str_operator, *func;
	enum teacut_BooleanOperator operation;
	int line;
	bool isAssertion;
};

// Boolean operations as a function
// Allows macros EQ, NE, etc. to be used like operators
bool teacut_compare(double expression_a, enum teacut_BooleanOperator, double expression_b);

void teacut_printTestOrSuiteResult(struct teacut_TestAndSuiteData*);

void teacut_printExpectationFail(struct teacut_ExpectationData*, struct teacut_TestAndSuiteData*);

int teacut_assert(struct teacut_ExpectationData, struct teacut_TestAndSuiteData*);

void teacut_printTestOrSuiteResult(struct teacut_TestAndSuiteData*);

void teacut_addFailToParentAndGlobalDataWhenFailed(struct teacut_TestAndSuiteData*);

extern struct teacut_TestAndSuiteData *const teacut_shadow;
extern const char TEACUT_STR_OPERATORS[TEACUT_OPS_LENGTH][3];

#define TEACUT_DATA_FOR_ASSERT (teacut_shadow->testDefined || teacut_shadow->suiteDefined ? \
								teacut_shadow : &teacut_globalData)

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
		},									\
		TEACUT_DATA_FOR_ASSERT				\
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
		},												\
		TEACUT_DATA_FOR_ASSERT							\
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
		},									\
		TEACUT_DATA_FOR_ASSERT				\
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
		},												\
		TEACUT_DATA_FOR_ASSERT							\
	)

#define GET_MACRO_NAME(DUMMY1, DUMMY2, SUMMY3, NAME, ...) NAME
#define EXPECT(...)		\
	GET_MACRO_NAME(__VA_ARGS__, TEACUT_EXPECT_CMP, DUMMY, TEACUT_EXPECT)(__VA_ARGS__)

#define TEACUT_TEST_OR_SUITE(NAME, TEST_OR_SUITE)									\
	auto void teacut_##TEST_OR_SUITE##_##NAME (struct teacut_TestAndSuiteData*);	\
	struct teacut_TestAndSuiteData* teacut_##TEST_OR_SUITE##_##NAME##Parent = teacut_shadow;\
	{																				\
		struct teacut_TestAndSuiteData teacut_##TEST_OR_SUITE = 					\
		{																			\
			.TEST_OR_SUITE##Name = #NAME,											\
			.TEST_OR_SUITE##Defined = true,											\
			.parent = teacut_##TEST_OR_SUITE##_##NAME##Parent						\
		};																			\
		teacut_##TEST_OR_SUITE##_##NAME (&teacut_##TEST_OR_SUITE);					\
		teacut_addFailToParentAndGlobalDataWhenFailed(&teacut_##TEST_OR_SUITE);		\
		teacut_printTestOrSuiteResult(&teacut_##TEST_OR_SUITE);						\
	}																				\
	void teacut_##TEST_OR_SUITE##_##NAME (struct teacut_TestAndSuiteData* teacut_shadow)

//*************************************************************************************
//
// 		IMPLEMENTATION
//
//*************************************************************************************

#ifdef TEACUT_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>

struct teacut_TestAndSuiteData teacut_globalData = {};
struct teacut_TestAndSuiteData *const teacut_shadow = &teacut_globalData;
struct teacut_TestAndSuiteData *const teacut_dummy  = &teacut_globalData;

//const char TEACUT_STR_OPERATORS[TEACUT_OPS_LENGTH][3] = {"==", "!=", ">", "<", ">=", "<="};
const char TEACUT_STR_OPERATORS[TEACUT_OPS_LENGTH][3] = {
#define X(DUMMY, OP) #OP,
	OP_TABLE
#undef X
};

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

// Finds suite by going trough all parent data
struct teacut_TestAndSuiteData* findSuite(struct teacut_TestAndSuiteData* data)
{
	bool suiteFound 	= data->suiteDefined;
	bool suiteNotFound	= data== &teacut_globalData;

	if (suiteFound)
		return data;
	else if (suiteNotFound)
		return NULL;
	else // keep looking
		return findSuite(data->parent);
}

void teacut_printExpectationFail(struct teacut_ExpectationData* expectation,
								 struct teacut_TestAndSuiteData* data)
{
	const char* finalTestName = data->testDefined  ? data->testName  :
								data->suiteDefined ? data->suiteName :
								expectation->func;

	if (expectation->isAssertion)
		fprintf(stderr, "\nAssertion ");
	else
		fprintf(stderr, "\nExpectation ");
			teacut_globalData.testFails += (data->parent != &teacut_globalData);
	fprintf(stderr,
			"in \"%s\" " TEACUT_RED("[FAILED]") " in %s " TEACUT_WHITE_BG("line %i") "\n", 
			finalTestName, __FILE__, expectation->line);

	fprintf(stderr, TEACUT_MAGENTA("%s"), expectation->str_a);
	if (expectation->operation != TEACUT_NO_OP)
		fprintf(stderr, TEACUT_MAGENTA(" %s %s"), expectation->str_operator, expectation->str_b);
	fprintf(stderr, " evaluated to " TEACUT_RED("%g"), expectation->a);
	if (expectation->operation != TEACUT_NO_OP)
		fprintf(stderr, TEACUT_RED(" %s %g"), expectation->str_operator, expectation->b);
	fprintf(stderr, ".\n");

	if (expectation->isAssertion && data->testDefined)
		teacut_printTestOrSuiteResult(data);
	struct teacut_TestAndSuiteData* suite = findSuite(data);
	if (suite != NULL)
		teacut_printTestOrSuiteResult(suite);
}

// Adds one fail to all parents all the way to teacut_globalData
void teacut_addExpectationFail(struct teacut_TestAndSuiteData* data)
{
	data->expectationFails++;
	if (data != &teacut_globalData)
		teacut_addExpectationFail(data->parent);
}

int teacut_assert(struct teacut_ExpectationData expectation,
				  struct teacut_TestAndSuiteData* data)
{
	bool passed = teacut_compare(expectation.a,
								 expectation.operation,
								 expectation.b);
	if ( ! passed)
	{
		teacut_addExpectationFail(data);
		teacut_printExpectationFail(&expectation, data);
		if(expectation.isAssertion)
			exit(EXIT_FAILURE);
		else 
			return TEACUT_FAILURE;
	}
	return 0;
}
void teacut_addFailToParentAndGlobalDataWhenFailed(struct teacut_TestAndSuiteData* data)
{
	bool anyFails = data->expectationFails || data->testFails || data->suiteFails;
	if (anyFails && data->testDefined)
	{
		data->parent->testFails++;
		if (data->parent != &teacut_globalData)
			teacut_globalData.testFails++;
	}
	if (anyFails && data->suiteDefined)
	{
		data->parent->suiteFails++;
		if (data->parent != &teacut_globalData)
			teacut_globalData.suiteFails++;
	}
}

void teacut_printTestOrSuiteResult(struct teacut_TestAndSuiteData* data)
{
	const char* testOrSuite = data->testDefined ? "Test" : "Suite";
	const char* testOrSuiteName = data->testDefined ? data->testName : data->suiteName;

	if ( ! data->expectationFails && ! data->testFails && ! data->suiteFails)
	{
		printf("\n%s \"%s\" " TEACUT_GREEN("[PASSED]") " \n", testOrSuite, testOrSuiteName);
	}
	else
	{
		fprintf(stderr, "\n%s \"%s\" " TEACUT_RED("[FAILED]") " \n",
				testOrSuite, testOrSuiteName);
	}
}

#endif // TEACUT_IMPLEMENTATION

#ifdef __cplusplus
} // extern "C"
#endif

#endif // TEACUT_H

