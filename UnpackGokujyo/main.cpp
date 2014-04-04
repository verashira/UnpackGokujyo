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


#include <iostream>
#include <vector>
#include <string>

#include "gokujyo_pack.h"

#include "Stackwalker.h"

const WCHAR kUnpackPath[] = L"data\\";

int main()
{
    InitAllocCheck();

    {
        std::string line;

        std::cout << "Press q to quit." << std::endl;
        std::cout << "Please enter the pack file name:";
        while (std::cin >> line && line != "q" && line != "Q") {
            std::cout << "Begin to unpack " << line << std::endl;

            WCHAR pack_filename[MAX_PATH];
            int wlen = MultiByteToWideChar(CP_ACP, 0, line.c_str(), -1, pack_filename, MAX_PATH);
            pack_filename[0] = L'\0';

            WCHAR buf[MAX_PATH];
            wcscpy_s(buf, MAX_PATH, kUnpackPath);
            int unpack_path_len = sizeof(kUnpackPath) / sizeof(kUnpackPath[0]);
            wcscpy_s(buf + unpack_path_len - 1, MAX_PATH - unpack_path_len + 1, pack_filename);

            gkj::Gpackage pack;
            pack.open(pack_filename);
            if (pack.extract_all(buf))
                std::cout << "Success to unpack." << std::endl;
            else
                std::cout << "Fail to unpack." << std::endl;
            pack.close();

            std::cout << "Press q to quit." << std::endl;
            std::cout << "Please enter the pack file name:";
        }
    }

    DeInitAllocCheck();

    return 0;
}
