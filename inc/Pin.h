#ifndef _PIN_H_
#define _PIN_H_

#include <iostream>
#include <string>
using namespace std;

class Pin{
private:
    string pinName;
    bool isIOPin;
    string instanceName;

public:
    Pin();
    ~Pin();

    // setter
    void setPinName(const string &);
    void setIsIOPin(bool);
    void setInstanceName(const string &);

    // getter
    const string &getPinName()const;
    bool getIsIOPin()const;
    const string &getInstanceName()const;

    friend ostream &operator<<(ostream &, const Pin &);
};

#endif