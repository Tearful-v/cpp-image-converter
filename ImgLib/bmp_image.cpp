#include "bmp_image.h"
#include "pack_defines.h"

#include <vector>
#include <fstream>
#include <climits>
#include <cstdint>

using namespace std;

namespace img_lib {

namespace {
constexpr int BYTES_PER_PIXEL = 3;
constexpr int BMP_ALIGNMENT = 4;
}  // namespace


PACKED_STRUCT_BEGIN BitmapFileHeader {
    char signature[2] = {'B', 'M'};
    uint32_t file_size = 0;
    uint32_t reserved = 0;
    uint32_t data_offset = 0;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint32_t size = sizeof(BitmapInfoHeader);
    int32_t width = 0;
    int32_t height = 0;
    uint16_t planes = 1;
    uint16_t bits_per_pixel = 24;
    uint32_t compression = 0;
    uint32_t image_size = 0;
    int32_t h_resolution = 11811;
    int32_t v_resolution = 11811;
    int32_t used_colors = 0;
    int32_t significant_colors = 0x1000000;
}
PACKED_STRUCT_END

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return BMP_ALIGNMENT *
        ((w * BYTES_PER_PIXEL + BMP_ALIGNMENT - 1) / BMP_ALIGNMENT);
}


bool SaveBMP(const Path& file, const Image& image) {
    std::ofstream out(file, std::ios::binary);
    if (!out) {
        return false;
    }

    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    const int width = image.GetWidth();
    const int height = image.GetHeight();
    const int stride = GetBMPStride(width);

    info_header.width = width;
    info_header.height = height;
    info_header.image_size = stride * height;

    file_header.data_offset = sizeof(file_header) + sizeof(info_header);
    file_header.file_size = file_header.data_offset + info_header.image_size;

    out.write(reinterpret_cast<const char*>(&file_header),
              sizeof(file_header));

    out.write(reinterpret_cast<const char*>(&info_header),
              sizeof(info_header));

    std::vector<char> buff(stride, 0);

    for (int y = height - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);

        for (int x = 0; x < width; ++x) {
            buff[x * BYTES_PER_PIXEL + 0]     = static_cast<char>(line[x].b);
            buff[x * BYTES_PER_PIXEL + 1] = static_cast<char>(line[x].g);
            buff[x * BYTES_PER_PIXEL + 2] = static_cast<char>(line[x].r);
        }

        out.write(buff.data(), stride);
    }

    out.close();
    return (bool)out;
}

// напишите эту функцию
Image LoadBMP(const Path& file) {
    ifstream in(file, ios::binary);
    if (!in) {
        return {};
    }

    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    in.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    in.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));

    if (!in ||
        file_header.signature[0] != 'B' ||
        file_header.signature[1] != 'M' ||
        info_header.size != sizeof(info_header) ||
        info_header.planes != 1 ||
        info_header.bits_per_pixel != 24 ||
        info_header.compression != 0 ||
        info_header.width <= 0 ||
        info_header.height <= 0 ||
        file_header.data_offset < sizeof(file_header) + sizeof(info_header)) {
        return {};
    }

    const int width = info_header.width;
    const int height = info_header.height;

    if (width > (INT_MAX - (BMP_ALIGNMENT - 1)) / BYTES_PER_PIXEL) {
        return {};
    }

    const int stride = GetBMPStride(width);

    if (width > INT_MAX / height) {
        return {};
    }

    in.seekg(0, ios::end);
    const streamoff file_size = in.tellg();
    if (!in || file_size < 0 || file_header.data_offset > file_size) {
        return {};
    }
    if ((file_size - file_header.data_offset) / stride < height) {
        return {};
    }

    in.seekg(file_header.data_offset, ios::beg);
    if (!in) {
        return {};
    }

    Image image(width, height, Color::Black());
    vector<unsigned char> buffer(stride);

    for (int y = height - 1; y >= 0; --y) {
        in.read(reinterpret_cast<char*>(buffer.data()), stride);
        if (!in) {
            return {};
        }

        Color* line = image.GetLine(y);

        for (int x = 0; x < width; ++x) {
            line[x] = Color{
                static_cast<byte>(buffer[x * BYTES_PER_PIXEL + 2]),
                static_cast<byte>(buffer[x * BYTES_PER_PIXEL + 1]),
                static_cast<byte>(buffer[x * BYTES_PER_PIXEL + 0]),
                byte{255}
            };
        }
    }

    return image;
}

}  // namespace img_lib
