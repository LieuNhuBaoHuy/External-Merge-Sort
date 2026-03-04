#include "pch.h"
#include "All_Purpose.h"

void get_testing_sample(unsigned int remainNum, const char* outFileName)
{
    std::ofstream file(outFileName, std::ios::binary);
    if (!file) return;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<double> dis(MinRandomLeft, MaxRandomRight);

    while (remainNum > 0)
    {
        DataType tempval = dis(gen);
        file.write(reinterpret_cast<char*>(&tempval), sizeof(tempval));
        --remainNum;
    }

    file.close();
}