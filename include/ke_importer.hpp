/*
 * MIT License
 *
 * Copyright (c) 2023 clauadv
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
vcopies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef ke_importer_hpp
#define ke_importer_hpp

#pragma region includes
#include <ntimage.h>
#pragma endregion includes

#pragma region macros
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#pragma endregion macros

#pragma region defines
#define ke_fn(function) (reinterpret_cast<decltype(&function)>(ke_importer::detail::get_export(ke_importer::hash::get(#function))))
#pragma endregion defines

namespace ke_importer
{
    namespace hash
    {
        __forceinline constexpr u32 get(const char* str, const u32 basis = 0x811c9dc5) noexcept
        {
            auto hash_value{ basis };

            const char* ptr = str;
            for (; *ptr != '\0'; ++ptr)
            {
                hash_value ^= static_cast<const u32>(*ptr++);
                hash_value *= 0x1000193;
            }

            return hash_value;
        }
    }

    namespace detail
    {
        inline u64 m_base{ 0 };

    #ifdef ke_importer_llvm_msvc
        __declspec(naked) __forceinline u64 get_base()
        {
            _asm {
                mov rax, qword ptr gs:[18h]
                mov rcx, [rax+38h]
                mov rax, 0fffffffffffff000h
                and rax, [rcx+4h]
                jmp while_begin
                search_begin:
                add rax, 0fffffffffffff000h
                while_begin:
                xor ecx, ecx
                jmp search_cmp
                search_next:
                add rcx, 1
                cmp rcx, 0ff9h
                jz  search_begin
                search_cmp:
                cmp byte ptr[rax+rcx], 48h
                jnz search_next
                cmp byte ptr[rax+rcx+1], 8dh
                jnz search_next
                cmp byte ptr[rax+rcx+2], 1dh
                jnz search_next
                cmp byte ptr[rax+rcx+6], 0ffh
                jnz search_next
                mov r8d, [rax+rcx+3]
                lea edx, [rcx+r8]
                add edx, eax
                add edx, 7
                test edx, 0fffh
                jnz search_next
                mov rdx, 0ffffffff00000000h
                and rdx, rax
                add r8d, eax
                lea eax, [rcx+r8]
                add eax, 7
                or rax, rdx
                ret
            }
        }
    #elif defined(ke_importer_kmdf)
        extern "C" u64 get_base();
    #else
        // define ke_importer_llvm_msvc or ke_importer_kmdf
    #endif

        __forceinline void* get_export(u64 function_hash)
        {
            if (!ke_importer::detail::m_base)
            {
                ke_importer::detail::m_base = ke_importer::detail::get_base();
            }

            const auto dos_header = reinterpret_cast<const PIMAGE_DOS_HEADER>(ke_importer::detail::m_base);
            const auto nt_header = reinterpret_cast<const PIMAGE_NT_HEADERS64>(ke_importer::detail::m_base + dos_header->e_lfanew);
            const auto export_directory = reinterpret_cast<const PIMAGE_EXPORT_DIRECTORY>(ke_importer::detail::m_base + nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

            const auto address_of_names = reinterpret_cast<const u32*>(ke_importer::detail::m_base + export_directory->AddressOfNames);
            const auto address_of_funcs = reinterpret_cast<const u32*>(ke_importer::detail::m_base + export_directory->AddressOfFunctions);
            const auto address_of_ordinals = reinterpret_cast<const u16*>(ke_importer::detail::m_base + export_directory->AddressOfNameOrdinals);

            for (auto idx{ 0u }; idx < export_directory->NumberOfFunctions; idx++)
            {
                if (!address_of_names[idx] || !address_of_ordinals[idx])
                {
                    continue;
                }

                if (ke_importer::hash::get(reinterpret_cast<const char*>(static_cast<const u64>(ke_importer::detail::m_base) + address_of_names[idx])) == function_hash)
                {
                    return reinterpret_cast<void*>(static_cast<const u64>(ke_importer::detail::m_base) + address_of_funcs[address_of_ordinals[idx]]);
                }
            }

            return nullptr;
        }
    }
}
#endif