#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>

#include "cunit.h" 

TEST(Asert_True_Success) {
    ASSERT_TRUE(1);
}

TEST(Assert_True_Fail) {
    ASSERT_TRUE(0);
}

TEST(Assert_False_Success) {
    ASSERT_FALSE(false);
}

TEST(Assert_False_Fail) {
    ASSERT_FALSE(true);
}

TEST(Int_Equals_Success) {
    ASSERT_EQ_INT(67,67);
}

TEST(Int_Equals_Fail) {
    ASSERT_EQ_INT(69,420);
} 

TEST(Float_Equals_Success) {
    ASSERT_EQ_FLOAT(1.0f,1.01f,1e-2);
}

TEST(Float_Equals_Fail) {
    ASSERT_EQ_FLOAT(1.0f,1.01f,1e-3);
}

TEST(String_Equals_Success) {
    ASSERT_EQ_STR("abcdf", "abcdf");
}

TEST(String_Equals_Fail) {
    ASSERT_EQ_STR("abcdf", "abc");
}

TEST(Empty_Strings_Equal_Success) {
    ASSERT_EQ_STR("","");
}

typedef struct {
    int field1;
    char field2;

} user_type;

int user_type_comparer(user_type* a, user_type* b) {
    if ( (a->field1 == b->field1) && (a->field2 == b->field2) ) return true;
    return false;
}

const char* user_type_string(user_type* a, user_type* b) {
    char* str = malloc(64 * sizeof(char));
    sprintf(str,"\n expected - {%d, %c}, actual - {%d, %c}", a->field1, a->field2, b->field1, b->field2);
    return str;
}

TEST(Custom_Type_Equals_Success) {

    user_type expected = {1, 'x'};
    user_type actual   = {1, 'x'};

    const char* str = user_type_string(&expected, &actual);
    
    ASSERT_EQ_CUSTOM(&expected, &actual, user_type_comparer, str);
}

TEST(Custom_Type_Equals_Fail) {

    user_type expected = {69, 'x'};
    user_type actual   = {420, 'x'};

    const char* str = user_type_string(&expected, &actual);

    ASSERT_EQ_CUSTOM(&expected, &actual, user_type_comparer, str);
}

TEST(Ptr_Null_Success) {
    int i = 77;
    int* pi = &i;
    pi = NULL;
    ASSERT_NULL(pi);
}

TEST(Ptr_Null_Fail) {
    int i = 77;
    int* pi = &i;
    ASSERT_NULL(pi);
}

TEST(Ptr_Not_Null_Success) {
    int i = 77;
    int* pi = &i;
    ASSERT_NOT_NULL(pi);
}

TEST(Ptr_Not_Null_Fail) {
    int i = 77;
    int* pi = &i;
    pi = NULL;
    ASSERT_NOT_NULL(pi);

}
TEST(Fails_With_Segmentaion_Fault) {
	int* volatile p = NULL;
	*p = 42;
	ASSERT_EQ_INT(42, *p);
}

TEST(Fails_With_Signal_Abort) {
    raise(SIGABRT);
}

TEST(Fails_With_Signal_Terminate) {
    raise(SIGTERM);
}

int main(void) {
    run_tests();
    return 0;
}

