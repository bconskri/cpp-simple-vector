#include "simple_vector.h"

// Tests
#include "tests.h"

int main() {
//    Test1();
//    Test2();
//    Test3();
//    Test4();
//    TestReserveConstructor();
//    TestReserveMethod();

    TestTemporaryObjConstructor();
    TestTemporaryObjOperator();
    TestNamedMoveConstructor();
    TestNamedMoveOperator();
    TestNoncopiableMoveConstructor();
    TestNoncopiablePushBack();
    TestNoncopiableInsert();
    TestNoncopiableErase();

    puts("Good");
    return 0;
}