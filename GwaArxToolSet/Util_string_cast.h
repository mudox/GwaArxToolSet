#pragma once

#include "Util.h"

namespace GwaArx
{
	namespace Util	
	{
		namespace _string_cast
		{
			// string ->> wstring
			template<std::wstring::size_type MAX_SIZE>
			std::wstring string_cast(std::string _in)
			{
				wchar_t _outBuf[MAX_SIZE] = { 0 };

				HRESULT hr = StringCchPrintfW(_outBuf, MAX_SIZE, L"%S", _in.c_str());

				if (SUCCEEDED(hr))
				{
					return std::wstring(_outBuf);
				}
				else // error occurred, throw proper info.
				{
					std::string _errInfo;

					switch (hr)
					{
					case STRSAFE_E_INVALID_PARAMETER:
						_errInfo = "invalid argument for StringcChPrintfW()";
						break;
					case STRSAFE_E_INSUFFICIENT_BUFFER:
						_errInfo = "insufficient buffer for StringcChPrintW()";
						break;
					default:
						_errInfo = "string_cast() - could not got here";
					}

					throw std::runtime_error(_errInfo);
				}
			}

			// wstring ->> string
			template<std::string::size_type MAX_SIZE>
			std::string string_cast(std::wstring _in)
			{
				char _outBuf[MAX_SIZE] = { 0 };

				HRESULT hr = StringCchPrintfA(_outBuf, MAX_SIZE, "%S", _in.c_str());

				if (SUCCEEDED(hr))
				{
					return std::string(_outBuf);
				}
				else // error occurred, throw proper info.
				{
					std::string _errInfo;

					switch (hr)
					{
					case STRSAFE_E_INVALID_PARAMETER:
						_errInfo = "invalid argument for StringcChPrintfA()";
							break;
					case STRSAFE_E_INSUFFICIENT_BUFFER:
						_errInfo = "insufficient buffer for StringcChPrintA()";
							break;
					default:
						_errInfo = "string_cast() - could not got here";
					}

					throw std::runtime_error(_errInfo);
				}
			}

			// string ->> wstring
			template<std::wstring::size_type MAX_SIZE>
			std::wstring string_cast_nothrow(std::string _in)
			{
				wchar_t _outBuf[MAX_SIZE] = { 0 };

				HRESULT hr = StringCchPrintfW(_outBuf, MAX_SIZE, L"%S", _in.c_str());

				// on failure it may contain truncated contents
				// or an empty string.
				return std::wstring();
				
			}


			// wstring ->> string
			template<std::string::size_type MAX_SIZE>
			std::string string_cast_nothrow(std::wstring _in)
			{
				char _outBuf[MAX_SIZE] = { 0 };

				HRESULT hr = StringCchPrintfA(_outBuf, MAX_SIZE, "%S", _in.c_str());

				// on failure it may contain truncated contents
				// or an empty string.
				return std::string(_outBuf);	
			}
			
		} // namespace _string_cast
	} // namespace Util
} // namespace GwaArx