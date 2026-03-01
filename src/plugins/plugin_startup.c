// =================================================================
#ifndef __AMIGA__

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

struct Library *Raw2IffBase;

void main(int argc, unsigned char **argv)
{
    Raw2IffBase = OldOpenLibrary("raw2iff.library");

    if(Raw2IffBase)
    {

#ifdef __PALETTE_PLUGIN__

        process(GetPalStruct());

#else

        process(GetPicStruct());

#endif

        CloseLibrary(Raw2IffBase);
    }
}

#endif
