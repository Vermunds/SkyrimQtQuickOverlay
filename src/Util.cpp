#include "Util.h"
#include <comdef.h>

std::string QSK::GetComErrorString(HRESULT hr)
{
	const _com_error error(hr);

#ifdef UNICODE
	std::wstring wresult = error.ErrorMessage();

	if (wresult.empty())
		return std::string();
	int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wresult[0], (int)wresult.size(), NULL, 0, NULL, NULL);
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wresult[0], (int)wresult.size(), &result[0], sizeNeeded, NULL, NULL);
#else
	std::string result = error.ErrorMessage();
#endif  // UNICODE

	return result;
}
