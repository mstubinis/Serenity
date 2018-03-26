#include "Engine.h"
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <windowsx.h>
#endif

#ifdef _DEBUG
    #include <vld.h> //TODO: remove this header eventually, it's only used to track memory leaks in debug mode

	// Debug memory block header:
	//    o  Borrowed from the Microsoft CRT to fix the false "memory leak" report
	//       when using typeinfo 'name' accessor in a _DEBUG build of the library.  
	struct _CrtMemBlockHeader{
	   struct _CrtMemBlockHeader*  pBlockHeaderNext;
	   struct _CrtMemBlockHeader*  pBlockHeaderPrev;
	   char*                       szFileName;
	   int                         nLine;
	   #ifdef _WIN64
		   int                     nBlockUse;
		   size_t                  nDataSize;
	   #else
		   size_t                  nDataSize;
		   int                     nBlockUse;
	   #endif
	   long                        lRequest;
	   unsigned char               gap[4];
	};
	static void __cdecl _FixTypeInfoBlockUse(void){
	   __type_info_node* pNode = __type_info_root_node._Next;
	   while(pNode != NULL){
		  __type_info_node* pNext = pNode->_Next;
		  (((_CrtMemBlockHeader*)pNode) - 1)->nBlockUse = _CRT_BLOCK;
		  if (pNode->_MemPtr != NULL)
			 (((_CrtMemBlockHeader*)pNode->_MemPtr) - 1)->nBlockUse = _CRT_BLOCK;
		  pNode = pNext;
	   }
	}
#endif

int main(){
    srand((unsigned)time(0));
    #ifdef _WIN32
        if(GetConsoleWindow() == NULL){ AllocConsole(); }
        freopen("CONIN$", "r", stdin);freopen("CONOUT$", "w", stdout);freopen("CONOUT$", "w", stderr);
        #ifndef _DEBUG
            //ShowWindow(GetConsoleWindow(), SW_HIDE);//hide console window if in release mode
        #endif
    #endif

    Engine::init("Engine",1024,768);
    Engine::run();

    #ifdef _WIN32
        FreeConsole();
    #endif
    #ifdef _DEBUG
		_FixTypeInfoBlockUse();
    #endif
    return 0;
}