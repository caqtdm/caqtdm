#ifndef __WIN32UNICODE_H__
#define __WIN32UNICODE_H__

// From MS VC Knowledge Base....


// ANSI -> OLE (wide) string.
// OLS string must be freed with CoTaskMemFree
// (unless passed to an OLE function which becomes owner).
HRESULT AnsiToUnicode(LPCSTR pszA, LPOLESTR* ppszW);

// Other direction. ANSI string must be freed with CoTaskMemFree...
HRESULT UnicodeToAnsi(LPCOLESTR pszW, LPSTR* ppszA);

#endif //__WIN32UNICODE_H__