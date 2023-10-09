#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

#define LINE_WRAP 80

void prt_arr_elm(FILE *f, int n, char *fmt);

int main(int argc, char *argv[])
{
    FILE *f;
    wchar_t buf[2351];
    int idx[100], idx_pos = 1, idx_priv = 0;

    setlocale(LC_ALL, "");

    if (NULL == (f = fopen(argv[1], "r"))) {
        perror("파일 열기 오류");
        exit(-1);
    }

    fgetws(buf, 2351, f);

    fclose(f);


    for (int i = 0; i < 2350; i++)
        if ((buf[i] - 0xac00) >> 8 != idx_priv) {
            idx_priv = (buf[i] - 0xac00) >> 8;
            idx[idx_pos++] = i;
        }
    idx[idx_pos++] = 2350;


    if (NULL == (f = fopen("han.h", "w"))) {
        perror("han.h 열기 오류");
        exit(-1);
    }

    fprintf(f, "#ifndef __HAN2350_H\n\
#define __HAN2350_H\n\
#include <stdint.h>\n\
#define HAN_IDX_LEN %d\n\
extern int hg_getidx(int hgch);\n\
extern const uint16_t han_idx[%d];\n\
extern const uint16_t han2350[2350];\n\
#endif\n", idx_pos - 1, idx_pos);

    fclose(f);

    
    if (NULL == (f = fopen("han.c", "w"))) {
        perror("han.c 열기 오류");
        exit(-1);
    }

    fputs("\
#include \"han.h\"\n\
\n\
int hg_getidx(int hgch)\n\
{\n\
    int i;\n\
    uint8_t firstByte;\n\n\
    hgch -= 0xac00;\n\
    firstByte = (hgch >> 8) & 0xFF;\n\
    if (firstByte >= HAN_IDX_LEN)\n\
        return -1; // out of range\n\
    for (i = han_idx[firstByte]; i < han_idx[firstByte + 1]; i++)\n\
        if (han2350[i] == hgch)\n\
            return i;\n\
    return -1; // chacter not in KS X 1001\n\
}\n", f);
    fprintf(f, "\nconst uint16_t han_idx[%d] = {\n", idx_pos);
    for (int i = 0; i < idx_pos; i++)
        prt_arr_elm(f, idx[i], NULL);
    fputs("\n};\n", f);

    prt_arr_elm(NULL, 0, NULL);
    fputs("\nconst uint16_t han2350[2350] = {\n", f);
    for (int i = 0; i < 2350; i++)
        prt_arr_elm(f, buf[i] - 0xac00, "0x%04x");
    fputs("\n};\n", f);

    fclose(f);

    return 0;
}


// 80줄 폭에 맞추어 배열 출력
void prt_arr_elm(FILE *f, int n, char *fmt)
{
    static int pos = 0;
    int b2wr;
    char default_fmt[] = "%d";
    if (fmt == NULL)
        fmt = default_fmt;
    if (f == NULL) { // reset position
        pos = 0;
        return;
    }
    if (!pos) {
        pos = fprintf(f, fmt, n);
        fputc(',', f);
        return;
    }
    b2wr = snprintf(NULL, 0, fmt, n) + 2;
    if (pos + b2wr < LINE_WRAP) {
        fputc(' ', f);
        pos += fprintf(f, fmt, n) + 2;
        fputc(',', f);
    } else {
        fputc('\n', f);
        pos = fprintf(f, fmt, n) + 1;
        fputc(',', f);
    }
}
