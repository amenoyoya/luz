#define UNICODE
#define SOL_ALL_SAFETIES_ON 1
#define SOL_LUAJIT 1
#include <sol/sol.hpp>

#ifdef _MSC_VER
    #include <io.h> // _setmode
    #include <fcntl.h> // _O_U16TEXT
    
    #pragma comment(lib, "lua51.lib")

    #define __export __declspec(dllexport)
#else
    #define __export __attribute__((visibility ("default")))
#endif

extern "C" {
    __export bool u8towcs(wchar_t *dest, const char *source, size_t size) {
        const unsigned char nMaxReadSize = 6;
        char      chBuffer[nMaxReadSize], *src = (char*)source;
        size_t    nReadDataSize = 0, i = 0;
        long      iCh1, sizeBytes = 0;
        wchar_t   wcWork1 = 0, wcWork2 = 0, wcWork3 = 0;
        // BOMの除去
        if (size > 2 && ((*src == '\xef') && (*(src + 1) == '\xbb') && (*(src + 2) == '\xbf'))) {
            src += 3;
            size -= 3;
        }
        for (size_t cursor = 0; cursor < size;) {
            /* srcより6バイトのデータを読み出し */
            nReadDataSize = (nMaxReadSize < (size - cursor))? nMaxReadSize: (size - cursor);
            memcpy(chBuffer, src + cursor, nReadDataSize);
            memset(chBuffer + nReadDataSize, 0, sizeof(chBuffer) - nReadDataSize);
            /* data size を調べる */
            iCh1 = ((long)(*chBuffer)) & 0x00ff;
            iCh1 = ~iCh1;    /* ビット反転 */
            if (iCh1 & 0x0080) sizeBytes = 1; /* 0aaabbbb */
            else if (iCh1 & 0x0040) return false; /* error */
            else if (iCh1 & 0x0020) sizeBytes = 2; /* 110aaabb 10bbcccc */
            else if (iCh1 & 0x0010) sizeBytes = 3; /* 1110aaaa 10bbbbcc 10ccdddd */
            else if (iCh1 & 0x0008) sizeBytes = 4; /* 未対応のマッピング(UTF-16に存在しないコード) */
            else if (iCh1 & 0x0004) sizeBytes = 5; /* 未対応のマッピング(UTF-16に存在しないコード) */
            else if (iCh1 & 0x0002) sizeBytes = 6; /* 未対応のマッピング(UTF-16に存在しないコード) */
            else return false; /* error */
            /* sizeBytes毎に処理を分岐 */
            switch (sizeBytes) {
            case 1:
                /*
                * ビット列
                * (0aaabbbb)UTF-8 ... (00000000 0aaabbbb)UTF-16
                */
                dest[i++] = (wchar_t)(chBuffer[0] & (wchar_t)0x00ff);     /* 00000000 0aaabbbb */
                break;
            case 2:
                /*
                * ビット列
                * (110aaabb 10bbcccc)UTF-8 ... (00000aaa bbbbcccc)UTF-16
                */
                wcWork1 = ((wchar_t)(chBuffer[0])) & (wchar_t)0x00ff;   /* 00000000 110aaabb */
                wcWork2 = ((wchar_t)(chBuffer[1])) & (wchar_t)0x00ff;   /* 00000000 10bbcccc */
                wcWork1 <<= 6;                                          /* 00110aaa bb?????? */
                wcWork1 &= 0x07c0;                                      /* 00000aaa bb000000 */
                wcWork2 &= 0x003f;                                      /* 00000000 00bbcccc */
                dest[i++] = wcWork1 | wcWork2;                          /* 00000aaa bbbbcccc */
                break;
            case 3:
                /*
                * ビット列
                * (1110aaaa 10bbbbcc 10ccdddd)UTF-8 ... (aaaabbbb ccccdddd)UTF-16
                */
                wcWork1 = ((wchar_t)(chBuffer[0])) & (wchar_t)0x00ff;   /* 00000000 1110aaaa */
                wcWork2 = ((wchar_t)(chBuffer[1])) & (wchar_t)0x00ff;   /* 00000000 10bbbbcc */
                wcWork3 = ((wchar_t)(chBuffer[2])) & (wchar_t)0x00ff;   /* 00000000 10ccdddd */
                wcWork1 <<= 12;                                         /* aaaa???? ???????? */
                wcWork1 &= 0xf000;                                      /* aaaa0000 00000000 */
                wcWork2 <<= 6;                                          /* 0010bbbb cc?????? */
                wcWork2 &= 0x0fc0;                                      /* 0000bbbb cc000000 */
                wcWork3 &= 0x003f;                                      /* 00000000 00ccdddd */
                dest[i++] = wcWork1 | wcWork2 | wcWork3;                /* aaaabbbb ccccdddd */
                break;
            case 4:
            case 5:
            case 6:
            default:
                /* ダミーデータ(uff1f)を出力 */
                dest[i++] = (wchar_t)0xff1f;
                break;
            }
            cursor += sizeBytes;
        }
        return true;
    }
    
    __export bool wcstou8(char *dest, const wchar_t *source, size_t size) {
        wchar_t   wcWork1 = 0, *src = (wchar_t*)source;
        size_t    i = 0;
        long      sizeBytes = 0;
        char      chWork1 = 0, chWork2 = 0, chWork3 = 0;
        
        for (size_t cursor = 0; cursor < size; ++cursor) {
            /* srcより1ワードのデータを読み出し */
            if (0 == (wcWork1 = *(src + cursor))) return true;
            else if((wcWork1 <= ((wchar_t)0x007f))) sizeBytes = 1; /* 0x0000 to 0x007f */
            else if((((wchar_t)0x0080) <= wcWork1) && (wcWork1 <= ((wchar_t)0x07ff))) sizeBytes = 2; /* 0x0080 to 0x07ff */
            else if((((wchar_t)0x0800) <= wcWork1)) sizeBytes = 3; /* 0x0800 to 0xffff */
            else return false; /* error */
            /* sizeBytes毎に処理を分岐 */
            switch (sizeBytes) {
            case 1:
                /*
                * ビット列
                * (0aaabbbb)UTF-8 ... (00000000 0aaabbbb)UTF-16
                */
                dest[i++] = (char)wcWork1;              /* 0aaabbbb */
                break;
            case 2:
                /*
                * ビット列
                * (110aaabb 10bbcccc)UTF-8 ... (00000aaa bbbbcccc)UTF-16
                */
                chWork1 = (char)(wcWork1 >> 6);     /* 000aaabb */
                chWork1 |= (char)0xc0;              /* 110aaabb */
                chWork2 = (char)wcWork1;            /* bbbbcccc */
                chWork2 &= (char)0x3f;              /* 00bbcccc */
                chWork2 |= (char)0x80;              /* 10bbcccc */
                dest[i++] = chWork1;
                dest[i++] = chWork2;
                break;
            case 3:
                /*
                * ビット列
                * (1110aaaa 10bbbbcc 10ccdddd)UTF-8 ... (aaaabbbb ccccdddd)UTF-16
                */
                chWork1 = (char)(wcWork1 >> 12);    /* ????aaaa */
                chWork1 &= (char)0x0f;              /* 0000aaaa */
                chWork1 |= (char)0xe0;              /* 1110aaaa */
                chWork2 = (char)(wcWork1 >> 6);     /* aabbbbcc */
                chWork2 &= (char)0x3f;              /* 00bbbbcc */
                chWork2 |= (char)0x80;              /* 10bbbbcc */
                chWork3 = (char)wcWork1;            /* ccccdddd */
                chWork3 &= (char)0x3f;              /* 00ccdddd */
                chWork3 |= (char)0x80;              /* 10ccdddd */
                dest[i++] = chWork1;
                dest[i++] = chWork2;
                dest[i++] = chWork3;
                break;
            default:
                break;
            }
        }
        return true;
    }

    __export size_t io_write(FILE *fp, const wchar_t *format, ...) {
        va_list args;
        va_start(args, format);

        size_t written = vfwprintf(fp, format, args);
        va_end(args);
        return written;
    }

    #ifdef _MSC_VER
        __export void setu16mode(FILE *fp) {
            _setmode(_fileno(fp), _O_U16TEXT);
        }
    #endif
}

int main(int argc, char *argv[]) {
    sol::state lua;

    lua.open_libraries(
        sol::lib::base,
        sol::lib::coroutine,
        sol::lib::package,
        sol::lib::string,
        sol::lib::table,
        sol::lib::io,
        sol::lib::math,
        sol::lib::os,
        sol::lib::debug,
        sol::lib::ffi,
        sol::lib::bit32,
        sol::lib::jit
    );

    /// main script: argv[1].lua
    if (argc < 2) {
        fprintf(stderr, "%s\n", "no main script file");
        return 1;
    }
    auto result = lua.safe_script_file(argv[1]);
    if (result.valid()) return 0;

    sol::error err = result;
    fprintf(stderr, "%s\n", err.what());
    return 1;
}