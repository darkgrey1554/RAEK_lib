#include <iostream>
#include "libgate.h"

#define SIZEDATA 600

int main(int num, char* arg[])
{
    int num_channels = 0;
    int size_data = 0;

    if (num == 3)
    {
        num_channels = atoi(arg[1]);
        size_data = atoi(arg[2]);
    }
    else
    {
        std::cout << "BAD PARAMETRS" << std::endl;
        return 0;
    }

    if (0 > atoi(arg[1]) || atoi(arg[1]) > 3)
    {
        std::cout << "BAD PARAMETRS" << std::endl;
        return 0;
    }

    if (0 > atoi(arg[2]) || atoi(arg[2]) > 1000)
    {
        std::cout << "BAD PARAMETRS" << std::endl;
        return 0;
    }

    int status = 0;
    unsigned int res = 0;
    int error_init = 0;
    int error_system = 0;
    Gate_EMT_DTS* dts_gate = new Gate_EMT_DTS();
    if (dts_gate->GetStatusInit() != Status_Init::OK)
    {
        std::cout << "ERROR INITIAL GATE" << std::endl;
        std::cout << "error - "<< dts_gate->GetError() << std::endl;
        std::cout << "error_system - " << dts_gate->GetSystemError() << std::endl;
        Sleep(2000);
        return -1;
    }

    float* bufA = new float[num_channels*size_data];
    for (int i = 0; i < num_channels * size_data; i++) bufA[i] = i;
    int* bufD = new int[num_channels * size_data];
    for (int i = 0; i < num_channels * size_data; i++) bufD[i] = i;
    char* bufB = new char[num_channels * size_data];
    for (int i = 0; i < num_channels * size_data; i++) bufB[i] = 1;
    float step = 0.1;

    float* buf_Ain = new float[num_channels * size_data];
    int* buf_Din = new int[num_channels * size_data];
    char* buf_Bin = new char[num_channels * size_data];


    LARGE_INTEGER t1;
    LARGE_INTEGER t2;
    LARGE_INTEGER f;
    float time = 0;
    QueryPerformanceFrequency(&f);

    Sleep(4000);
    

    for (;;)
    {

        Sleep(500);

        for (int i = 0; i < num_channels * size_data; i++)
        {
            bufA[i] += step;
            bufD[i]++;
            bufB[i] = (~bufB[i]) & 1;
        }

        /*dts_gate->WriteAnalogData(bufA, 100);  
        dts_gate->WriteDiscreteData(bufD, 100);
        dts_gate->WriteBinarData(bufB, 100);

        dts_gate->ReadAnalogData(buf_Ain, 100);
        dts_gate->ReadDiscreteData(buf_Din, 100);
        dts_gate->ReadBinarData(buf_Bin, 100);
        */

        
        res=dts_gate->WriteData(TypeData::Analog, bufA, num_channels * size_data);
        res=dts_gate->WriteData(TypeData::Discrete, bufD, num_channels * size_data);
        res=dts_gate->WriteData(TypeData::Binar, bufB, num_channels * size_data);

        res=dts_gate->ReadData(TypeData::Analog, buf_Ain, num_channels * size_data);
        res=dts_gate->ReadData(TypeData::Discrete, buf_Din, num_channels * size_data);
        res=dts_gate->ReadData(TypeData::Binar, buf_Bin, num_channels * size_data);
      
        
        system("cls");
        std::cout << "ANALOG:\tDISCRETE:\tBINAR:" <<std::endl;

        for (int j = 0; j < 3; j++)
        {
            for (int i = 0; i < 2; i++)
            {
                std::cout << buf_Ain[size_data*j+i] << "\t" << buf_Din[size_data* j+i] << "\t" << (int)buf_Bin[size_data * j + i] << std::endl;
            }
            for (int i = 2; i > 0; i--)
            {
                std::cout << buf_Ain[size_data * j + size_data - 1 - i] << "\t" << buf_Din[size_data * j + size_data - 1 - i] << "\t" << (int)buf_Bin[size_data * j + size_data - 1 - i] << std::endl;
            }
            std::cout<<" -------------------- "<< std::endl;
        }



        if (res != 0)
        {
            std::cout << "WARNING ReadData: " << res << std::endl;
        }
        continue;

    }
             
}
