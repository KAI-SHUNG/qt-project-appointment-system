#ifndef HUMAN_H
#define HUMAN_H

#include <QObject>

enum class Gender {Male, Female};

class Human
{
public:
    Human(){}
    Human(QString, int, Gender);

    QString getName() const;
    void setName(const QString &newName);
    int getAge() const;
    void setAge(int newAge);
    Gender getGender() const;
    void setGender(Gender newGender);

private:
    QString name;
    int age;
    Gender gender;
};

#endif // HUMAN_H
