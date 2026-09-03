// 声明在各自 test_*.cpp 中定义的测试函数（避免与独立编译冲突产生多重定义）
void testTimeslot();
void testHuman();
void testDoctor();
void testPatient();
void testHospital();

int main()
{
    testTimeslot();
    testHuman();
    testDoctor();
    testPatient();
    testHospital();
}
