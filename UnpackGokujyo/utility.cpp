/*******************************************************************************\
* Copyright 2012-2012 Chenchen Xu                                              *
* All rights reserved.                                                         *
* Redistribution and use in source and binary forms, with or without           *
* modification, are permitted provided that the following conditions are met:  *
*                                                                              *
* 1. Redistributions of source code must retain the above copyright notice,    *
*    this list of conditions and the following disclaimer.                     *
* 2. Redistributions in binary form must reproduce the above copyright notice, *
*    this list of conditions and the following disclaimer in the documentation *
*    and/or other materials provided with the distribution.                    *
* 3. Neither the name of the copyright holder nor the names of its             *
*    contributors may be used to endorse or promote products derived from this *
*    software without specific prior written permission.                       *
*                                                                              *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"  *
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE    *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   *
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE    *
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR          *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF         *
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS     *
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN      *
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)      *
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE   *
* POSSIBILITY OF SUCH DAMAGE.                                                  *
*                                                                              *
* IF WITHOUT ANY FURTHER DECLARATION, EVERY SOURCE CODE FILE IN THIS PROJECT   *
* OF COPYRIGHT TO CHENCHEN XU IS UNDER THE SAME LICENSE DOCUMENTED HERE.       *
\*******************************************************************************/

#include <Windows.h>

#include "utility.h"

bool ansi_shift_jis_to_utf16(const CHAR *src, WCHAR *des, int max_len)
{
    if (max_len <= 0)
        return false;

    des[max_len - 1] = L'\0'; // Ensure it's zero-ended before conversion.

    int wlen = MultiByteToWideChar(932, 0, src, -1, des, max_len);
    if (wlen < 0)
        wlen = 0;

    if (des[max_len - 1] != 0) {
        des[0] = L'\0'; // It's undefined when the buffer is not enough. 
        return false;
    }

    des[wlen] = L'\0';
    return true;
}

void create_directory_r(WCHAR *path)
{
    DWORD file_attr = ::GetFileAttributes(path);
    if (file_attr == INVALID_FILE_ATTRIBUTES) {
        wchar_t *sep = wcsrchr(path, L'\\');
        if (sep != nullptr) {
            wchar_t temp_char = *sep;
            *sep = L'\0';
            create_directory_r(path);
            *sep = temp_char;
        }
        ::CreateDirectory(path, nullptr);
    }

}