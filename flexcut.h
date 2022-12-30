/*
 * MIT License
 * 
 * Copyright (c) 2022 Lauri Lorenzo Fiestas
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//*************************************************************************************
//
//		USAGE OF THIS LIBRARY
//
//		To use this library define FLEXCUT_IMPLEMENTATION in EXACTLY ONE C/C++ file 
//		BEFORE including it as shown in the example below. Settings should also be 
//		defined similarly. 
//
//		Details on usage can be found in PUBLIC API section.
//
//		SETTINGS
//
//		// Use this flag if your terminal does not support color escape sequences
//		#define FLEXCUT_DISABLE_COLOR
//
//*************************************************************************************
/*

// usage example

#define FLEXCUT_IMPLEMENTATION
// define settings here
#include "flexcut.h"

*/

#ifndef FLEXCUT_H
#define FLEXCUT_H

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
#define TEST(NAME)			FCUT_TEST_OR_SUITE(NAME,test)
#define TEST_SUITE(NAME)	FCUT_TEST_OR_SUITE(NAME,suite)
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

#define FCUT_FAILURE 1

// Does nothing when expression is true.
// Exits program and prints failure message when expression is false.
// Assertions are counted as expectations.
// Really a macro.
void ASSERT(bool expression);

// Returns 0 when expression is true.
// Prints failure message and returns FCUT_FAILURE when expression is false.
// Really a macro.
int EXPECT(bool expression);

// 'Pseudo-operators' to be used in argument for ASSERT() or EXPECT().
// Use ASSERT(A EQ B) instead of ASSERT(A == B) for more info at failure!
#define EQ ,FCUT_EQ, // ==
#define NE ,FCUT_NE, // !=
#define GT ,FCUT_GT, // >
#define LT ,FCUT_LT, // <
#define GE ,FCUT_GE, // >=
#define LE ,FCUT_LE, // <=

//*************************************************************************************
//
//		END OF PUBLIC API
//
//		Structs, functions and macros below are not meant to be used by the user.
//		However, they are required for macros to work so here you go I guess.
//
//*************************************************************************************

#ifdef __cplusplus
#define ATOMIC(T) std::atomic<T>
#else
#define ATOMIC(T) _Atomic T
#endif

struct fcut_TestAndSuiteData
{
	const char *testName, *suiteName;
	ATOMIC(int) testFails, suiteFails, expectationFails/*includes assertion fails*/;
	ATOMIC(int) testCount, suiteCount, expectationCount;
	// testDefined has 'test' written in lowercase which is necessary for FCUT_TEST_OR_SUITE()
	const union {bool isTest;  bool testDefined;};
	const union {bool isSuite; bool suiteDefined;};
	bool testOrSuiteRunning;
	struct fcut_TestAndSuiteData* parent;
};
extern struct fcut_TestAndSuiteData fcut_globalData;

#undef ATOMIC

#define OP_TABLE	\
	X(_EQ, ==)		\
	X(_NE, !=)		\
	X(_GT, >)		\
	X(_LT, <)		\
	X(_GE, >=)		\
	X(_LE, <=)		\

enum fcut_BooleanOperator
{
	FCUT_NO_OP = -1,

#define X(OP, DUMMY) FCUT##OP,
	OP_TABLE
#undef X

// Expands to

/*	FCUT_EQ,
	FCUT_NE,
	FCUT_GT,
	FCUT_LT,
	FCUT_GE,
	FCUT_LE,*/

	FCUT_OPS_LENGTH
};

struct fcut_ExpectationData
{
	const double a, b;
	const char *str_a, *str_b, *str_operator, *func, *file;
	const enum fcut_BooleanOperator operation;
	const int line;
	const bool isAssertion;
};

// Boolean operations as a function
// Allows macros EQ, NE, etc. to be used like operators
bool fcut_compare(double expression_a, enum fcut_BooleanOperator, double expression_b);

void fcut_printStartingMessageAndInitExitMessage();

void fcut_printTestOrSuiteResult(struct fcut_TestAndSuiteData*);

void fcut_printExpectationFail(struct fcut_ExpectationData*, struct fcut_TestAndSuiteData*);

int fcut_assert(struct fcut_ExpectationData, struct fcut_TestAndSuiteData*);

bool fcut_testOrSuiteRunning(struct fcut_TestAndSuiteData*);

void fcut_printTestOrSuiteResult(struct fcut_TestAndSuiteData*);

void fcut_addTestOrSuiteFailToParentAndGlobalIfFailed(struct fcut_TestAndSuiteData*);

extern struct fcut_TestAndSuiteData *const fcut_currentTestOrSuite;

extern const char FCUT_STR_OPERATORS[FCUT_OPS_LENGTH][3];

#define FCUT_COMMON_DATA .line = __LINE__, .func = __func__, .file = __FILE__,

#define FCUT_EXPECT(EXP, IS_ASS) 						\
	fcut_assert											\
	(													\
		(struct fcut_ExpectationData)					\
		{												\
			.a 			 	= EXP,						\
			.str_a		 	= #EXP,						\
			.operation	 	= FCUT_NO_OP,				\
			.isAssertion 	= IS_ASS,					\
			FCUT_COMMON_DATA							\
		},												\
		fcut_currentTestOrSuite							\
	)

#define FCUT_EXPECT_CMP(A, OP, B, IS_ASS)				\
	fcut_assert											\
	(													\
	 	(struct fcut_ExpectationData)					\
		{												\
			.a 	   		  	= A,						\
			.b 			  	= B,						\
			.str_a 		  	= #A,						\
			.str_b 		  	= #B,						\
			.str_operator 	= FCUT_STR_OPERATORS[OP],	\
			.operation	  	= OP,						\
			.isAssertion  	= IS_ASS,					\
			FCUT_COMMON_DATA							\
		},												\
		fcut_currentTestOrSuite							\
	)

#define FCUT_NOT_ASS 0
#define FCUT_IS_ASS  1

// For overloading EXPECT() and ASSERT() on number of arguments
#define GET_MACRO_NAME(DUMMY1, DUMMY2, DUMMY3, NAME, ...) NAME

#define EXPECT(...)		\
	GET_MACRO_NAME(__VA_ARGS__,FCUT_EXPECT_CMP,DUMMY,FCUT_EXPECT)(__VA_ARGS__,FCUT_NOT_ASS)
	
#define ASSERT(...)		\
	GET_MACRO_NAME(__VA_ARGS__,FCUT_EXPECT_CMP,DUMMY,FCUT_EXPECT)(__VA_ARGS__,FCUT_IS_ASS)

#define FCUT_TEST_OR_SUITE(NAME, TEST_OR_SUITE)											\
	struct fcut_TestAndSuiteData fcut_##TEST_OR_SUITE##_##NAME =						\
	{																					\
		.TEST_OR_SUITE##Name = #NAME,													\
		.TEST_OR_SUITE##Defined = true,													\
		.parent = fcut_currentTestOrSuite												\
	};																					\
																						\
	for(struct fcut_TestAndSuiteData* fcut_currentTestOrSuite = &fcut_##TEST_OR_SUITE##_##NAME;\
		fcut_testOrSuiteRunning(fcut_currentTestOrSuite);)
/*	{
		// user defined test or suite code for for-loop
	}
*/

//*************************************************************************************
//
// 		IMPLEMENTATION
//
//*************************************************************************************

#ifdef FLEXCUT_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>

struct fcut_TestAndSuiteData fcut_globalData = {};
struct fcut_TestAndSuiteData *const fcut_currentTestOrSuite = &fcut_globalData;

#ifndef FLEXCUT_DISABLE_COLOR
// Appends desired color escape sequence in front and default color in back
#define FCUT_RED(STR_LITERAL)		"\033[0;31m"			STR_LITERAL "\033[0m"
#define FCUT_GREEN(STR_LITERAL)		"\033[0;92m"			STR_LITERAL "\033[0m"
#define FCUT_MAGENTA(STR_LITERAL)	"\033[0;95m"			STR_LITERAL "\033[0m"
#define FCUT_CYAN(STR_LITERAL)		"\033[0;96m"			STR_LITERAL "\033[0m"
#define FCUT_WHITE_BG(STR_LITERAL)	"\033[0;107m\033[30m"	STR_LITERAL "\033[0m"
#else
#define FCUT_RED(STR_LITERAL)		STR_LITERAL
#define FCUT_GREEN(STR_LITERAL)		STR_LITERAL
#define FCUT_MAGENTA(STR_LITERAL)	STR_LITERAL
#define FCUT_CYAN(STR_LITERAL)		STR_LITERAL
#define FCUT_WHITE_BG(STR_LITERAL)	STR_LITERAL
#endif // FLEXCUT_DISABLE_COLOR

#define PRINT_DATA(DATA)													\
	printf("A total of " FCUT_CYAN("%i") " " #DATA "s completed, ",			\
			fcut_globalData. DATA##Count );									\
	if (fcut_globalData. DATA##Fails)										\
		printf(FCUT_RED("%i failed")"\n", fcut_globalData. DATA##Fails);	\
	else																	\
		printf(FCUT_GREEN("%i failed")"\n", fcut_globalData. DATA##Fails);

bool fcut_anyFails(struct fcut_TestAndSuiteData* data)
{
	return data->expectationFails || data->testFails || data->suiteFails;
}

void fcut_printExitMessageAndAddExitStatus()
{
	printf("\n");

	PRINT_DATA(expectation);
	PRINT_DATA(test);
	PRINT_DATA(suite);

	if (fcut_anyFails(&fcut_globalData))
		exit(FCUT_FAILURE);
}

#undef PRINT_DATA

void fcut_printStartingMessageAndInitExitMessage()
{
	static bool initialized = false;
	if ( ! initialized)
	{
		printf("\n\tStarting tests...\n");
		atexit(fcut_printExitMessageAndAddExitStatus);
		initialized = true;
	}
}

//const char FCUT_STR_OPERATORS[FCUT_OPS_LENGTH][3] = {"==", "!=", ">", "<", ">=", "<="};
const char FCUT_STR_OPERATORS[FCUT_OPS_LENGTH][3] = {
#define X(DUMMY, OP) #OP,
	OP_TABLE
#undef X
};

bool fcut_compare(double a, int operation, double b)
{
	switch(operation)
	{
		case FCUT_NO_OP:
			return a;

	#define X(OP_ENUM, OP) 		\
		case FCUT##OP_ENUM:	\
			return a OP b;

		OP_TABLE

	#undef X

	// Expands to
	
	/*	case FCUT_EQ:
			return a == b;
		case FCUT_NE:
			return a != b;
		// etc...
	*/
	}

	return 0&&(a+b); // Gets rid of pointless compiler warnings
}

// Finds suite by going trough all parent data
struct fcut_TestAndSuiteData* findSuite(struct fcut_TestAndSuiteData* data)
{
	bool suiteFound 	= data->isSuite;
	bool suiteNotFound	= data == &fcut_globalData;

	if (suiteFound)
		return data;
	else if (suiteNotFound)
		return NULL;
	else // keep looking
		return findSuite(data->parent);
}

void fcut_printExpectationFail(struct fcut_ExpectationData* expectation,
								 struct fcut_TestAndSuiteData* data)
{
	const char* finalTestName = data->isTest  ? data->testName  :
								data->isSuite ? data->suiteName :
								expectation->func;

	if (expectation->isAssertion)
		fprintf(stderr, "\nAssertion ");
	else
		fprintf(stderr, "\nExpectation ");

	fprintf(stderr,
			"in \"%s\" " FCUT_RED("[FAILED]") " in \"%s\" " FCUT_WHITE_BG("line %i") "\n", 
			finalTestName, expectation->file, expectation->line);

	fprintf(stderr, FCUT_MAGENTA("%s"), expectation->str_a);
	if (expectation->operation != FCUT_NO_OP)
		fprintf(stderr, FCUT_MAGENTA(" %s %s"), expectation->str_operator, expectation->str_b);
	fprintf(stderr, " evaluated to " FCUT_RED("%g"), expectation->a);
	if (expectation->operation != FCUT_NO_OP)
		fprintf(stderr, FCUT_RED(" %s %g"), expectation->str_operator, expectation->b);
	fprintf(stderr, ".\n");

	if (expectation->isAssertion) // print test and suite results early before exiting
	{
		if (data->isTest)
			fcut_printTestOrSuiteResult(data);
		struct fcut_TestAndSuiteData* suite = findSuite(data);
		if (suite != NULL)
			fcut_printTestOrSuiteResult(suite);
	}
}

// Adds one fail to all parents all the way to fcut_globalData
void fcut_addExpectationFail(struct fcut_TestAndSuiteData* data)
{
	data->expectationFails++;
	if (data != &fcut_globalData)
		fcut_addExpectationFail(data->parent);
}

int fcut_assert(struct fcut_ExpectationData expectation,
				  struct fcut_TestAndSuiteData* data)
{
	fcut_globalData.expectationCount++;
	bool passed = fcut_compare(expectation.a,
								 expectation.operation,
								 expectation.b);
	if ( ! passed)
	{
		fcut_addExpectationFail(data);
		fcut_printExpectationFail(&expectation, data);
		if(expectation.isAssertion)
			exit(EXIT_FAILURE);
		else 
			return FCUT_FAILURE;
	}
	return 0;
}

bool fcut_testOrSuiteRunning(struct fcut_TestAndSuiteData* data)
{
	bool testOrSuiteHasRan = data->testOrSuiteRunning;

	if ( ! testOrSuiteHasRan)
	{
		fcut_printStartingMessageAndInitExitMessage();
	}
	else
	{
		if (data->isTest)
			fcut_globalData.testCount++;
		else
			fcut_globalData.suiteCount++;

		fcut_addTestOrSuiteFailToParentAndGlobalIfFailed(data);
		fcut_printTestOrSuiteResult(data);
	}

	return data->testOrSuiteRunning = ! testOrSuiteHasRan;
}

void fcut_addTestOrSuiteFailToParentAndGlobalIfFailed(struct fcut_TestAndSuiteData* data)
{
	bool anyFails = fcut_anyFails(data);
	if (anyFails && data->isTest)
	{
		data->parent->testFails++;
		if (data->parent != &fcut_globalData)
			fcut_globalData.testFails++;
	}
	if (anyFails && data->isSuite)
	{
		data->parent->suiteFails++;
		if (data->parent != &fcut_globalData)
			fcut_globalData.suiteFails++;
	}
}

void fcut_printTestOrSuiteResult(struct fcut_TestAndSuiteData* data)
{
	const char* testOrSuite = data->isTest ? "Test" : "Suite";
	const char* testOrSuiteName = data->isTest ? data->testName : data->suiteName;

	if ( ! data->expectationFails && ! data->testFails && ! data->suiteFails)
	{
		printf("\n%s \"%s\" " FCUT_GREEN("[PASSED]") " \n", testOrSuite, testOrSuiteName);
	}
	else
	{
		fprintf(stderr, "\n%s \"%s\" " FCUT_RED("[FAILED]") " \n",
				testOrSuite, testOrSuiteName);
	}
}

#endif // FLEXCUT_IMPLEMENTATION

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FLEXCUT_H

