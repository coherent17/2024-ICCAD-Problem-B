#ifndef _PIN_H_
#define _PIN_H_

#include <iostream>
#include <string>


class Pin{
private:
    std::string pinName;
    bool isIOPin;
    std::string instanceName;

public:
    Pin();
    ~Pin();

    // setter
    void setPinName(const std::string &);
    void setIsIOPin(bool);
    void setInstanceName(const std::string &);

    // getter
    const std::string &getPinName()const;
    bool getIsIOPin()const;
    const std::string &getInstanceName()const;

    friend std::ostream &operator<<(std::ostream &os, const Pin &pin);
};

#endif