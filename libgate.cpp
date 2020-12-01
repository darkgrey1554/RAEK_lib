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

/// --- ������������� ����������� ������������ ---///
/*
 0 - ������ AllocateAndInitializeSid
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

/// --- ������� ������ ������ KKSOut --- /// 
/*
0 - ������ �������� �����
1 - ������ � ������� ������ KKS
2 - ������ ���� ������ KKS
3 - �������� ������ KKS
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


/// --- ������� ������ ������ KKSIn --- ///
/*
0 - ������ �������� �����
1 - ������ � ������� ������ KKS
2 - ������ ���� ������ KKS
3 - �������� ������ KKS
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


/// --- ��������� ������� ����� ������ --- ///
/*
-1 - ������ ������������� ��������
-2 - ������ ������������� ������
-3 - ������ ����������� ������
>0 - ��� �������
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



/// --- ������� ������ ������ ������� --- ///
/*
0 - ������ ������������� ��������
1 - ������ �������� ������
2 - ������ ����������� ������

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

/// --- ������� ���������� ����� KKS IN --- ///
/*
0 - ������ ������������� ��������
1 - ������ �������� ������
2 - ������ ����������� ������
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

    VectKKSIn�em.clear();
    for (int i = 0; i < num_KKSIn; i++)
    {
        VectKKSIn�em.push_back(*(buf + i));
    }

    ReleaseMutex(MutexUpdateListKKSInMem);

    UnmapViewOfFile(buf);
    CloseHandle(memory);

    return result;
}


/// --- ������� ���������� ����� KKS OUT--- ///
/*
0 - ������ ������������� ��������
1 - ������ �������� ������
2 - ������ ����������� ������
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

    VectKKSOut�em.clear();
    for (int i = 0; i < num_KKSOut; i++)
    {
        VectKKSOut�em.push_back(*(buf + i));
    }

    ReleaseMutex(MutexUpdateListKKSOutMem);

    UnmapViewOfFile(buf);
    CloseHandle(memory);

    return result;
}


/// --- ���������� ������� ����������� ������ REAK DTS --- /// ��� �� �������������� �������
/*
0 - ������ KKS IN REAK ����
1 - ������ KKS IN DTS ����

*/

unsigned int GateDTS::UpdateTabConcordKKSIn()
{
    unsigned int  result = 0;
    unsigned int count = 0;
    char flag_write = 0;
    if (VectKKSIn.size() == 0) { result |= 1; return result; }
    if (VectKKSIn�em.size() == 0) { result |= 2; return result; }
    KKSConcord_RAEK unit;

    TabConcordKKSIn.clear();

    for (int i = 0; i < VectKKSIn�em.size(); i++)
    {
        unit.channel = VectKKSIn�em[i].channel;
        unit.type = VectKKSIn�em[i].typedata;
        unit.indexdts = VectKKSIn�em[i].indexdts;
        for (int j = 0; j < 10; j++)
        {
            unit.KKS[j] = VectKKSIn�em[i].KKS[j];
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

/// --- ���������� ������� ����������� ������ REAK DTS --- /// ��� �� �������������� �������
/*
0 - ������ KKS OUT REAK ����
1 - ������ KKS OUT DTS ����

*/

unsigned int GateDTS::UpdateTabConcordKKSOut()
{
    unsigned int  result = 0;
    unsigned int count = 0;
    char flag_write = 0;
    if (VectKKSOut.size() == 0) { result |= 1; return result; }
    if (VectKKSOut�em.size() == 0) { result |= 2; return result; }
    KKSConcord_RAEK unit;

    TabConcordKKSOut.clear();

    for (int i = 0; i < VectKKSOut�em.size(); i++)
    {
        unit.channel = VectKKSOut�em[i].channel;
        unit.type = VectKKSOut�em[i].typedata;
        unit.indexdts = VectKKSOut�em[i].indexdts;
        for (int j = 0; j < 10; j++)
        {
            unit.KKS[j] = VectKKSOut�em[i].KKS[j];
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


/// --- ������� ������ ������� KKS IN � ����� ������ ��� ������ --- ///
/*
 0 - ������� ������ �������
 1 - ������ ������������� ��������
 2 - ������ ������������� ������
 3 - ������ ����������� ������

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


/// --- ������� ������ ������� KKS OUT � ����� ������ ��� ������ --- ///
/*
 0 - ������� ������ �������
 1 - ������ ������������� ��������
 2 - ������ ������������� ������
 3 - ������ ����������� ������

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


/// --- ������� ������ ���������� ������ --- ///
/*
0 - ������� ������������ KKS �����
1 - ����������� ������ �������� ������
2 - ����������� ������ �����������
3 - ������ ������� �� ������� �������� ������
4 - ������ ������� �� ������� ������ ����� ������
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


// --- �������/���������� ������ --- ///
/*
0 - ������ GetStatusMemory ������������� �������� 
1 - ������ GetStatusMemory ������������� ������
2 - ������ GetStatusMemory ����������� ������
3 - ������ UpdateListChannels ������������� ��������
4 - ������ UpdateListChannels �������� ������
5 - ������ UpdateListChannels ����������� ������
6 - ������ UpdateListKKSMemIn ������������� ��������
7 - ������ UpdateListKKSMemIn �������� ������
8 - ������ UpdateListKKSMemIn ����������� ������
9 - ������ UpdateListKKSMemOut ������������� ��������
10 - ������ UpdateListKKSMemOut �������� ������
11 - ������ UpdateListKKSMemOut ����������� ������
12 - ������ UpdateTabConcordKKSIn ������ KKS EMT ����
13 - ������ UpdateTabConcordKKSIn KKS IN DTS ����
14 - ������ UpdateTabConcordKKSOut ������ KKS EMT ����
15 - ������ UpdateTabConcordKKSOut KKS IN DTS ����
16 - ������ UpdateMemoryTabConcordin ������������� ��������
17 - ������ UpdateMemoryTabConcordin ������������� ������
18 - ������ UpdateMemoryTabConcordin ����������� ������
19 - ������ UpdateMemoryTabConcordout ������������� ��������
20 - ������ UpdateMemoryTabConcordout ������������� ������
21 - ������ UpdateMemoryTabConcordout ����������� ������
22 - ������� ������������ KKS �����
23 - ����������� ������ �������� ������
24 - ����������� ������ �����������
25 - ������ ������� �� ������� �������� ������
26 - ������ ������� �� ������� ������ ����� ������
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
            res = UpdateListChannels();  /// 3 ����
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
                result |= (res << 3); // 3 ����
                continue;
            }

            res = UpdateTabConcordKKSIn();
            if (res != 0)
            {
                result |= (res << 6); //  2 ����
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