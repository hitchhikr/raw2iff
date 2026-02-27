// =================================================================
#ifdef __WINDOWS__

// =================================================================
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

#else

// It's a hack but it works
unsigned int *SysBase;
void __free_all();

int start(PLUGIN_COMMAND *cmd_struct)
{
    int ret_value;
    asm("move.l 4.w,a0\n"
        "move.l a0,_SysBase\n");
    ret_value = process(cmd_struct);
    __free_all();
    return ret_value;
}

_ATTRIBUTE ((__noreturn__)) __stdargs void exit(int __status)
{
    __builtin_unreachable();
}

#endif
