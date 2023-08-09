
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFSIZE (8 * 1024 * 1024)

uint32_t buffer[BUFSIZE];
uint16_t pixels16[BUFSIZE];
uint32_t pixels32[BUFSIZE];

#define NOISE_SIZE 65536
uint8_t noise[NOISE_SIZE];


typedef struct _xwd_file_header {
    uint32_t header_size;
    uint32_t file_version;
    uint32_t pixmap_format;
    uint32_t pixmap_depth;
    uint32_t pixmap_width;
    uint32_t pixmap_height;
    uint32_t xoffset;
    uint32_t byte_order;
    uint32_t bitmap_unit;
    uint32_t bitmap_bit_order;
    uint32_t bitmap_pad;
    uint32_t bits_per_pixel;
    uint32_t bytes_per_line;
    uint32_t visual_class;
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t bits_per_rgb;
    uint32_t colormap_entries;
    uint32_t ncolors;
    uint32_t window_width;
    uint32_t window_height;
    uint32_t window_x;
    uint32_t window_y;
    uint32_t window_bdrwidth;
} XWDFileHeader;

int width = 0;
int height = 0;

#define DITHER_TO_5_BITS_MUL (31u * (1u << 26) / 255u)
#define DITHER_TO_6_BITS_MUL (63u * (1u << 26) / 255u)

static uint32_t be(uint32_t value) {
    return ((value << 24) | ((value << 8) & 0xFF0000) + ((value >> 8) & 0xFF00) + (value >> 24));
}


void dither_to_15(uint8_t *input, uint16_t *output, int width, int height)
{
    uint8_t *image_end = input + 4 * width * height;
    uint8_t *noise_ptr = noise;
    uint8_t *noise_end = noise_ptr + NOISE_SIZE - 3;
    while (input < image_end) {
        uint8_t *line_end = input + 4 * width;
        while (input < line_end) {
            uint32_t red = input[2];
            red = (DITHER_TO_5_BITS_MUL * red + ((uint32_t)(*noise_ptr++) << 18)) >> 26;
            uint32_t green = input[1];
            green = (DITHER_TO_5_BITS_MUL * green + ((uint32_t)(*noise_ptr++) << 18)) >> 26;
            uint32_t blue = input[0];
            blue = (DITHER_TO_5_BITS_MUL * blue + ((uint32_t)(*noise_ptr++) << 18)) >> 26;
            output[0] = red | (green << 5) | (blue << 10);
            input += 4;
            output++;
            if (noise_ptr >= noise_end) {
                noise_ptr = noise;
            }
        }
    }
}


void dither_to_16(uint8_t *input, uint16_t *output, int width, int height)
{
    // uint32_t noise = 0x283423;
    uint8_t *image_end = input + 4 * width * height;
    uint8_t *noise_ptr = noise;
    uint8_t *noise_end = noise_ptr + NOISE_SIZE - 3;
    while (input < image_end) {
        uint8_t *line_end = input + 4 * width;
        while (input < line_end) {
            uint32_t red = input[2];
            red = (DITHER_TO_5_BITS_MUL * red + ((uint32_t)(*noise_ptr++) << 18)) >> 26;
            // TODO: or calculate noise on the fly
            //noise = (noise * 61 + 12345) & 0x3FFFFFF;
            //red = (DITHER_TO_5_BITS_MUL * red + noise) >> 26;
            uint32_t green = input[1];
            green = (DITHER_TO_6_BITS_MUL * green + ((uint32_t)(*noise_ptr++) << 18)) >> 26;
            uint32_t blue = input[0];
            blue = (DITHER_TO_5_BITS_MUL * blue + ((uint32_t)(*noise_ptr++) << 18)) >> 26;
            output[0] = red | (green << 5) | (blue << 11);
            input += 4;
            output++;
            if (noise_ptr >= noise_end) {
                noise_ptr = noise;
            }
        }
    }
}


void dither_to_18(uint8_t *input, uint32_t *output, int width, int height)
{
    uint8_t *image_end = input + 4 * width * height;
    uint8_t *noise_ptr = noise;
    uint8_t *noise_end = noise_ptr + NOISE_SIZE - 3;
    while (input < image_end) {
        uint8_t *line_end = input + 4 * width;
        while (input < line_end) {
            uint32_t red = input[2];
            red = (DITHER_TO_6_BITS_MUL * red + ((uint32_t)(*noise_ptr++) << 18)) >> 26;
            uint32_t green = input[1];
            green = (DITHER_TO_6_BITS_MUL * green + ((uint32_t)(*noise_ptr++) << 18)) >> 26;
            uint32_t blue = input[0];
            blue = (DITHER_TO_6_BITS_MUL * blue + ((uint32_t)(*noise_ptr++) << 18)) >> 26;
            output[0] = red | (green << 6) | (blue << 12);
            input += 4;
            output++;
            if (noise_ptr >= noise_end) {
                noise_ptr = noise;
            }
        }
    }
}


void no_dither(uint8_t *input, uint16_t *output, int width, int height, int in_pixel_bytes, int in_line_padding_bytes, int out_line_padding_pixels)
{
    uint8_t *image_end = input + (in_pixel_bytes * width + in_line_padding_bytes) * height;
    while (input < image_end) {
        uint8_t *line_end = input + in_pixel_bytes * width;
        while (input < line_end) {
            uint32_t red = input[2];
            red >>= 3;
            uint32_t green = input[1];
            green >>= 2;
            uint32_t blue = input[0];
            blue >>= 3;
            output[0] = red | (green << 5) | (blue << 11);
            input += in_pixel_bytes;
            output++;
        }
        input += in_line_padding_bytes;
        output += out_line_padding_pixels;
    }
}

uint8_t* read_xwd(uint8_t* input, int* width, int* height) {
    XWDFileHeader* header = (XWDFileHeader*)input;
    *width = be(header->pixmap_width);
    *height = be(header->pixmap_height);
    #define CHECK(field, expected) if (be(header->field) != expected) { fprintf(stderr, "Invalid " #field " %d, expected " #expected "\n", be(header->field)); exit(1); }
    #define CHECKX(field, expected) if (be(header->field) != expected) { fprintf(stderr, "Invalid " #field " 0x%08X, expected " #expected "\n", be(header->field)); exit(1); }
    CHECK(pixmap_depth, 24);
    CHECK(byte_order, 0);
    CHECK(bits_per_pixel, 32);
    CHECK(bytes_per_line, *width * 4);
    CHECK(bits_per_pixel, 32);
    CHECK(red_mask, 0x00FF0000);
    CHECK(green_mask, 0x0000FF00);
    CHECK(blue_mask, 0x000000FF);
    return input + be(header->header_size) + 12 * be(header->ncolors);
}

uint8_t *input_pixels;

void to24bits_15(uint16_t* input, uint8_t* output, uint32_t width, uint32_t height)
{
    int x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            uint32_t px = input[x + y * width];
            uint32_t red = px & 0x1F;
            uint32_t green = (px >> 5) & 0x1F;
            uint32_t blue = (px >> 10) & 0x1F;
            output[4 * (x + y * width) + 2] = red << 3;
            output[4 * (x + y * width) + 1] = green << 3;
            output[4 * (x + y * width) + 0] = blue << 3;
        }
    }
}

void to24bits_16(uint16_t* input, uint8_t* output, uint32_t width, uint32_t height)
{
    int x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            uint32_t px = input[x + y * width];
            uint32_t red = px & 0x1F;
            uint32_t green = (px >> 5) & 0x3F;
            uint32_t blue = (px >> 11) & 0x1F;
            output[4 * (x + y * width) + 2] = red << 3;
            output[4 * (x + y * width) + 1] = green << 2;
            output[4 * (x + y * width) + 0] = blue << 3;
        }
    }
}

void to24bits_18(uint32_t* input, uint8_t* output, uint32_t width, uint32_t height)
{
    int x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            uint32_t px = input[x + y * width];
            uint32_t red = px & 0x3F;
            uint32_t green = (px >> 6) & 0x3F;
            uint32_t blue = (px >> 12) & 0x3F;
            output[4 * (x + y * width) + 2] = red << 2;
            output[4 * (x + y * width) + 1] = green << 2;
            output[4 * (x + y * width) + 0] = blue << 2;
        }
    }
}

void gradient(uint16_t* input, uint8_t* output, uint32_t width, uint32_t height)
{
    int x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            output[4 * (x + y * width) + 2] = 90 + y / 16;
            output[4 * (x + y * width) + 1] = 0;
            output[4 * (x + y * width) + 0] = 0;
        }
    }
}

int main() {
    srand(0);
    for (int i = 0; i < sizeof(noise); i++) {
        noise[i] ^= rand();
    }
    for (int i = 0; i < sizeof(noise); i++) {
        noise[i] ^= rand();
    }
    for (int i = 0; i < sizeof(noise); i++) {
        noise[i] += rand();
    }
    for (int i = 0; i < sizeof(noise); i++) {
        noise[i] += rand();
    }
    FILE* f = fopen("/tmp/fb/a.xwd", "rb");
    int n = fread(buffer, 1, BUFSIZE, f);
    fclose(f);
    input_pixels = read_xwd((uint8_t*)buffer, &width, &height);
    //gradient(NULL, input_pixels, width, height);
    f = fopen("/tmp/fb/c.xwd", "wb");
    fwrite(buffer, 1, n, f);
    fclose(f);
    dither_to_18(input_pixels, pixels32, width, height);
    to24bits_18(pixels32, input_pixels, width, height);
    f = fopen("/tmp/fb/b.xwd", "wb");
    fwrite(buffer, 1, n, f);
    fclose(f);
}
