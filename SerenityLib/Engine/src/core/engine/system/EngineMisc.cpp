#include <core/engine/system/EngineMisc.h>

#ifdef _WIN32
#include <Windows.h>
#endif

using namespace std;

string Engine::misc::extractClipboardText() {
    string res = "";
    #ifdef _WIN32
        // Try opening the clipboard
        if (!OpenClipboard(nullptr)) {
            return res;
        }
        // Get handle of clipboard object for ANSI text
        HANDLE hData = GetClipboardData(CF_TEXT);
        if (hData == nullptr) {
            //... // error
        }else{
            // Lock the handle to get the actual text pointer
            char* pszText = static_cast<char*>(GlobalLock(hData));
            if (pszText == nullptr) {
                return res;
            }
            // Save text in a string class instance
            res = pszText;
            GlobalUnlock(hData);
        }
        CloseClipboard();
    #endif
    return res;
}
