#include "qdebug.h"
#include "src/human.h"

void testAge(){
    try{
        Human man("人", -1, Gender::Male);
    }
    catch(std::exception& e){
        qDebug("%s", e.what());
    }
}

void testHuman(){
    qDebug("=== Testing Human ===");
    testAge();
    Human man("张三", 25, Gender::Male);
    qDebug() << man.getName() << ' ' << man.getAge();

    qDebug("=== Human Test Passed ===");
}