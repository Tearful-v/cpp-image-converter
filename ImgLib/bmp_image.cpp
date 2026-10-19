#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <vector>
#include <fstream>
#include <string_view>
#include <climits>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    char signature[2] = {'B', 'M'};
    uint32_t file_size = 0;
    uint32_t reserved = 0;
    uint32_t data_offset = 54;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint32_t size = 40;
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
    return 4 * ((w * 3 + 3) / 4);
}


bool SaveBMP(const Path& file, const Image& image) {
    std::ofstream out(file, std::ios::binary);
    if (!out) {
        return false;
    }

    BitmapFileHeader fheader;
    BitmapInfoHeader infheader;

    const int width = image.GetWidth();
    const int height = image.GetHeight();
    const int stride = GetBMPStride(width);

    infheader.width = width;
    infheader.height = height;
    infheader.image_size = stride * height;

    fheader.data_offset = sizeof(fheader) + sizeof(infheader);
    fheader.file_size = fheader.data_offset + infheader.image_size;

    out.write(reinterpret_cast<const char*>(&fheader),
              sizeof(fheader));

    out.write(reinterpret_cast<const char*>(&infheader),
              sizeof(infheader));

    std::vector<char> buff(stride, 0);

    for (int y = height - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);

        for (int x = 0; x < width; ++x) {
            buff[x * 3 + 0]     = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
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

    BitmapFileHeader fheader;
    BitmapInfoHeader infheader;

    in.read(reinterpret_cast<char*>(&fheader), sizeof(fheader));
    in.read(reinterpret_cast<char*>(&infheader), sizeof(infheader));

    if (!in ||
        fheader.signature[0] != 'B' ||
        fheader.signature[1] != 'M' ||
        infheader.size != 40 ||
        infheader.planes != 1 ||
        infheader.bits_per_pixel != 24 ||
        infheader.compression != 0 ||
        infheader.width <= 0 ||
        infheader.height <= 0 ||
        fheader.data_offset < sizeof(fheader) + sizeof(infheader)) {
        return {};
    }

    const int width = infheader.width;
    const int height = infheader.height;

    if (width > (INT_MAX - 3) / 3) {
        return {};
    }

    const int stride = GetBMPStride(width);

    in.seekg(fheader.data_offset, ios::beg);
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
                static_cast<byte>(buffer[x * 3 + 2]), 
                static_cast<byte>(buffer[x * 3 + 1]), 
                static_cast<byte>(buffer[x * 3 + 0]),     
                byte{255}                           
            };
        }
    }

    return image;
}

}  // namespace img_lib