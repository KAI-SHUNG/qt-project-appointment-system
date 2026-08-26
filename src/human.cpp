#include "human.h"

Human::Human(QString name, int age, Gender gender)
    :name(name), age(age), gender(gender)
{}

QString Human::getName() const
{
    return name;
}

void Human::setName(const QString &newName)
{
    name = newName;
}

int Human::getAge() const
{
    return age;
}

void Human::setAge(int newAge)
{
    if (age < 0){
        throw std::invalid_argument("年龄必须是正数！");
    }
    age = newAge;
}

Gender Human::getGender() const
{
    return gender;
}

void Human::setGender(Gender newGender)
{
    gender = newGender;
}
