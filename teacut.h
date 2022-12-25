#ifndef TEACUT_H
#define TEACUT_H

#ifdef __cplusplus
#include <atomic>
extern "C" {
#else
#include <stdatomic.h>
#endif

#include <stdbool.h>

#define TEST(NAME)			TEACUT_TEST_OR_SUITE(NAME,test)
#define TEST_SUITE(NAME)	TEACUT_TEST_OR_SUITE(NAME,suite)

#define TEACUT_RED(STR_LITERAL)			"\033[0;31m"			STR_LITERAL "\033[0m"
#define TEACUT_GREEN(STR_LITERAL)		"\033[0;92m"			STR_LITERAL "\033[0m"
#define TEACUT_MAGENTA(STR_LITERAL)		"\033[0;95m"			STR_LITERAL "\033[0m"
#define TEACUT_CYAN(STR_LITERAL)		"\033[0;96m"			STR_LITERAL "\033[0m"
#define TEACUT_WHITE_BG(STR_LITERAL)	"\033[0;107m\033[30m"	STR_LITERAL "\033[0m"

struct teacut_TestAndSuiteData
{
#ifdef __cplusplus
	std::atomic<int> testFails, suiteFails, expectationFails;
#else
	_Atomic int testFails, suiteFails, expectationFails;
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

enum teacut_BooleanOperators
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
	enum teacut_BooleanOperators operation;
	int line;
	bool isAssertion;
};

// Does nothing when expression is true
// Exits program and prints failure message when expression is false
void ASSERT(bool expression);

// Returns 0 when expression is true
// Prints failure message and returns TEACUT_FAILURE==1 when expression is false
int EXPECT(bool expression);

#define TEACUT_FAILURE 1

// Boolean operations as a function
// Allows macros EQ, NE, etc. to be used like operators
bool teacut_compare(double a, int operation, double b);

#define EQ ,TEACUT_EQ,
#define NE ,TEACUT_NE,
#define GT ,TEACUT_GT,
#define LT ,TEACUT_LT,
#define GE ,TEACUT_GE,
#define LE ,TEACUT_LE,

void teacut_printTestOrSuiteResult(struct teacut_TestAndSuiteData*);

void teacut_printFailMessage(struct teacut_ExpectationData*, struct teacut_TestAndSuiteData*);

int teacut_assert(struct teacut_ExpectationData, struct teacut_TestAndSuiteData*);

void teacut_printTestOrSuiteResult(struct teacut_TestAndSuiteData*);

void teacut_updateParentTestOrSuite(struct teacut_TestAndSuiteData*, int newFails);

extern struct teacut_TestAndSuiteData *const teacut_shadow;
extern struct teacut_TestAndSuiteData *const teacut_dummy;
extern const char TEACUT_STR_OPERATORS[TEACUT_OPS_LENGTH][3];

#define TEACUT_DATA (teacut_shadow->testDefined || teacut_shadow->suiteDefined ?	\
					 teacut_shadow : teacut_dummy)

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
		TEACUT_DATA							\
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
		TEACUT_DATA										\
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
		TEACUT_DATA							\
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
		TEACUT_DATA										\
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
		teacut_updateParentTestOrSuite(&teacut_##TEST_OR_SUITE,						\
									   teacut_##TEST_OR_SUITE.expectationFails);	\
		teacut_printTestOrSuiteResult(&teacut_##TEST_OR_SUITE);						\
	}																				\
	void teacut_##TEST_OR_SUITE##_##NAME (struct teacut_TestAndSuiteData* teacut_shadow)

//*************************************************************************************
//
//		IMPLEMENTATION
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

void teacut_printFailMessage(struct teacut_ExpectationData* expectation,
							 struct teacut_TestAndSuiteData* data)
{
	const char* finalTestName = data->testDefined  ? data->testName  :
								data->suiteDefined ? data->suiteName :
								expectation->func;

	if (expectation->isAssertion)
		fprintf(stderr, "\nAssertion ");
	else
		fprintf(stderr, "\nExpectation ");
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

	// MUISTA KORJAA
/*	if (expectation->isAssertion && teacut_data.testDefined)
		teacut_printResult("Test", teacut_data.testName, teacut_data.testErrors);
	if (expectation->isAssertion && teacut_data.suiteDefined)
		teacut_printResult("Suite", teacut_data.suiteName, teacut_data.suiteErrors);*/
}

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
	//	data->expectationFails++;
		teacut_addExpectationFail(data);
		teacut_printFailMessage(&expectation, data);
		if(expectation.isAssertion)
			exit(EXIT_FAILURE);
		else 
			return TEACUT_FAILURE;
	}
	return 0;
}

void teacut_updateParentTestOrSuite(struct teacut_TestAndSuiteData* data, int newFails)
{
	if ( ! newFails) // tarviiks?
	{
		return;
	}
	//if (data->expectationFails)
	else
	{
		//printf("newFails: %i, data.fails: %i\n", newFails, data->expectationFails);
		//data->parent->expectationFails 	+= newFails;
		data->parent->testFails 		+= (int)data->parent->testDefined;
		data->parent->suiteFails		+= (int)data->parent->suiteDefined;
	}

	if (data->parent->testDefined || data->parent->suiteDefined)
		teacut_updateParentTestOrSuite(data->parent, newFails);
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

