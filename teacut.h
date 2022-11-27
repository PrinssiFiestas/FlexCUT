#ifndef TEACUT_H
#define TEACUT_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define TEACUT_PRINT_RED(...) 			\
	printf("\033[0;31m"); 				\
	printf(__VA_ARGS__); 				\
	printf("\033[0m")

#define TEACUT_PRINT_GREEN(...) 		\
	printf("\033[0;92m"); 				\
	printf(__VA_ARGS__); 				\
	printf("\033[0m")

#define TEACUT_PRINT_CYAN(...) 			\
	printf("\033[0;96m"); 				\
	printf(__VA_ARGS__); 				\
	printf("\033[0m")

#define TEACUT_PRINT_WHITE_BG(...) 		\
	printf("\033[0;107m"); 				\
	printf("\033[30m"); 				\
	printf(__VA_ARGS__); 				\
	printf("\033[0m")

//-------------------------------------------------------

#define MAX_SUITE_NAME_LENGTH 100

struct teacut_LinkedList
{
	void (*test)();
	char suite[MAX_SUITE_NAME_LENGTH];
	void* nextObject;
};

void teacut_traverseList(struct teacut_LinkedList* object)
{
	object->test();

	if(object->nextObject == NULL)
	{
		TEACUT_PRINT_GREEN("\nSuite %s [PASSED]\n", object->suite);
		return;
	}

	char nextSuite[MAX_SUITE_NAME_LENGTH];
	strcpy(nextSuite, ((struct teacut_LinkedList*)(object->nextObject))->suite);

	bool changedSuite =  strcmp(object->suite, nextSuite);
	bool firstSuite   = !strcmp(object->suite, "teacut_globalSuite");

	if( (!firstSuite) && changedSuite)
	{
		TEACUT_PRINT_GREEN("\nSuite %s [PASSED]\n\n", object->suite);
	}

	if(changedSuite)
		printf("Starting suite %s\n\n", nextSuite);

	teacut_traverseList( (struct teacut_LinkedList*)object->nextObject );
}

#define TEST_SUITE(NAME) strncpy(teacut_currentSuite, #NAME, MAX_SUITE_NAME_LENGTH);

#define TEST_FUNCTION(FUNCTION, CODE)						\
int FUNCTION												\
{															\
	printf("\n\nStarting tests\n\n\n");						\
	struct teacut_LinkedList* teacut_lastObject;			\
	char teacut_currentSuite[MAX_SUITE_NAME_LENGTH]; 		\
	void teacut_doNothing() {}								\
	struct teacut_LinkedList teacut_firstObject;			\
	teacut_firstObject.test = teacut_doNothing; 			\
	strcpy(teacut_firstObject.suite, "teacut_globalSuite");	\
	teacut_lastObject = &teacut_firstObject; 				\
															\
	TEST_SUITE(teacut_globalSuite)							\
															\
	CODE													\
															\
	teacut_traverseList(&teacut_firstObject);				\
	TEACUT_PRINT_GREEN("\nAll tests [PASSED]\n\n");			\
	return 0;												\
}

#define TEST(NAME, CODE)										\
	void NAME()													\
	{															\
		printf("Test %s ", #NAME); 								\
																\
		CODE													\
																\
		TEACUT_PRINT_GREEN("[PASSED]\n");						\
	}															\
	struct teacut_LinkedList teacut_##NAME; 					\
	teacut_##NAME.test 	    	 	= NAME; 					\
	strcpy(teacut_##NAME.suite, 	  teacut_currentSuite); 	\
	teacut_##NAME.nextObject 		= NULL;						\
	teacut_lastObject->nextObject 	= (void*)&teacut_##NAME; 	\
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
	#define X(OP_ENUM, OP) 		\
		case TEACUT##OP_ENUM:	\
			return a OP b;

		OP_TABLE

	#undef X
	}

	// Evaluates to
	
	/*switch(operation)
	{
		case TEACUT_EQ:
			return a == b;
		case TEACUT_NE:
			return a != b;
		// etc...
	}*/

	return 0&&(a+b); // Gets rid of compiler warnings
}

#define TEACUT_ASSERT(ASS) teacut_assert(ASS, #ASS, __LINE__)

#define TEACUT_ASSERT_CMP(A, OP, B) 										\
	teacut_assertComparasion(OP, #A, #B, A, B, TEACUT_STR_OPERATORS[OP], __LINE__)

#define GET_FUNCTION_NAME(DUMMY1, DUMMY2, DUMMY3, NAME, ...) NAME
#define ASSERT(...) 														\
	GET_FUNCTION_NAME(__VA_ARGS__, TEACUT_ASSERT_CMP, DUMMY, TEACUT_ASSERT)(__VA_ARGS__)

void teacut_assert(bool assertion, const char* str_ass, int line)
{
	if( ! assertion)
	{
		TEACUT_PRINT_RED("[FAILED]");
		printf("\n\n  Expected ");
		TEACUT_PRINT_CYAN("%s", str_ass);
		printf(" at ");
		TEACUT_PRINT_WHITE_BG("line %i", line);
		printf("\n\n");
		exit(1);
	}
}

void teacut_assertComparasion(enum teacut_BooleanOperators op,
							  const char* str_a, const char* str_b,
							  double a, double b, const char* str_operator, int line)
{
	bool assertion = teacut_compare(a, op, b);
	if( ! assertion)
	{
		TEACUT_PRINT_RED("[FAILED]\n\n");
		TEACUT_PRINT_CYAN("  %s %s %s", str_a, str_operator, str_b);
		printf(" evaluated to ");
		TEACUT_PRINT_CYAN("%g %s %g", a, str_operator, b);
		printf(" at ");
		TEACUT_PRINT_WHITE_BG("line %i", line);
		printf("\n\n");
		exit(1);
	}
}

#endif //TEACUT_H

