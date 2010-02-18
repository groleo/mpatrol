/* gcc -g -Wall -DPSAPI_VERSION=1 -c -o valgrind_dll.o valgrind_dll.c */
/* gcc -shared -Wl,-soname,valgrind_dll.dll -Wl,--out-implib,libvalgrind_dll.dll.a -o valgrind_dll.dll valgrind_dll.o -lpsapi -limagehlp */

#include <stdio.h>

#include <windows.h>
#include <psapi.h>
#include <imagehlp.h>

typedef struct
{
  char *func_name_old;
  PROC func_proc_old;
  PROC func_proc_new;
} Vg_Hook;

typedef LPVOID (WINAPI *vgd_heap_alloc_t) (HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
typedef BOOL   (WINAPI *vgd_heap_free_t)  (HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);


Vg_Hook vg_hooks_kernel32[2];

LPVOID WINAPI VG_HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes)
{
  vgd_heap_alloc_t ha;
  LPVOID data;

  printf("HeapAlloc !!!\n");

  ha = (vgd_heap_alloc_t)vg_hooks_kernel32[0].func_proc_old;
  data = ha(hHeap, dwFlags, dwBytes);

  return data;
}

BOOL WINAPI VG_HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem)
{
  vgd_heap_free_t hf;
  BOOL res;

  printf("HeapFree !!!\n");

  hf = (vgd_heap_free_t)vg_hooks_kernel32[1].func_proc_old;
  res = hf(hHeap, dwFlags, lpMem);

  return res;  
}


void
_vgd_modules_hook_set(HMODULE module, const char *lib_name, PROC old_function_proc, PROC new_function_proc)
{
  PIMAGE_IMPORT_DESCRIPTOR iid;
  PIMAGE_THUNK_DATA thunk;
  ULONG size;

  iid = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(module, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size);
  if (!iid)
    return;

  while (iid->Name)
    {
      PSTR module_name;

      module_name = (PSTR)((PBYTE) module + iid->Name);
      if (_stricmp(module_name, lib_name) == 0)
        break;
      iid++;
    }

  if (!iid->Name)
    return;

  thunk = (PIMAGE_THUNK_DATA)((PBYTE)module + iid->FirstThunk );
  while (thunk->u1.Function)
    {
      PROC *func;

      func = (PROC *)&thunk->u1.Function;
      if (*func == old_function_proc)
        {
          MEMORY_BASIC_INFORMATION mbi;
          DWORD dwOldProtect;

          VirtualQuery(func, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

          if (!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &mbi.Protect))
            return;

          *func = *new_function_proc;
          VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &dwOldProtect);
          break;
        }
      thunk++;
    }
}

void
_vgd_modules_hook(HMODULE main_module, const char *lib_name)
{
  HMODULE mods[1024];
  HMODULE lib_module;
  DWORD mods_nbr;
  unsigned int i;
  unsigned int j;

  lib_module = LoadLibrary(lib_name);

  for (j = 0; vg_hooks_kernel32[j].func_name_old; j++)
    vg_hooks_kernel32[j].func_proc_old = GetProcAddress(lib_module, vg_hooks_kernel32[j].func_name_old);

  if (!EnumProcessModules(GetCurrentProcess(), mods, sizeof(mods), &mods_nbr))
    return;

  for (i = 0; i < (mods_nbr / sizeof(HMODULE)); i++)
    {
      if (mods[i] == main_module)
        continue;

      for (j = 0; vg_hooks_kernel32[j].func_name_old; j++)
        _vgd_modules_hook_set(mods[i], lib_name,
                              vg_hooks_kernel32[j].func_proc_old,
                              vg_hooks_kernel32[j].func_proc_new);
    }

  FreeLibrary(lib_module);
}

void
_vgd_modules_unhook(HMODULE main_module, const char *lib_name)
{
  HMODULE mods[1024];
  DWORD mods_nbr;
  unsigned int i;
  unsigned int j;

  if (!EnumProcessModules(GetCurrentProcess(), mods, sizeof(mods), &mods_nbr))
    return;

  for (i = 0; i < (mods_nbr / sizeof(HMODULE)); i++)
    {
      if (mods[i] == main_module) continue;

      for (j = 0; vg_hooks_kernel32[j].func_name_old; j++)
        _vgd_modules_hook_set(mods[i], lib_name,
                              vg_hooks_kernel32[j].func_proc_new,
                              vg_hooks_kernel32[j].func_proc_old);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReason, LPVOID lpReserved)
{
  switch (ulReason)
    {
    case DLL_PROCESS_ATTACH:
      printf ("process attach\n");
      vg_hooks_kernel32[0].func_name_old = "HeapAlloc";
      vg_hooks_kernel32[0].func_proc_old = NULL;
      vg_hooks_kernel32[0].func_proc_new = (PROC)VG_HeapAlloc;

      vg_hooks_kernel32[1].func_name_old = "HeapFree";
      vg_hooks_kernel32[1].func_proc_old = NULL;
      vg_hooks_kernel32[1].func_proc_new = (PROC)VG_HeapFree;

      vg_hooks_kernel32[2].func_name_old = NULL;
      vg_hooks_kernel32[2].func_proc_old = NULL;
      vg_hooks_kernel32[2].func_proc_new = NULL;

      break;
    case DLL_THREAD_ATTACH:
      printf ("thread attach\n");
      _vgd_modules_hook(hModule, "kernel32.dll");
      break;
    case DLL_THREAD_DETACH:
      _vgd_modules_unhook(hModule, "kernel32.dll");
      break;
    case DLL_PROCESS_DETACH:
      break;
    }

    return TRUE;
}
