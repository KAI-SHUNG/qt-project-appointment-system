#ifndef HUMAN_H
#define HUMAN_H

#include <QDebug>
#include <QObject>

class Human {
public:
    enum class Gender {
        Male,
        Female
    };

    Human()
    {
    }

    Human(QString, int, Gender);

    QString getName() const;
    void    setName(const QString& newName);
    int     getAge() const;
    void    setAge(int newAge);
    Gender  getGender() const;
    void    setGender(Gender newGender);

private:
    QString name;
    int     age;
    Gender  gender;
};

#endif // HUMAN_H
