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

#ifndef UNPACKGOKUJYO_GOKUJYO_PACK_H
#define UNPACKGOKUJYO_GOKUJYO_PACK_H

#include <windows.h>

#include <vector>
#include <string>

#define MAX_FILENAME_LEN 256

namespace gkj
{
    using std::wstring;
    using std::vector;
    using std::string;

    static const BYTE kPackFileSign[] = "ARC2";
    static const BYTE kUncomprSign[] = "\x76\xaf\xb1\xb8";

    class Gpackage
    {
    public:
        struct Entry
        {
            UINT offset;
            UINT compr_len;
            UINT uncompr_len;
            BYTE filename_len;
            BYTE filename[MAX_FILENAME_LEN]; // filename in package is ANSI shift-JIS encoded,
                             // you won't get what you want by just output this
                             // for the default locale.
        };

    public:
        Gpackage() {}
        Gpackage(const Gpackage&) = delete;
        ~Gpackage() {}

        bool open(const WCHAR *filename);
        void close();

        bool is_opened();
        bool decode_entry(const Entry &e, const WCHAR *path);
        bool extract_all(const WCHAR *path = L"");

        int num_of_entires() const;
        const Entry& get_entry(int index) const;

    private:
        bool read_entries(HANDLE file, int number_of_entries);
        void calc_compr_len(HANDLE file);
        bool decode(const BYTE *compr, BYTE *uncopmr, const Entry &e);
        bool write_file(const WCHAR *path, const BYTE *filename, const BYTE *uncompr, UINT uncompr_len);

    private:
        HANDLE pack_file_ = NULL;
        vector<Entry> entries_;
    };
}

#endif // #define UNPACKGOKUJYO_GOKUJYO_PACK_H