#ifndef TEACUT_H
#define TEACUT_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define TEACUT_RED "\033[0;31m"
#define TEACUT_GREEN "\033[0;92m"
#define TEACUT_CYAN "\033[0;96m"
#define TEACUT_WHITE_BG "\033[0;107m\033[30m"
#define TEACUT_DEFAULT_COLOR "\033[0m"

int teacut_printColor(const char* color, const char* format, ...)
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

//-------------------------------------------------------

#define TEACUT_MAX_NAME_LENGTH 100

struct teacut_LinkedList
{
	void (*test)();
	char suite[TEACUT_MAX_NAME_LENGTH];
	void* nextObject;
};

void teacut_traverseList(struct teacut_LinkedList* object)
{
	static int calls = 0;
	if(calls++ > 1000000)
	{
		printf("Infinite recursion detected!");
		exit(1);
	}

	object->test();

	if(object->nextObject == NULL)
	{
		return;
	}

	teacut_traverseList( (struct teacut_LinkedList*)object->nextObject );
}

#define TEST_SUITE(NAME) strncpy(teacut_currentSuite, #NAME, TEACUT_MAX_NAME_LENGTH);

#define TEST_FUNCTION(FUNCTION, CODE)								\
int FUNCTION														\
{																	\
	printf("\n\nStarting tests\n\n\n");								\
	struct teacut_LinkedList* teacut_lastObject;					\
	char teacut_currentSuite[TEACUT_MAX_NAME_LENGTH];				\
	void teacut_doNothing() {}										\
	struct teacut_LinkedList teacut_firstObject;					\
	teacut_firstObject.test = teacut_doNothing; 					\
	teacut_lastObject = &teacut_firstObject; 						\
	int teacut_totalFailures = 0;									\
																	\
	TEST_SUITE(teacut_globalSuite)									\
																	\
	CODE															\
																	\
	teacut_traverseList(&teacut_firstObject);						\
																	\
	if( ! teacut_totalFailures)										\
	{																\
		teacut_printColor(TEACUT_GREEN, "\nAll tests [PASSED] in ");\
		teacut_printColor(TEACUT_GREEN, #FUNCTION);					\
		printf("\n\n");												\
		return 0;													\
	}																\
	else															\
	{																\
		teacut_printColor(TEACUT_RED, "\nTests [FAILED] in ");		\
		teacut_printColor(TEACUT_RED, #FUNCTION);					\
		printf("\n\n");												\
		return 1;													\
	}																\
}

#define TEST(NAME, CODE)												\
	void NAME()															\
	{																	\
		printf("Test %s ", #NAME);										\
		int teacut_failuresPerTest = 0;									\
		const char* testName = #NAME;									\
																		\
		/*Get rid of pointless compiler warning of unused variables*/	\
		teacut_failuresPerTest = teacut_failuresPerTest*(*testName);	\
																		\
		CODE															\
																		\
		if( ! teacut_failuresPerTest)									\
			teacut_printColor(TEACUT_GREEN, "[PASSED]\n");				\
		else															\
			teacut_totalFailures += teacut_failuresPerTest;				\
		/* Failure messgaes are in teacut_assert() */					\
	}																	\
	struct teacut_LinkedList teacut_##NAME; 							\
	teacut_##NAME.test 	    	 	= NAME; 							\
	strcpy(teacut_##NAME.suite,       teacut_currentSuite);				\
	teacut_##NAME.nextObject 		= NULL;								\
	teacut_lastObject->nextObject 	= (void*)&teacut_##NAME; 			\
	teacut_lastObject 				= &teacut_##NAME;

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
// Allows macros EQ, NE, etc. to be used as operators
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

	return 0&&(a+b); // Gets rid of compiler warnings
}

struct teacut_expectationData
{
	double a, b;
	const char *str_a, *str_b, *str_operator, *testName;
	enum teacut_BooleanOperators operation;
	int line;
	bool isAssertion;
};

#define TEACUT_ASSERT(ASS) 					\
	teacut_failuresPerTest += teacut_assert				\
	(										\
		(struct teacut_expectationData)		\
		{									\
			.a 			 = ASS,				\
			.str_a		 = #ASS,			\
			.operation	 = TEACUT_NO_OP,	\
			.line 		 = __LINE__,		\
			.isAssertion = true,			\
			.testName	 = testName			\
		}									\
	)

#define TEACUT_ASSERT_CMP(A, OP, B) 		\
	teacut_failuresPerTest += teacut_assert				\
	(										\
	 	(struct teacut_expectationData)		\
		{									\
			.a 	   			= A,			\
			.b 				= B,			\
			.str_a 			= #A,			\
			.str_b 			= #B,			\
			.str_operator 	= #OP,			\
			.operation		= OP,			\
			.line 			= __LINE__,		\
			.isAssertion	= true,			\
			.testName		= testName		\
		}									\
	)

#define GET_MACRO_NAME(DUMMY1, DUMMY2, SUMMY3, NAME, ...) NAME
#define ASSERT(...)		\
	GET_MACRO_NAME(__VA_ARGS__, TEACUT_ASSERT_CMP, DUMMY, TEACUT_ASSERT)(__VA_ARGS__)

#define TEACUT_EXPECT(EXP) 					\
	teacut_failuresPerTest += teacut_assert				\
	(										\
		(struct teacut_expectationData)		\
		{									\
			.a 			 = EXP,				\
			.str_a		 = #EXP,			\
			.operation	 = TEACUT_NO_OP,	\
			.line 		 = __LINE__,		\
			.isAssertion = false,			\
			.testName	 = testName			\
		}									\
	)

#define TEACUT_EXPECT_CMP(A, OP, B) 		\
	teacut_failuresPerTest += teacut_assert				\
	(										\
	 	(struct teacut_expectationData)		\
		{									\
			.a 	   		  = A,				\
			.b 			  = B,				\
			.str_a 		  = #A,				\
			.str_b 		  = #B,				\
			.str_operator = #OP,			\
			.operation	  = OP,				\
			.line 		  = __LINE__,		\
			.isAssertion  = false,			\
			.testName	  = testName		\
		}									\
	)

#define GET_MACRO_NAME(DUMMY1, DUMMY2, SUMMY3, NAME, ...) NAME
#define EXPECT(...)		\
	GET_MACRO_NAME(__VA_ARGS__, TEACUT_EXPECT_CMP, DUMMY, TEACUT_EXPECT)(__VA_ARGS__)

#define TEACUT_FAILURE 1

int teacut_assert(struct teacut_expectationData expectation)
{
	bool passed = teacut_compare(expectation.a,
								 expectation.operation,
								 expectation.b);

	/* Passing messages are in TEST() macro */

	if( ! passed)
	{
		teacut_printColor(TEACUT_RED, "[FAILED]\n");

		if(expectation.isAssertion)
			exit(EXIT_FAILURE);
		else 
			return TEACUT_FAILURE;
	}

	return 0;
}

#endif //TEACUT_H

