#include <Arduino.h>

#ifndef PasswordAccess_hpp
#define PasswordAccess_hpp

using namespace std;

#define LENGHT 6

class PasswordAccess
{
private:
    uint8_t access_pattern[LENGHT] { 7, 9, 8, 9, 8, 7 };
    uint8_t user_input[LENGHT] { 0 };

public:
    bool inputCode(uint8_t number)
    {
        for (uint8_t index = LENGHT - 1; index > 0; index--)
            user_input[index] = user_input[index - 1];
        user_input[0] = number;

        if (equal(begin(access_pattern), end(access_pattern), begin(user_input)))
        {
            user_input[0] = 0;
            return true;
        }
        return false;
    }
};

#endif