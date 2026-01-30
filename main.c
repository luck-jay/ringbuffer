#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ring_buffer.h"

#define  TEST(t, ok_msg, f_msg) \
    if (t) { \
        printf("[PASS]: " ok_msg "\r\n"); \
    } \
    else { \
        printf("[FAILED]: " f_msg "\r\n"); \
        goto _err; \
    }

int main(int argc, char *argv[]) {
    ring_t ring;
    uint8_t buf[8];

    srand((unsigned)time(NULL));

    /* 测试初始化 */
    TEST(
        ring_buffer_init(&ring, buf, 8, 2) == 0,
        "Init ring buffer successful!",
        "Init ring buffer failed!"
        );

    /* 测试单次读写 */
    for (int i = 0; i < 8; i++) {
        TEST(
        ring_put(&ring, &i, 1) == (i > 3 ? 0 : 1),
        "Write one data successful!",
        "Write one data failed"
        );
        printf("Write data %d successful\n", i);
    }
    for (int i = 0; i < 8; i++) {
        uint16_t tmp;
        TEST(
            ring_get(&ring, &tmp, 1) == (i > 3 ? 0 : 1),
            "Read one data successful!",
            "Read one data failed"
            );
        printf("Read data %d successful\n", tmp);
    }
    /* 随机数量读写测试 */
    for (int i = 0; i < 10000; ++i) {
        size_t const write_len = rand() % 4 + 1;
        uint8_t *tmp = malloc(write_len);

        for (int j = 0; j < write_len; j++) {
            ((uint16_t *)tmp)[j] = i;
        }

        TEST(
            ring_put(&ring, tmp, write_len) == write_len,
            "Write block successful",
            "Write block failed"
            );
        printf("Write data size %llu\n", write_len);
        TEST(
            ring_get(&ring, tmp, write_len) == write_len,
            "Read block successful",
            "Read block failed"
            );
        printf("Read data: ");
        for (int j = 0; j < write_len; j++) {
            printf("%d ", ((uint16_t *)tmp)[j]);
        }
        printf("\n");
    }

    /* 测试写入较多的数据 */
    /* 测试单次读写 */
    for (int i = 0; i < 16; i++) {
        TEST(
        ring_put(&ring, &i, 1) == (i > 3 ? 0 : 1),
        "Write one data successful!",
        "Write one data failed"
        );
        printf("Write data %d successful\n", i);
    }
    printf("write data: ");
    for (int i = 0; i < 16; i++) {
        uint16_t tmp;
        if (ring_get(&ring, &tmp, 1) == 0)
            break;
        printf("%d ", tmp);
    }
    printf("\n");

    /* 索引超限测试 */
    ring.head = UINT32_MAX - 4;
    ring.tail = UINT32_MAX - 4;
    /* 随机数量读写测试 */
    for (int i = 0; i < 10000; ++i) {
        size_t const write_len = rand() % 4 + 1;
        uint8_t *tmp = malloc(write_len);

        for (int j = 0; j < write_len; j++) {
            ((uint16_t *)tmp)[j] = i;
        }

        TEST(
            ring_put(&ring, tmp, write_len) == write_len,
            "Write block successful",
            "Write block failed"
            );
        printf("Write data size %llu, head %u, tail %u \n", write_len, ring.head, ring.tail);
        printf("free space %llu, used %llu \n", ring_space(&ring), ring_used(&ring));
        TEST(
            ring_get(&ring, tmp, write_len) == write_len,
            "Read block successful",
            "Read block failed"
            );
        printf("head %u, tail %u Read data:", ring.head, ring.tail);
        for (int j = 0; j < write_len; j++) {
            printf("%d ", ((uint16_t *)tmp)[j]);
        }
        printf("\n");
    }

    printf("Test Pass!\n");

    return 0;
_err:
    printf("Test Failed!\n");
}
