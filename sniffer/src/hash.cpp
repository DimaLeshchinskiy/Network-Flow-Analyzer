#include "hash.h"

namespace hash
{
    namespace math
    {
        uint32_t rightRotate(uint32_t n, uint32_t shift_by)
        {
            return (n >> shift_by) | (n << (32 - shift_by));
        }

        void endianSwap(uint32_t &x)
        {
            x = (x >> 24) |
                ((x << 8) & 0x00FF0000) |
                ((x >> 8) & 0x0000FF00) |
                (x << 24);
        }

        void toHex(char *hex, uint32_t *n)
        {
            char table[] = "0123456789abcdef";
            char *byte_repre = (char *)n;
            for (int i = 0; i < 4; i++)
            {
                char byte = byte_repre[i];
                char lowBits = byte & 0x0F;
                char highBits = (byte >> 4) & 0x0F;

                hex[8 - (2 * i + 2)] = table[highBits];
                hex[8 - (2 * i + 1)] = table[lowBits];
            }
        }

        int hex2int(char &ch)
        {
            if (ch >= '0' && ch <= '9')
                return ch - '0';
            if (ch >= 'A' && ch <= 'F')
                return ch - 'A' + 10;
            if (ch >= 'a' && ch <= 'f')
                return ch - 'a' + 10;
            return -1;
        }

        // return: 0 -> equals; 1 -> hex1 > hex2; -1 -> hex1 < hex2
        int compareHex(char *hex1, char *hex2, size_t size)
        {
            for (int i = 0; i < size; i++)
            {
                int a = hex2int(hex1[i]);
                int b = hex2int(hex2[i]);

                if (a == b)
                    continue;

                if (a > b)
                    return 1;
                else
                    return -1;
            }
            return 0;
        }
    };

    auto transform(const char *arr, size_t size)
    {
        size_t transform_str_size = size - 1;                                               // remove last character(\0)
        transform_str_size++;                                                               // append 1 on end
        transform_str_size += BYTES_PER_BLOCK - (transform_str_size % BYTES_PER_BLOCK) - 8; // append 0 to end
        transform_str_size += 8;                                                            // append last 64 bits = len(str)

        char *transform_str = (char *)malloc(transform_str_size);

        // copy str
        uint32_t i = 0;
        for (; i < size - 1; i++)
        {
            transform_str[i] = arr[i];
        }
        // append 1
        transform_str[i] = ONE;

        // fill 0 to end
        uint32_t count_0 = BYTES_PER_BLOCK - (i % BYTES_PER_BLOCK) - 8;
        for (uint32_t j = 0; j < count_0; j++)
        {
            transform_str[++i] = ZERO;
        }

        // append last 64 bits = len(str)
        long long len = (size - 1) * 8; // count of bits
        for (uint32_t j = 0; j < BYTES_FOR_LENGTH; j++)
        {
            transform_str[i + j] = (len >> (8 * (BYTES_FOR_LENGTH - j - 1))) & 0xFF;
        }

        struct retVals
        { // Declare a local structure
            size_t size;
            char *arr;
        };

        return retVals{(size_t)(i + 8), transform_str};
    }

    uint32_t *getBlock(char *_str, uint32_t index)
    {
        uint32_t *block = (uint32_t *)calloc(BYTES_PER_BLOCK / 4 + 48, sizeof(uint32_t)); // block_len + 48 words of 32bit len
        uint32_t *str = (uint32_t *)_str;

        // swap from big indian to little indian
        for (uint32_t i = 0; i < BYTES_PER_BLOCK / 4; i++)
        {
            block[i] = str[i + (index * BYTES_PER_BLOCK / 4)];
            math::endianSwap(block[i]);
        }

        return block;
    }

    void hash(char *result, char *_str, size_t size)
    {

        uint32_t h0 = 0x6a09e667;
        uint32_t h1 = 0xbb67ae85;
        uint32_t h2 = 0x3c6ef372;
        uint32_t h3 = 0xa54ff53a;
        uint32_t h4 = 0x510e527f;
        uint32_t h5 = 0x9b05688c;
        uint32_t h6 = 0x1f83d9ab;
        uint32_t h7 = 0x5be0cd19;
        uint32_t k[64] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
                          0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
                          0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
                          0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
                          0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
                          0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
                          0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
                          0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

        auto retValues = transform(_str, size);
        char *transformed = retValues.arr;
        size_t transformed_size = retValues.size;

        uint32_t blocks_count = transformed_size / BYTES_PER_BLOCK;
        for (uint32_t i = 0; i < blocks_count; i++)
        {
            uint32_t *block = getBlock(transformed, i);

            for (uint32_t j = 16; j < 64; j++)
            {
                uint32_t s0 = math::rightRotate(block[j - 15], 7) ^ math::rightRotate(block[j - 15], 18) ^ (block[j - 15] >> 3);
                uint32_t s1 = math::rightRotate(block[j - 2], 17) ^ math::rightRotate(block[j - 2], 19) ^ (block[j - 2] >> 10);
                block[j] = block[j - 16] + s0 + block[j - 7] + s1;
            }

            uint32_t a = h0;
            uint32_t b = h1;
            uint32_t c = h2;
            uint32_t d = h3;
            uint32_t e = h4;
            uint32_t f = h5;
            uint32_t g = h6;
            uint32_t h = h7;

            for (uint32_t j = 0; j < 64; j++)
            {
                uint32_t S1 = math::rightRotate(e, 6) ^ math::rightRotate(e, 11) ^ math::rightRotate(e, 25);
                uint32_t ch = (e & f) ^ ((~e) & g);
                uint32_t temp1 = h + S1 + ch + k[j] + block[j];
                uint32_t S0 = math::rightRotate(a, 2) ^ math::rightRotate(a, 13) ^ math::rightRotate(a, 22);
                uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                uint32_t temp2 = S0 + maj;
                h = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c;
                c = b;
                b = a;
                a = temp1 + temp2;
            }

            h0 += a;
            h1 += b;
            h2 += c;
            h3 += d;
            h4 += e;
            h5 += f;
            h6 += g;
            h7 += h;

            free(block);
        }
        free(transformed);

        math::toHex(result + 0, &h0);
        math::toHex(result + 8, &h1);
        math::toHex(result + 16, &h2);
        math::toHex(result + 24, &h3);
        math::toHex(result + 32, &h4);
        math::toHex(result + 40, &h5);
        math::toHex(result + 48, &h6);
        math::toHex(result + 56, &h7);

        result[64] = 0;
    }
}

char *hashIP(uint32_t ipSrc, uint16_t portSrc, uint32_t ipDest, uint16_t portDest){
    std::string input = std::to_string(ipSrc);
    input.append(std::to_string(ipDest));
    input.append(std::to_string(portSrc));
    input.append(std::to_string(portDest));

    char result_hash[65]; // last char is 0; first hash

    size_t buf_len =  input.length();
    char *buffer = new char[buf_len];

    for(int i = 0; i < buf_len; i++){
        buffer[i] = input.at(i);
    }

    hash::hash(result_hash, buffer, buf_len);
    return result_hash;
}