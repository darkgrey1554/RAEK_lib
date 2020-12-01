#include "libgate.h"

std::string CreateNameMutexMemory(TypeData TD, TypeValue TV, int channel)
{
    std::string str;
    str += "Global\\MUTEX_";
    if (TD == TypeData::Analog)
    {
        str += "A";
    }
    else if (TD == TypeData::Discrete)
    {
        str += "D";
    }
    else if (TD == TypeData::Binar)
    {
        str += "B";
    }

    if (TV == TypeValue::INPUT)
    {
        str += "IN";
    }
    else if (TV == TypeValue::INPUT)
    {
        str += "OUT";
    }

    str += "_CHANNEL_" + std::to_string(channel);
    return str;
}

std::string CreateNameMemory(TypeData TD, TypeValue TV, int channel)
{
    std::string str;
    str += "Global\\MAPFILE_";
    if (TD == TypeData::Analog)
    {
        str += "A";
    }
    else if (TD == TypeData::Discrete)
    {
        str += "D";
    }
    else if (TD == TypeData::Binar)
    {
        str += "B";
    }

    if (TV == TypeValue::INPUT)
    {
        str += "IN";
    }
    else if (TV == TypeValue::INPUT)
    {
        str += "OUT";
    }

    str += "_CHANNEL_" + std::to_string(channel);
    return str;
}

/// --- инициализация дескриптора безопасности ---///
/*
 0 - ошибка AllocateAndInitializeSid
 1 - SetEntriesInAclA
 2 - LocalAlloc
 3 - InitializeSecurityDescriptor
 4 - SetSecurityDescriptorDacl
*/
unsigned int SecurityHandle::InitSecurityAttrubuts()
{
    std::string messeng;
    unsigned result = 0;
    DWORD res = 0;

    if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSID))
    {
        lasterror = GetLastError();
        result |= 1;
        return result;
    }

    ZeroMemory(&ea, 1 * sizeof(EXPLICIT_ACCESS));
    ea.grfAccessPermissions = KEY_ALL_ACCESS | MUTEX_ALL_ACCESS;
    ea.grfAccessMode = SET_ACCESS;
    ea.grfInheritance = NO_INHERITANCE;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea.Trustee.ptstrName = (LPTSTR)pEveryoneSID;

    res = SetEntriesInAclA(1, (PEXPLICIT_ACCESSA)&ea, NULL, &pACL);
    if (res != ERROR_SUCCESS)
    {
        lasterror = GetLastError();
        result |= 2;
        return result;
    }

    pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (pSD == NULL)
    {
        lasterror = GetLastError();
        result |= 4;
        return result;
    }

    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
    {
        lasterror = GetLastError();
        result |= 8;
        return result;
    }

    if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE))
    {
        lasterror = GetLastError();
        result |= 16;
        return result;
    }

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = pSD;
    sa.bInheritHandle = FALSE;

    return result;
}

DWORD SecurityHandle::getlasterror()
{
    return lasterror;
}

SECURITY_ATTRIBUTES& SecurityHandle::getsecurityattrebut()
{
    return sa;
}

GateDTS::GateDTS()
{
    security = new SecurityHandle();
    security->InitSecurityAttrubuts();
}

/// --- функция чтения списка KKSOut --- /// 
/*
0 - ошибка открытия файла
1 - ошибка в формате данных KKS
2 - ошибка типа данных KKS
3 - повтрный индекс KKS
*/
unsigned int GateDTS::ReadListKKSIn()
{
    FILE* config_file = NULL;
    char simvol = 0;
    std::string str_info;
    std::string helpstr;
    int res_read = 0;
    int pos[2] = { 0,0 };
    int count = 0;
    char status = 0;
    unsigned int result = 0;
    auto iter = VectKKSIn.begin();

    KKS_RAEK KKS;
    char flag_err_type = 0;

    config_file = fopen(NameFileListKKSIn, "r");
    if (config_file == NULL)
    {
        result |= 1;
        return result;
    }

    for (;;)
    {
        simvol = 0;
        str_info.clear();
        while (simvol != '\n' && res_read != EOF)
        {
            res_read = fscanf(config_file, "%c", &simvol);
            if ((simvol > 0x1F || simvol == '\t') && res_read != EOF) str_info += simvol;
        }

        if (res_read == EOF && str_info.size() == 0)
        {
            break;
        }

        pos[0] = str_info.find('\t', 0);
        if (pos[0] != 10) { result |= 2; continue; }

        for (int i = 0; i < 10; i++)
        {
            KKS.KKS[i] = str_info[i];
        }

        pos[0] = str_info.find('\t', pos[0] + 1);
        KKS.index = atoi(str_info.substr((size_t)pos[0] + 1).c_str());

        for (;;)
        {
            flag_err_type = 0;
            if (str_info.find("Analog") != -1) { KKS.type = TypeData::Analog; break; }
            if (str_info.find("Discrete") != -1) { KKS.type = TypeData::Discrete; break; }
            if (str_info.find("Binar") != -1) { KKS.type = TypeData::Binar; break; }

            flag_err_type = 1;
            result |= 4;
            break;
        }

        if (flag_err_type == 1) continue;

        count = 0;
        for (;;)
        {
            if (count == VectKKSIn.size()) break;
            if ((char)VectKKSIn[count].type > (char)KKS.type) { count++; continue; }
            break;
        }

        for (;;)
        {
            if (count == VectKKSIn.size()) break;
            if ((char)VectKKSIn[count].type == (char)KKS.type && VectKKSIn[count].index < KKS.index) { count++; continue; }
            break;
        }

        if (count == VectKKSIn.size())
        {
            VectKKSIn.push_back(KKS);
            continue;
        }

        if (VectKKSIn[count].index == KKS.index)
        {
            result |= 8;
            continue;
        }

        iter = VectKKSIn.begin();
        iter+=count;
        VectKKSIn.insert(iter, KKS);

        if (res_read == EOF)
        {
            break;
        }
    }
    return result;
}


/// --- функция чтения списка KKSIn --- ///
/*
0 - ошибка открытия файла
1 - ошибка в формате данных KKS
2 - ошибка типа данных KKS
3 - повтрный индекс KKS
*/
unsigned int GateDTS::ReadListKKSOut()
{
    FILE* config_file = NULL;
    char simvol = 0;
    std::string str_info;
    std::string helpstr;
    int res_read = 0;
    int pos[2] = { 0,0 };
    int count = 0;
    char status = 0;
    unsigned int result = 0;
    auto iter = VectKKSOut.begin();

    KKS_RAEK KKS;
    char flag_err_type = 0;

    config_file = fopen(NameFileListKKSOut, "r");
    if (config_file == NULL)
    {
        result |= 1;
        return result;
    }

    for (;;)
    {
        simvol = 0;
        str_info.clear();
        while (simvol != '\n' && res_read != EOF)
        {
            res_read = fscanf(config_file, "%c", &simvol);
            if ((simvol > 0x1F || simvol == '\t') && res_read != EOF) str_info += simvol;
        }

        if (res_read == EOF && str_info.size() == 0)
        {
            break;
        }

        pos[0] = str_info.find('\t', 0);
        if (pos[0] != 10) { result |= 2; continue; }

        for (int i = 0; i < 10; i++)
        {
            KKS.KKS[i] = str_info[i];
        }

        pos[0] = str_info.find('\t', pos[0] + 1);
        KKS.index = atoi(str_info.substr((size_t)pos[0] + 1).c_str());

        for (;;)
        {
            flag_err_type = 0;
            if (str_info.find("Analog") != -1) { KKS.type = TypeData::Analog; break; }
            if (str_info.find("Discrete") != -1) { KKS.type = TypeData::Discrete; break; }
            if (str_info.find("Binar") != -1) { KKS.type = TypeData::Binar; break; }

            flag_err_type = 1;
            result |= 4;
            break;
        }

        if (flag_err_type == 1) continue;

        count = 0;
        for (;;)
        {
            if (count == VectKKSOut.size()) break;
            if ((char)VectKKSOut[count].type > (char)KKS.type) { count++; continue; }
            break;
        }

        for (;;)
        {
            if (count == VectKKSOut.size()) break;
            if ((char)VectKKSOut[count].type == (char)KKS.type && VectKKSOut[count].index < KKS.index) { count++; continue; }
            break;
        }

        if (count == VectKKSOut.size())
        {
            VectKKSOut.push_back(KKS);
            continue;
        }

        if (VectKKSOut[count].index == KKS.index)
        {
            result |= 8;
            break;
        }

        iter = VectKKSOut.begin();
        iter += count;
        VectKKSOut.insert(iter, KKS);

        if (res_read == EOF)
        {
            break;
        }
    }
    return result;
}


/// --- проыервка статуса общей памяти --- ///
/*
-1 - ошибка инициализации мьютекса
-2 - ошибка инициализации памяти
-3 - ошибка отображения памяти
>0 - код команды
*/
int GateDTS::GetStatusMemory()
{
    unsigned char status;

    unsigned int result = 0;
    if (MutexSharMemStatus == NULL)
    {
        MutexSharMemStatus = CreateMutexA(NULL, FALSE, NameMutStatusMemoryGate);
        if (MutexSharMemStatus == NULL)
        {
            result = -1;
            return result;
        }
    }

    if (SharMemStatus == NULL)
    {
        SharMemStatus = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, NameStatusMemoryGate);
        if (SharMemStatus == NULL)
        {
            result = -2;
            return result;
        }
    }

    if (buf_status == NULL)
    {
        buf_status = (char*)MapViewOfFile(SharMemStatus, FILE_MAP_ALL_ACCESS, 0, 0, SizeMapStatus);
        if (buf_status == NULL)
        {
            result = -3;
            return result;
        }
    }


    WaitForSingleObject(MutexSharMemStatus, INFINITE);

    num_KKSIn = *(int*)(buf_status + 2);
    num_KKSOut = *(int*)(buf_status + 6);
    num_channels = *(int*)(buf_status + 10);

    status = *(buf_status + 1);
    status |= flag_first_init;

    if ((status & 4) > 0)
    {
        *(buf_status + 1) = status & (~4);
        flag_first_init &= (~4);
        status = status & 4;
    }
    else if ((status & 2) > 0)
    {
        *(buf_status + 1) = status & (~2);
        flag_first_init &= (~2);
        status = status & 2;
    }
    else if ((status & 1) > 0)
    {
        *(buf_status + 1) = status & (~1);
        flag_first_init &= (~1);
        status = status & 1;
    }

    ReleaseMutex(MutexSharMemStatus);
    
    return status;
}



/// --- функция чтения списка каналов --- ///
/*
0 - ошибка инициализации мьютекса
1 - ошибка открытия памяти
2 - ошибка отображения памяти

*/

unsigned int GateDTS::UpdateListChannels()
{
    HANDLE memory;
    InfoChannels* buf;
    int result = 0;
    if (MutexMemoryInfoChannels == NULL)
    {
        MutexMemoryInfoChannels = CreateMutexA(&security->getsecurityattrebut(), FALSE, NameMutMemoryInfoChannels);
        if (MutexMemoryInfoChannels == NULL)
        {
            result |= 1;
            return result;
        }
    }

    memory = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, NameMemoryInfoChannels);
    if (memory == NULL)
    {
        result |= 2;
        return result;
    }

    buf = (InfoChannels*)MapViewOfFile(memory,FILE_MAP_ALL_ACCESS,0,0, sizeof(InfoChannels)*num_channels);
    if (buf == NULL)
    {
        CloseHandle(memory);
        result |= 4;
        return result;
    }

    WaitForSingleObject(MutexMemoryInfoChannels, INFINITE);
    
    VectChannels.clear();
    for (int i = 0; i < num_channels; i++)
    {
        VectChannels.push_back(*(buf + i));
    }
    
    UnmapViewOfFile(buf);
    CloseHandle(memory);

    ReleaseMutex(MutexMemoryInfoChannels);

    return result;
}

/// --- функция обновления листа KKS IN --- ///
/*
0 - ошибка инициализации мьютекса
1 - ошибка открытия памяти
2 - ошибка отображения памяти
*/
unsigned int GateDTS::UpdateListKKSInMem()
{
    int result = 0;
    HANDLE memory;
    KKSDTS* buf;

    if (MutexUpdateListKKSInMem == NULL)
    {
        MutexUpdateListKKSInMem = CreateMutexA(&security->getsecurityattrebut(), FALSE, NameMutKKSInPut);
        if (MutexUpdateListKKSInMem == NULL)
        {
            result |= 1;
            return result;
        }
    }

    memory = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, NameMemoryKKSInPut);
    if (memory == NULL)
    {
        result |= 2;
        return result;
    }

    buf = (KKSDTS*)MapViewOfFile(memory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(KKSDTS) * num_KKSIn);
    if (buf == NULL)
    {
        CloseHandle(memory);
        result |= 4;
        return result;
    }

    WaitForSingleObject(MutexUpdateListKKSInMem, INFINITE);

    VectKKSInМem.clear();
    for (int i = 0; i < num_KKSIn; i++)
    {
        VectKKSInМem.push_back(*(buf + i));
    }

    ReleaseMutex(MutexUpdateListKKSInMem);

    UnmapViewOfFile(buf);
    CloseHandle(memory);

    return result;
}


/// --- функция обновления листа KKS OUT--- ///
/*
0 - ошибка инициализации мьютекса
1 - ошибка открытия памяти
2 - ошибка отображения памяти
*/
unsigned int GateDTS::UpdateListKKSOutMem()
{
    int result = 0;
    HANDLE memory;
    KKSDTS* buf;

    if (MutexUpdateListKKSOutMem == NULL)
    {
        MutexUpdateListKKSOutMem = CreateMutexA(&security->getsecurityattrebut(), FALSE, NameMutKKSOutPut);
        if (MutexUpdateListKKSOutMem == NULL)
        {
            result |= 1;
            return result;
        }
    }

    memory = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, NameMemoryKKSOutPut);
    if (memory == NULL)
    {
        result |= 2;
        return result;
    }

    buf = (KKSDTS*)MapViewOfFile(memory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(KKSDTS) * num_KKSOut);
    if (buf == NULL)
    {
        CloseHandle(memory);
        result |= 4;
        return result;
    }

    WaitForSingleObject(MutexUpdateListKKSOutMem, INFINITE);

    VectKKSOutМem.clear();
    for (int i = 0; i < num_KKSOut; i++)
    {
        VectKKSOutМem.push_back(*(buf + i));
    }

    ReleaseMutex(MutexUpdateListKKSOutMem);

    UnmapViewOfFile(buf);
    CloseHandle(memory);

    return result;
}


/// --- обновдения таблицы соотношения данных REAK DTS --- /// тут бы оптимизировать процесс
/*
0 - вектор KKS IN REAK пуст
1 - вектор KKS IN DTS пуст

*/

unsigned int GateDTS::UpdateTabConcordKKSIn()
{
    unsigned int  result = 0;
    unsigned int count = 0;
    char flag_write = 0;
    if (VectKKSIn.size() == 0) { result |= 1; return result; }
    if (VectKKSInМem.size() == 0) { result |= 2; return result; }
    KKSConcord_RAEK unit;

    TabConcordKKSIn.clear();

    for (int i = 0; i < VectKKSInМem.size(); i++)
    {
        unit.channel = VectKKSInМem[i].channel;
        unit.type = VectKKSInМem[i].typedata;
        unit.indexdts = VectKKSInМem[i].indexdts;
        for (int j = 0; j < 10; j++)
        {
            unit.KKS[j] = VectKKSInМem[i].KKS[j];
        }
        unit.indexraek = -1;

        TabConcordKKSIn.push_back(unit);
    }

    unit.channel = -1;
    unit.type = TypeData::Empty;
    unit.indexdts = -1;
    for (int j = 0; j < 10; j++)
    {
        unit.KKS[j] = 0;
    }
    unit.indexraek = -1;

    for (int i = 0; i< VectKKSIn.size(); i++)
    {
        flag_write = 0;
        for (int j = 0; j < TabConcordKKSIn.size(); j++)
        {
            count = 0;
            if (TabConcordKKSIn[j].channel < 0) break;
            for (int x = 0; x < 10; x++)
            {
                if (VectKKSIn[i].KKS[x] == TabConcordKKSIn[j].KKS[x]) { count++; continue; }
                break;
            }
            if (count != 10) continue;

            TabConcordKKSIn[j].indexraek = VectKKSIn[i].index;
            flag_write = 1;
            break;
        }

        if (flag_write == 1) continue;
        
        for (int x = 0; x < 10; x++)
        {
            unit.KKS[x] = VectKKSIn[i].KKS[x];
        }
        unit.indexraek = VectKKSIn[i].index;

        TabConcordKKSIn.push_back(unit);
    }

    return result;
}

/// --- обновдения таблицы соотношения данных REAK DTS --- /// тут бы оптимизировать процесс
/*
0 - вектор KKS OUT REAK пуст
1 - вектор KKS OUT DTS пуст

*/

unsigned int GateDTS::UpdateTabConcordKKSOut()
{
    unsigned int  result = 0;
    unsigned int count = 0;
    char flag_write = 0;
    if (VectKKSOut.size() == 0) { result |= 1; return result; }
    if (VectKKSOutМem.size() == 0) { result |= 2; return result; }
    KKSConcord_RAEK unit;

    TabConcordKKSOut.clear();

    for (int i = 0; i < VectKKSOutМem.size(); i++)
    {
        unit.channel = VectKKSOutМem[i].channel;
        unit.type = VectKKSOutМem[i].typedata;
        unit.indexdts = VectKKSOutМem[i].indexdts;
        for (int j = 0; j < 10; j++)
        {
            unit.KKS[j] = VectKKSOutМem[i].KKS[j];
        }
        unit.indexraek = -1;

        TabConcordKKSOut.push_back(unit);
    }

    unit.channel = -1;
    unit.type = TypeData::Empty;
    unit.indexdts = -1;
    for (int j = 0; j < 10; j++)
    {
        unit.KKS[j] = 0;
    }
    unit.indexraek = -1;

    for (int i = 0; i < VectKKSOut.size(); i++)
    {
        flag_write = 0;
        for (int j = 0; j < TabConcordKKSOut.size(); j++)
        {
            count = 0;
            if (TabConcordKKSOut[j].channel < 0) break;
            for (int x = 0; x < 10; x++)
            {
                if (VectKKSOut[i].KKS[x] == TabConcordKKSOut[j].KKS[x]) { count++; continue; }
                break;
            }
            if (count != 10) continue;

            TabConcordKKSOut[j].indexraek = VectKKSOut[i].index;
            flag_write = 1;
            break;
        }

        if (flag_write == 1) continue;

        for (int x = 0; x < 10; x++)
        {
            unit.KKS[x] = VectKKSOut[i].KKS[x];
        }
        unit.indexraek = VectKKSOut[i].index;

        TabConcordKKSOut.push_back(unit);
    }

    return result;
}


/// --- функция записи таблицы KKS IN в общую память для визора --- ///
/*
 0 - нулевой размер таблицы
 1 - ошибка инициализации мьютекса
 2 - ошибка инициализации памяти
 3 - ошибка отображения памяти

*/
unsigned int GateDTS::UpdateMemoryTabConcordIn()
{
    unsigned int result = 0;
    int* number;
    char* buf;

    if (TabConcordKKSIn.size() == 0)
    {
        result |= 1;
        return result;
    }

    if (MutexConcordKKSIn == NULL)
    {
        MutexConcordKKSIn = CreateMutexA(&security->getsecurityattrebut(), FALSE, NameMutexConcordKKSIn);
        if (MutexConcordKKSIn == NULL)
        {
            result |=2;
            return result;
        }
    }

    WaitForSingleObject(MutexConcordKKSIn, INFINITE);

    if (MemoryConcordKKSIn != NULL)
    {
        CloseHandle(MemoryConcordKKSIn);
        MemoryConcordKKSIn = NULL;
    }

    MemoryConcordKKSIn = CreateFileMappingA(INVALID_HANDLE_VALUE, &security->getsecurityattrebut(), PAGE_READWRITE, 0, 4 + sizeof(KKSConcord_RAEK) * TabConcordKKSIn.size(), NameMemoryConcordKKSIn);
    if (MemoryConcordKKSIn == NULL)
    {
        ReleaseMutex(MutexConcordKKSIn);
        result |= 4;
        return result;
    }
    
    buf= (char*)MapViewOfFile(MemoryConcordKKSIn, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (buf == NULL)
    {
        ReleaseMutex(MutexConcordKKSIn);
        result |= 8;
        return result;
    }
    
    *(int*)buf = TabConcordKKSIn.size();

    for (int i = 0; i < TabConcordKKSIn.size(); i++)
    {
        *(KKSConcord_RAEK*)(buf + 4 + sizeof(KKSConcord_RAEK) * i) = TabConcordKKSIn[i];
    }
    UnmapViewOfFile(buf);

    ReleaseMutex(MutexConcordKKSIn);

    return result;
}


/// --- функция записи таблицы KKS OUT в общую память для визора --- ///
/*
 0 - нулевой размер таблицы
 1 - ошибка инициализации мьютекса
 2 - ошибка инициализации памяти
 3 - ошибка отображения памяти

*/
unsigned int GateDTS::UpdateMemoryTabConcordOut()
{
    unsigned int result = 0;
    int* number;
    char* buf;

    if (TabConcordKKSOut.size() == 0)
    {
        result |= 1;
        return result;
    }

    if (MutexConcordKKSOut == NULL)
    {
        MutexConcordKKSOut = CreateMutexA(&security->getsecurityattrebut(), FALSE, NameMutexConcordKKSOut);
        if (MutexConcordKKSOut == NULL)
        {
            result |= 2;
            return result;
        }
    }

    WaitForSingleObject(MutexConcordKKSOut, INFINITE);

    if (MemoryConcordKKSOut != NULL)
    {
        CloseHandle(MemoryConcordKKSOut);
        MemoryConcordKKSOut = NULL;
    }

    MemoryConcordKKSOut = CreateFileMappingA(INVALID_HANDLE_VALUE, &security->getsecurityattrebut(), PAGE_READWRITE, 0, 4 + sizeof(KKSConcord_RAEK) * TabConcordKKSOut.size(), NameMemoryConcordKKSOut);
    if (MemoryConcordKKSOut == NULL)
    {
        ReleaseMutex(MutexConcordKKSOut);
        result |= 4;
        return result;
    }

    buf = (char*)MapViewOfFile(MemoryConcordKKSOut, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (buf == NULL)
    {
        ReleaseMutex(MutexConcordKKSOut);
        result |= 8;
        return result;
    }

    *(int*)buf = TabConcordKKSOut.size();

    for (int i = 0; i < TabConcordKKSOut.size(); i++)
    {
        *(KKSConcord_RAEK*)(buf + 4 + sizeof(KKSConcord_RAEK) * i) = TabConcordKKSOut[i];
    }
    UnmapViewOfFile(buf);

    ReleaseMutex(MutexConcordKKSOut);

    return result;
}


/// --- функция записи аналоговых данных --- ///
/*
0 - таблица соглалования KKS пуста
1 - присуствует ошибра открытия памяти
2 - присуствует ошибка отображения
3 - индекс выходит за границы входного буфера
4 - индекс выходин за границы буфера общей памяти
*/


unsigned int GateDTS::WriteData(TypeData TP, void* buf, int size_buf)
{
    unsigned int result = 0;
    HANDLE memory = NULL;
    //TypeData td = TypeData::Analog;
    int current_channel = -1;
    char* buf_dts = NULL;
    char flag_write = 0;
    int size_type = 0;

    result = CheckStatusSharedMemory();

    if (TabConcordKKSOut.size() == 0) { result |= (1<<21); return result; }
    int size_data_channel = 0;

    if (TP == TypeData::Analog) size_type = sizeof(float);
    if (TP == TypeData::Discrete) size_type = sizeof(int);
    if (TP == TypeData::Binar) size_type = sizeof(char);

    for (int i = 0; i < TabConcordKKSOut.size(); i++)
    {
        if (current_channel != TabConcordKKSOut[i].channel)
        {
            if (TabConcordKKSOut[i].channel < 0) break;
            current_channel = TabConcordKKSOut[i].channel;
            size_data_channel = 0;

            for (int j = 0; j < VectChannels.size(); j++)
            {
                if (VectChannels[j].channel == current_channel) { size_data_channel = VectChannels[j].countAout; break; }
            }
            if (size_data_channel <= 0) continue;

            if (buf_dts != NULL) { UnmapViewOfFile(buf_dts); buf_dts = NULL; }
            if (memory != NULL) { CloseHandle(memory); memory = NULL; }

            memory = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, CreateNameMemory(TP, TypeValue::OUTPUT, current_channel).c_str());
            if (memory == NULL) { result |= 1; flag_write = 0; continue; }
            buf_dts = (char*)MapViewOfFile(memory, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            if (buf_dts == NULL) { CloseHandle(memory); memory = NULL; flag_write = 0; result |= (1<<22); continue; }
            flag_write = 1;
        }

        if (flag_write == 1 && TabConcordKKSOut[i].type == TP)
        {
            if (TabConcordKKSOut[i].indexraek >= size_buf) { result |= (1<<23); continue; }
            if (TabConcordKKSOut[i].indexdts >= size_data_channel) { result |= (1<<24); continue; }

            for (int j = 0; j < size_type; j++)
            {
                *(char*)(buf_dts + size_type * TabConcordKKSOut[i].indexdts+j) = *(((char*)buf) + TabConcordKKSOut[i].indexraek*size_type + j);
            }
            //*(float*)(buf_dts + size_type * TabConcordKKSOut[i].indexdts) = *(((float*)buf) + TabConcordKKSOut[i].indexraek);
        }
    }

    if (buf_dts != NULL) { UnmapViewOfFile(buf_dts); buf_dts = NULL; }
    if (memory != NULL) { CloseHandle(memory); memory = NULL; }

    return result;
}

unsigned int GateDTS::ReadData(TypeData TP, void* buf, int size_buf)
{
    unsigned int result = 0;
    HANDLE memory = NULL;
    //TypeData td = TypeData::Analog;
    int current_channel = -1;
    char* buf_dts = NULL;
    char flag_read = 0;
    int size_type = 0;

    result = CheckStatusSharedMemory();

    if (TabConcordKKSIn.size() == 0) { result |= (1<<21); return result; }
    int size_data_channel = 0;

    if (TP == TypeData::Analog) size_type = sizeof(float);
    if (TP == TypeData::Discrete) size_type = sizeof(int);
    if (TP == TypeData::Binar) size_type = sizeof(char);
    

    for (int i = 0; i < TabConcordKKSIn.size(); i++)
    {
        if (TabConcordKKSIn[i].channel < 0) break;
        if (current_channel != TabConcordKKSIn[i].channel)
        {
            current_channel = TabConcordKKSIn[i].channel;
            size_data_channel = 0;

            for (int j = 0; j < VectChannels.size(); j++)
            {
                if (VectChannels[j].channel == current_channel) { size_data_channel = VectChannels[j].countAin; break; }
            }
            if (size_data_channel <= 0) continue;

            if (memory != NULL) { CloseHandle(memory); memory = NULL; }
            if (buf_dts != NULL) { UnmapViewOfFile(buf_dts); buf_dts = NULL; }
            memory = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, CreateNameMemory(TP, TypeValue::INPUT, current_channel).c_str());
            if (memory == NULL) { result |= 1; flag_read = 0; continue; }
            buf_dts = (char*)MapViewOfFile(memory, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            if (buf_dts == NULL) { CloseHandle(memory); memory = NULL; flag_read = 0; result |= (1<<22); continue; }
            flag_read = 1;
        }

        if (flag_read == 1 && TabConcordKKSIn[i].type == TP)
        {
            if (TabConcordKKSIn[i].indexraek >= size_buf) { result |= (1<<23); continue; }
            if (TabConcordKKSIn[i].indexdts >= size_data_channel) { result |= (1<<24); continue; }

            for (int j = 0; j < size_type; j++)
            {
                *(((char*)buf) + TabConcordKKSIn[i].indexraek*size_type+j) = *(char*)(buf_dts + size_type * TabConcordKKSIn[i].indexdts+j);
            }
            //*(((float*)buf) + TabConcordKKSIn[i].indexraek) = *(float*)(buf_dts + size_type * TabConcordKKSIn[i].indexdts);
        }
    }

    if (buf_dts != NULL) { UnmapViewOfFile(buf_dts); buf_dts = NULL; }
    if (memory != NULL) { CloseHandle(memory); memory = NULL; }
    return result;
}


// --- проверк/обновления данных --- ///
/*
0 - ошибка GetStatusMemory инициализации мьютекса 
1 - ошибка GetStatusMemory инициализации памяти
2 - ошибка GetStatusMemory отображения памяти
3 - ошибка UpdateListChannels инициализации мьютекса
4 - ошибка UpdateListChannels открытия памяти
5 - ошибка UpdateListChannels отображения памяти
6 - ошибка UpdateListKKSMemIn инициализации мьютекса
7 - ошибка UpdateListKKSMemIn открытия памяти
8 - ошибка UpdateListKKSMemIn отображения памяти
9 - ошибка UpdateListKKSMemOut инициализации мьютекса
10 - ошибка UpdateListKKSMemOut открытия памяти
11 - ошибка UpdateListKKSMemOut отображения памяти
12 - ошибка UpdateTabConcordKKSIn вектор KKS EMT пуст
13 - ошибка UpdateTabConcordKKSIn KKS IN DTS пуст
14 - ошибка UpdateTabConcordKKSOut вектор KKS EMT пуст
15 - ошибка UpdateTabConcordKKSOut KKS IN DTS пуст
16 - ошибка UpdateMemoryTabConcordin инициализации мьютекса
17 - ошибка UpdateMemoryTabConcordin инициализации памяти
18 - ошибка UpdateMemoryTabConcordin отображения памяти
19 - ошибка UpdateMemoryTabConcordout инициализации мьютекса
20 - ошибка UpdateMemoryTabConcordout инициализации памяти
21 - ошибка UpdateMemoryTabConcordout отображения памяти
22 - таблица соглалования KKS пуста
23 - присуствует ошибра открытия памяти
24 - присуствует ошибка отображения
25 - индекс выходит за границы входного буфера
26 - индекс выходин за границы буфера общей памяти
*/

unsigned int GateDTS::CheckStatusSharedMemory()
{
    unsigned int result = 0;
    unsigned res = 0;
    int status;
    for (;;)
    {
        status = GetStatusMemory();
        if (status < 0)
        {
            if (status == -1) result|=1;
            if (status == -2) result |= 2;
            if (status == -3) result |= 4;
            continue;
        }

        if (status == (int)CommandEmt::UpdateListChannel)
        {
            res = UpdateListChannels();  /// 3 бита
            if (res != 0)
            {
                result |= (res << 3);
            }
            continue;
        }

        if (status == (int)CommandEmt::UpdateKKSListIn)
        {
            res = UpdateListKKSInMem();
            if (res != 0)
            {
                result |= (res << 3); // 3 бита
                continue;
            }

            res = UpdateTabConcordKKSIn();
            if (res != 0)
            {
                result |= (res << 6); //  2 бита
                continue;
            }

            res = UpdateMemoryTabConcordIn();
            if (res != 0)
            {
                result |= (res << 8);
                continue;
            }
            continue;
        }

        if (status == (int)CommandEmt::UpdateKKSListOut)
        {
            res = UpdateListKKSOutMem();
            if (res != 0)
            {
                result |= (res << 12);
                continue;
            }

            UpdateTabConcordKKSOut();
            if (res != 0)
            {
                result |= (res << 15);
                continue;
            }

            UpdateMemoryTabConcordOut();
            if (res != 0)
            {
                
                result |= (res << 17);
                continue;
            }
            continue;
        }

        break;
    }

    return result;
}