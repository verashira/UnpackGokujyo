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

#include <Shlwapi.h>

#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <memory>

#include "gokujyo_pack.h"
#include "lzss_var.h"
#include "utility.h"


namespace gkj
{


bool Gpackage::open(const WCHAR *filename)
{
    HANDLE pack = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == pack) {
        return false;
    }

    /* packfile format:
     *       4          4
     *       [ARC2][numberOfEntries][Entry Table]
     */

    vector<Entry> entries;

    DWORD number_of_bytes_Read = 0;

    BYTE sign_buffer[4];
    ReadFile(pack, (LPVOID)sign_buffer, 4, &number_of_bytes_Read, NULL);
    if (std::memcmp((LPVOID)sign_buffer, (LPVOID)kPackFileSign, 4)) {
        CloseHandle(pack);
        return false;
    }

    UINT number_of_entries = 0;
    ReadFile(pack, (LPVOID)&number_of_entries, 4, &number_of_bytes_Read, NULL);
    if (number_of_bytes_Read < 4) {
        CloseHandle(pack);
        return false;
    }

    if (!read_entries(pack, number_of_entries)) {
        CloseHandle(pack);
        return false;
    }

    pack_file_ = pack;
    return true;
}


void Gpackage::close()
{
    if (is_opened())
        CloseHandle(pack_file_);
    entries_.clear();
}


bool Gpackage::is_opened()
{
    return pack_file_ != NULL && pack_file_ != INVALID_HANDLE_VALUE;
}


int Gpackage::num_of_entires() const
{
    return entries_.size();
}


const Gpackage::Entry& Gpackage::get_entry(int index) const
{
    return entries_[index];
}


bool Gpackage::extract_all(const WCHAR *path)
{
    for (auto &e : entries_) {
        decode_entry(e, path);
    }
    return true;
}

bool Gpackage::decode_entry(const Entry &e, const WCHAR *path)
{
    DWORD number_of_bytes_read;

    // std::unique_ptr<BYTE []> compr may be a better choice.
    BYTE *compr = new BYTE[e.compr_len];
    SetFilePointer(pack_file_, e.offset, NULL, FILE_BEGIN);
    if (!ReadFile(pack_file_, compr, e.compr_len, &number_of_bytes_read, NULL)) {
        delete[] compr;
        return false;
    }

    BYTE *uncompr = new BYTE[e.uncompr_len];
    if (!decode(compr, uncompr, e) ||
        !write_file(path, e.filename, uncompr, e.uncompr_len)) {
        delete[] compr;
        delete[] uncompr;
        return false;
    }

    delete[] compr;
    delete[] uncompr;
    return true;
}

bool Gpackage::decode(const BYTE *compr, BYTE *uncompr, const Entry &e)
{
    UINT un_len = 0;
    if (!std::memcmp(compr, kUncomprSign, 4)) {
        BYTE *p = uncompr;
        std::for_each(compr, compr + e.compr_len, [&p](BYTE b)->void{ *(p++) = ~b; });
        un_len = p - uncompr;
    }
    else {
        un_len = lzss_var_decomp(uncompr, e.uncompr_len, compr, e.compr_len);
    }

    if (un_len < e.uncompr_len)
        return false;

    return true;
}

bool Gpackage::write_file(const WCHAR *path, const BYTE *filename, const BYTE *uncompr, UINT uncompr_len)
{
    WCHAR wfilename[MAX_FILENAME_LEN * 3 + MAX_PATH];
    size_t path_len = wcsnlen_s(path, MAX_PATH);
    wcscpy_s(wfilename, MAX_PATH-1, path);
    wcscpy_s(wfilename + path_len, 2, L"\\");

    if (!ansi_shift_jis_to_utf16((CHAR*)filename, wfilename+path_len+1, MAX_FILENAME_LEN * 3))
        return false;

    WCHAR *sep = wcsrchr((WCHAR *)wfilename, L'\\');
    HANDLE un_file = INVALID_HANDLE_VALUE;
    if (sep != nullptr) {
        WCHAR temp_char = *sep;
        *sep = L'\0';
        create_directory_r(wfilename);
        *sep = temp_char;
    }
    un_file = CreateFileW(wfilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

    if (un_file == INVALID_HANDLE_VALUE) 
        return false;

    DWORD number_of_bytes_written;
    if (!WriteFile(un_file, uncompr, uncompr_len, &number_of_bytes_written, NULL))
        return false;
    return true;
}


bool Gpackage::read_entries(HANDLE file, int number_of_entires)
{
    /* [entries in Entry Table]
    *     4         4                   1           [filename Length]
    * [Offset][Uncompressed Length][filename Length][    filename   ]
    *                                                 NOT - operated
    */

    entries_.clear();

    BYTE filename_len = 0;
    BYTE filename[MAX_FILENAME_LEN];
    UINT offset = 0;
    UINT uncompr_len = 0;

    DWORD number_of_bytes_read;
    for (int i = 0; i < number_of_entires; ++i) {
        if (!ReadFile(file, (LPVOID)&offset, 4, &number_of_bytes_read, NULL) ||
            !ReadFile(file, (LPVOID)&uncompr_len, 4, &number_of_bytes_read, NULL) ||
            !ReadFile(file, (LPVOID)&filename_len, 1, &number_of_bytes_read, NULL) ||
            !ReadFile(file, (LPVOID)filename, filename_len, &number_of_bytes_read, NULL)) {
            // Fail to read entry.
            entries_.clear();
            return false;
        }

        // Try C++11 but the IntelliSense here is still bad.
        Entry e{ offset, 0, uncompr_len, filename_len };

        for (int k = 0; k < filename_len; ++k){
            filename[k] = ~filename[k];
        }
        filename[filename_len] = 0;
        CopyMemory(e.filename, filename, filename_len);

        entries_.push_back(e);
    }

    // Every compressed file length is calculated by the offset of adjacent two files.
    calc_compr_len(file);
    return true;
}

void Gpackage::calc_compr_len(HANDLE file)
{
    for (auto iter = entries_.begin(); iter != entries_.end() - 1; ++iter) {
        iter->compr_len = (iter + 1)->offset - iter->offset;
    }
    entries_.back().compr_len = GetFileSize(file, NULL) - entries_.back().offset;
}

} // namespace gkj
