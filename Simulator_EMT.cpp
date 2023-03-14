#include <iostream>
#include "libgate.h"

int main(int num, char* arg[])
{
    
    int size_data_analog_in = 0;
    int size_data_binar_in = 0;
    int size_data_discrete_in = 0;
    int size_data_analog_out = 0;
    int size_data_binar_out = 0;
    int size_data_discrete_out = 0;

    if (num == 7)
    {
        size_data_analog_out = atoi(arg[1]);
        size_data_binar_out = atoi(arg[2]);
        size_data_discrete_out = atoi(arg[3]);
        size_data_analog_in = atoi(arg[4]);
        size_data_binar_in = atoi(arg[5]);
        size_data_discrete_in = atoi(arg[6]);
    }
    else
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
        std::cout << "error - " << dts_gate->GetError() << std::endl;
        std::cout << "error_system - " << dts_gate->GetSystemError() << std::endl;
        Sleep(2000);
        return -1;
    }

    float* bufA_out = new float[size_data_analog_out];
    for (int i = 0; i < size_data_analog_out; i++) bufA_out[i] = i;
    int* bufD_out = new int[size_data_discrete_out];
    for (int i = 0; i < size_data_discrete_out; i++) bufD_out[i] = i;
    char* bufB_out = new char[size_data_binar_out];
    for (int i = 0; i < size_data_binar_out; i++) bufB_out[i] = 1;

    float* bufA_in = new float[size_data_analog_in];
    int* bufD_in = new int[size_data_discrete_in];
    char* bufB_in = new char[size_data_binar_in];


    LARGE_INTEGER t1;
    LARGE_INTEGER t2;
    LARGE_INTEGER f;
    float time = 0;
    QueryPerformanceFrequency(&f);

    Sleep(1000);


    for (;;)
    {

        Sleep(500);

        for (int i = 0; i < size_data_analog_out; i++)
        {
            bufA_out[i] += 0.1;
        }
        for (int i = 0; i < size_data_discrete_out; i++)
        {
            bufD_out[i]++;
        }
        for (int i = 0; i < size_data_binar_out; i++)
        {
            bufB_out[i] = (~bufB_out[i]) & 1;
        }

        system("cls");

        res = dts_gate->WriteData(TypeData::Analog, bufA_out, size_data_analog_out);
        if (res != 0)
        {
            std::cout << "WARNING WriteData: " << res << std::endl;
        }
        res = dts_gate->WriteData(TypeData::Discrete, bufD_out, size_data_discrete_out);
        if (res != 0)
        {
            std::cout << "WARNING WriteData: " << res << std::endl;
        }
        res = dts_gate->WriteData(TypeData::Binar, bufB_out, size_data_binar_out);
        if (res != 0)
        {
            std::cout << "WARNING WriteData: " << res << std::endl;
        }
        

        res = dts_gate->ReadData(TypeData::Analog, bufA_in, size_data_analog_in);
        if (res != 0)
        {
            std::cout << "WARNING ReadData: " << res << std::endl;
        }
        res = dts_gate->ReadData(TypeData::Discrete, bufD_in, size_data_discrete_in);
        if (res != 0)
        {
            std::cout << "WARNING ReadData: " << res << std::endl;
        }
        res = dts_gate->ReadData(TypeData::Binar, bufB_in, size_data_binar_out);
        if (res != 0)
        {
            std::cout << "WARNING ReadData: " << res << std::endl;
        }


        std::cout << "\tANALOG:\tDISCRETE:\tBINAR:" << std::endl;
        std::cout << "OUT_DATA:" << std::endl;

        for (int i = 0; i < 2; i++)
        {
            std::cout << "["<<i<< "]:\t"
                << bufA_out[i] << "\t"
                << bufD_out[i] << "\t"
                << (int)bufB_out[i] << std::endl;
        }
        for (int i = 0; i < 2; i++)
        {
            std::cout << "[" << (i > 0 ? "l" : "l-1") << "]\t"
                << bufA_out[size_data_analog_out - 2 + i] << "\t"
                << bufD_out[size_data_discrete_out - 2 + i] << "\t"
                << (int)bufB_out[size_data_binar_out - 2 + i] << std::endl;
        }

    std::cout << "size A: " << size_data_analog_out << "\tD : " << size_data_discrete_out << "\tB : " << size_data_binar_out << std::endl;

        std::cout << "IN_DATA:" << std::endl;

        for (int i = 0; i < 2; i++)
        {
            std::cout << "[" << i << "]:\t"
                << bufA_in[i] << "\t"
                << bufD_in[i] << "\t"
                << (int)bufB_in[i] << std::endl;
        }
        for (int i = 0; i < 2; i++)
        {
            std::cout << "[" << (i > 0 ? "l" : "l-1") << "]\t"
                << bufA_in[size_data_analog_in - 1 - i] << "\t"
                << bufD_in[size_data_discrete_in - 1 - i] << "\t"
                << (int)bufB_in[size_data_binar_in - 1 - i] << std::endl;
        }

        std::cout << "size A: " << size_data_analog_in << "\tD : " << size_data_discrete_in << "\tB : " << size_data_binar_in << std::endl;

        continue;
    }

}
