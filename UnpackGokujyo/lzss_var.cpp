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

#include "lzss_var.h"

/*
 * A merely modified implemetation of LZSS compression algorithm, with a NOT
 * operation on uncompressed bytes.
 */


/* Decompression:
 *
 * Return: Uncompressed data length.
 *
 * Remind: The implementation here for the rollback of Compression Window 
 *         implicitly acquire the Window Size to be of 2^N.
 *
 * Remark: For the lzss itself,
 *         Window: The most important part of lzss cache the recent bytes, lzss
 *                  memorise these bytes and use these as compression pattern. Window
 *                  is not indefinite and will slide forward for the coming bytes.
 *
 *         Structure:
 *         Compressed data is stored in groups, every group contains 1 flag byte,
 *         and compression data based on the flag.
 *         Flag: 1 Byte, every bit as bool value represent whether this slot is compressed
 *               or not. 1:not 0:compressed
 *         Uncompressed slot: 1 Byte, leave it unchanged as for Standard, but need a
 *               bit-wise not operation in this modified version.
 *         Compressed slot: 2Bytes,
 *               [4bit][4bit][4bit][4bit]
 *               [---off1---][off2][-len]
 *               offset: off2-off1
 *               len: len + 3, data of length less than 3 is not to be compressed,
 *                    so len==0 means a 3 bytes length slot.
 */
UINT lzss_var_decomp(BYTE *uncompr, DWORD uncomprlen, const BYTE *compr, DWORD comprlen)
{
    UINT act_uncomprlen = 0;
    UINT curByte = 0;

    // LZSS
    const static UINT WIND_SIZE = 0x1000;
    BYTE window[WIND_SIZE];
    UINT nCurWindowByte = 0xFEE;
    ZeroMemory(window, WIND_SIZE);

    while (1){
        UINT flags = 0; // flags indicates whether the following 8 bytes are encoded or not, respectively
                        // 1: not-encoded, 0: encoded(offset,length)
        if (curByte >= comprlen){
            return act_uncomprlen;
        }
        flags = compr[curByte++];

        for (int i = 0; i < 8; ++i, flags = flags >> 1){

            if (flags & 1){
                // not-encoded
                if (curByte >= comprlen){
                    return act_uncomprlen;
                }
                if (act_uncomprlen >= uncomprlen){
                    return act_uncomprlen;
                }

                // NOTICE: Here is different from Standard.
                BYTE data = ~compr[curByte++];
                // output to uncompressed data
                uncompr[act_uncomprlen++] = data;
                window[nCurWindowByte++] = data;

                // Only works when WIND_SIZE are 2^n
                // or we should write if(nCurWindowByte==WIND_SIZE) nCurWindwoByte = 0;
                nCurWindowByte &= WIND_SIZE - 1;
            }
            else{
                // encoded
                if (curByte >= comprlen){
                    return act_uncomprlen;
                }
                UINT win_offset = compr[curByte++];
                if (curByte >= comprlen){
                    return act_uncomprlen;
                }
                UINT copy_len = compr[curByte++];

                // the high-4-bits of the copy_len are used for the high-11~8-bits win_offset
                win_offset |= (copy_len >> 4) << 8;
                copy_len &= 0x0F;
                // copy_len == 0 means 3 for length
                copy_len += 3;

                for (UINT i = 0; i < copy_len; ++i){
                    BYTE data;
                    if (curByte >= uncomprlen){
                        return act_uncomprlen;
                    }
                    if (act_uncomprlen >= uncomprlen){
                        return act_uncomprlen;
                    }

                    // NOTICE
                    data = window[(win_offset + i) & (WIND_SIZE - 1)];

                    uncompr[act_uncomprlen++] = data;
                    window[nCurWindowByte++] = data;
                    nCurWindowByte &= WIND_SIZE - 1;
                }
            }
        }
    }
    return act_uncomprlen;
}