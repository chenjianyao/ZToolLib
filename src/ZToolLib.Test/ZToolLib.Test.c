#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <ZToolLib/ztl_atomic.h>
#include <ZToolLib/ztl_buffer.h>
#include <ZToolLib/ztl_common.h>
#include <ZToolLib/ztl_config.h>
#include <ZToolLib/ztl_logger.h>
#include <ZToolLib/lockfreequeue.h>

#include <ZToolLib/ztl_base64.h>

#include <ZToolLib/ztl_utils.h>


void test_ztl_config();
void test_ztl_log();

void test_lfqueue();

void test_base64();

void test_read_file();

void test_char_conv();


int main(int argc, char* argv[])
{
    ZTL_NOTUSED(argc);
    ZTL_NOTUSED(argv);

    //test_ztl_config();
    //test_ztl_log();

    //test_lfqueue();

    //test_base64();

    // test_read_file();

    test_char_conv();

    return 0;
}

void test_ztl_config()
{
    ztl_config_t* zconf;
    zconf = ztl_config_open("ztl_test_config.txt", '#', ' ');
    assert(zconf);

    char* lpval = NULL;
    ztl_config_read_str(zconf, "ip", &lpval, NULL);

    uint16_t port = 0;
    ztl_config_read_int16(zconf, "port", &port);

    uint32_t lnum = 0;
    ztl_config_read_int32(zconf, "num", &lnum);

    ztl_config_close(zconf);
}

void test_ztl_log()
{
    ztl_log_t* log1, *log2;
    log1 = ztl_log_create("test_ztl_log1.log", ZTL_PrintScrn, false);

    log2 = ztl_log_create("test_ztl_log2.log", ZTL_WritFile, true);

    ztl_log_set_level(log1, ZTL_LOG_DEBUG);
    ztl_log_set_level(log2, ZTL_LOG_INFO);

    ztl_log_info(log1, "hello log %d", 1);
    ztl_log_warn(log2, "hello log 2");
    getchar();

    ztl_log_close(log1);
    ztl_log_close(log2);
}


void test_lfqueue()
{
    typedef struct  
    {
        void* handle;
        void* arg;
    }ztest_data_t;
    lfqueue_t* que;
    que = lfqueue_create(4, sizeof(ztest_data_t));

    ztest_data_t ldata;
    ldata.handle = (void*)1;
    ldata.arg = (void*)2;
    lfqueue_push(que, &ldata);

    ldata.handle = (void*)3;
    ldata.arg = (void*)4;
    lfqueue_push(que, &ldata);

    lfqueue_pop(que, (void**)&ldata);
    lfqueue_pop(que, (void**)&ldata);

}

void test_base64()
{
    char lString[256] = "111111";
    char* lpChanged, *lpRaw;

    char lBase64String[256] = "";
    char lRawString[256] = "";
    uint32_t lBase64Length, lRawLength;

#if 0
    lpChanged = zpassword_change(lString);
    lpRaw = zpassword_change(lpChanged);

    assert(strcmp(lpRaw, lString) == 0);

    char lBase64String[256] = "";
    uint32_t lBase64Length = sizeof(lBase64String) - 1;
    ztl_base64_encode(lpRaw, strlen(lpRaw), lBase64String, &lBase64Length);

    char lRawString[256] = "";
    uint32_t lRawLength = sizeof(lRawString) - 1;
    ztl_base64_decode(lBase64String, lBase64Length, lRawString, &lRawLength);

    assert(strcmp(lRawString, lString) == 0);

    printf("%s\n", lRawString);

#endif

    // change and encode
    lpChanged = zpassword_change(lString);
    lBase64Length = sizeof(lBase64String) - 1;
    ztl_base64_encode(lpChanged, strlen(lpChanged), lBase64String, &lBase64Length);

    // decode and change
    char lTemp[256] = "";
    strcpy(lTemp, lBase64String);
    lRawLength = sizeof(lRawString) - 1;
    ztl_base64_decode(lTemp, lBase64Length, lRawString, &lRawLength);
    lpRaw = zpassword_change(lRawString);

    assert(strcmp(lpRaw, "111111") == 0);
}

void test_read_file()
{
    typedef struct {
        char name[16];
        int  age;
        double score;
    }test_rf_st;

    const char* filename = "test_read.txt";
    char buf1[16] = "";
    read_file_content(filename, buf1, sizeof(buf1) - 1);
    printf("read: %s\n", buf1);

    const char* filename2 = "test_read_ex.txt";
    test_rf_st rfst = { 0 };
    strcpy(rfst.name, "yizhe");
    rfst.age = 29;
    rfst.score = 7.6;
    FILE* fp = fopen(filename2, "w+");
    if (fp)
    {
        fwrite(&rfst, sizeof(rfst), 1, fp);
        fclose(fp);
    }

    test_rf_st rfst1 = { 0 };
    read_file_content(filename2, (char*)&rfst1, 18);

    test_rf_st rfst2 = { 0 };
    read_file_content(filename2, (char*)&rfst2, sizeof(rfst2));
    printf("rfst2: %s,%d,%lf\n", rfst2.name, rfst2.age, rfst2.score);
}

static const uint32_t powers_of_10_32[] = {
    UINT32_C(0),          UINT32_C(10),       UINT32_C(100),
    UINT32_C(1000),       UINT32_C(10000),    UINT32_C(100000),
    UINT32_C(1000000),    UINT32_C(10000000), UINT32_C(100000000),
    UINT32_C(1000000000)
};

static inline uint32_t to_chars_len(int value)
{
    // FIXME error
    // const unsigned t = (32 - __builtin_clz(value | 1)) * 1233 >> 12;
    const unsigned t = 0;
    return t - (value < powers_of_10_32[t]) + 1;
}

void test_char_conv()
{
    for (int i = 0; i < 10; ++i)
    {
        int x = powers_of_10_32[i];
        printf("%d,%d\n", x, to_chars_len(x));
    }
    printf("\n");
}

