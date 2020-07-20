#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <locale.h>
#include <wchar.h>
#include "han.h"

#define PAD_CHAR 2324 //흐

void puthg(short num);

static int wrap = 40, wrapcount = 0;

const char usage[] = "Usage: basehangul [-d] [-w\x1b[3mwrap\x1b[0m] \x1b[3mfilename\x1b[0m\n";

int main(int argc, char *argv[])
{
    int c, ind = 0, numBytes = 5, opt;
    wint_t ch;
    uint8_t buf[5];
    uint16_t hgbuf[4];
    char decode = 0;
    FILE *f;
    opterr = 0; // getopt 에러 메시지 비활성화

    setlocale(LC_ALL, ""); // 환경 변수와 같이 로케일 설정 wide character를 위해 필요함

    // 옵션 파싱
    // -d는 디코드
    // -w로 line wrap 설정 0은 비활성화
    // 파일 이름이 없거나 - 이면 stdin/stdout 사용
    while ((opt = getopt(argc, argv, "w:d")) != -1) {
        switch (opt) {
            case 'w':
                wrap = atoi(optarg);
                break;
            case 'd':
                decode = 1;
                break;
            case '?':
                fputs(usage, stderr);
                exit(-1);
        }
    }
    if (optind == argc - 1) {
        if (!strcmp(argv[optind], "-"))
            f = stdin;
        else
            f = fopen(argv[optind], "r");

        if (f == NULL) {
            perror("open failed");
            exit(-1);
        }
    } else if (optind == argc) {
        f = stdin;
    } else {
        fputs(usage, stderr);
        exit(-1);
    }

    if (!decode) {
        while (EOF != (c = fgetc(f))) {
            buf[ind++] = (char) c;
            ind %= 5;
            if (ind == 0) {
                hgbuf[0] = 0x3FF & (buf[0]<<2 | buf[1]>>6);
                hgbuf[1] = 0x3FF & (buf[1]<<4 | buf[2]>>4);
                hgbuf[2] = 0x3FF & (buf[2]<<6 | buf[3]>>2);
                hgbuf[3] = 0x3FF & (buf[3]<<8 | buf[4]);
                for (int i = 0; i < 4; i++)
                    puthg(hgbuf[i]);
            }
        }
        if (ind > 0) {
            for(int i = ind; i < 5; i++)
                buf[i] = 0;
            for(int i = ind; i < 4; i++)
                hgbuf[i] = PAD_CHAR;
            switch (ind) {
                case 4:
                    hgbuf[3] = (buf[3] & 3) + 1024;
                case 3:
                    hgbuf[2] = 0x3FF & (buf[2]<<6 | buf[3]>>2);
                case 2:
                    hgbuf[1] = 0x3FF & (buf[1]<<4 | buf[2]>>4);
                case 1:
                    hgbuf[0] = 0x3FF & (buf[0]<<2 | buf[1]>>6);
            }
            for (int i = 0; i < 4; i++)
                puthg(hgbuf[i]);
        }
        fputwc('\n', stdout);
    } else {
        while (WEOF != (ch = fgetwc(f))) {
            if (ch == '\n' || ch == '\r')
                continue;
            hgbuf[ind++] = hg_getidx(ch);
            ind %= 4;
            if (!ind) {
                buf[0] = (uint8_t) (hgbuf[0] >> 2);
                buf[1] = (uint8_t) (hgbuf[0] << 6 | (0x3FF & hgbuf[1]) >> 4);
                buf[2] = (uint8_t) (hgbuf[1] << 4 | (0x3FF & hgbuf[2]) >> 6);
                buf[3] = (uint8_t) (hgbuf[2] << 2 | (0x3FF & hgbuf[3]) >> 8);
                buf[4] = (uint8_t) (hgbuf[3]);
                numBytes = 5;
                for (int i = 3; i > 0; i--)
                    if (hgbuf[i] != PAD_CHAR) {
                        if (i != 3)
                            numBytes = i+1;
                        break;
                    }
                if ((hgbuf[3] & 0x7FC) == 0x400) {
                    numBytes = 4;
                    buf[3] = (char) (hgbuf[2] << 2 | (0x3 & hgbuf[3]));
                } else
                    for (int i = 3; i > 0 && hgbuf[i] == PAD_CHAR; i--)
                        numBytes = i;
                for (int i = 0; i < numBytes; i++)
                    putc(buf[i], stdout);
            }
        }
    }

    fclose(f);

    return 0;
}


void puthg(short num)
{
    fputwc(han2350[num%2350]+0xac00, stdout);
    if (wrap && ++wrapcount == wrap) {
        fputwc('\n', stdout);
        wrapcount = 0;
    }
}

// vim: ft=c ts=4 sw=4 et nu rnu
