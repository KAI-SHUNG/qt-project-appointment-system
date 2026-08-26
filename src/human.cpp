#include "human.h"

human::human(QString name, int age, Gender gender)
    :name(name), age(age), gender(gender)
{}

QString human::getName() const
{
    return name;
}

void human::setName(const QString &newName)
{
    name = newName;
}

int human::getAge() const
{
    return age;
}

void human::setAge(int newAge)
{
    if (age < 0){
        throw std::invalid_argument("年龄必须是正数！");
    }
    age = newAge;
}

Gender human::getGender() const
{
    return gender;
}

void human::setGender(Gender newGender)
{
    gender = newGender;
}
