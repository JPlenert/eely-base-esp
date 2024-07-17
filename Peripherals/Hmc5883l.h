// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef Hmc5883l_h
#define Hmc5883l_h

#include "I2c.h"

class Hmc5883l
{
    public:
        int x;
        int y;
        int z;
        
    private:
        I2c &_i2c;
        int _chipAddr;
        bool _isQmc;

    public:
        Hmc5883l(I2c &i2c);

        void SetGain(int gain);

        void Init();
        void Read();
};

#endif