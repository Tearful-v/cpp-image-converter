#include "ppm_image.h"

#include <array>
#include <fstream>
#include <string_view>
#include <climits>

using namespace std;

namespace img_lib {

namespace {
constexpr int BYTES_PER_PIXEL = 3;
}  // namespace


static const string_view PPM_SIG = "P6"sv;
static const int PPM_MAX = 255;

bool SavePPM(const Path& file, const Image& image) {
    ofstream out(file, ios::binary);
    if(!out) {
        return false;
    }

    out << PPM_SIG << '\n' << image.GetWidth() << ' ' << image.GetHeight() << '\n' << PPM_MAX << '\n';

    const int w = image.GetWidth();
    const int h = image.GetHeight();
    std::vector<char> buff(w * BYTES_PER_PIXEL);

    for (int y = 0; y < h; ++y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < w; ++x) {
            buff[x * BYTES_PER_PIXEL + 0] = static_cast<char>(line[x].r);
            buff[x * BYTES_PER_PIXEL + 1] = static_cast<char>(line[x].g);
            buff[x * BYTES_PER_PIXEL + 2] = static_cast<char>(line[x].b);
        }
        out.write(buff.data(), w * BYTES_PER_PIXEL);
    }

    return out.good();
}

Image LoadPPM(const Path& file) {
    // открываем поток с флагом ios::binary
    // поскольку будем читать данные в двоичном формате
    ifstream ifs(file, ios::binary);
    std::string sign;
    int w = 0;
    int h = 0;
    int color_max = 0;

    // читаем заголовок: он содержит формат, размеры изображения
    // и максимальное значение цвета
    ifs >> sign >> w >> h >> color_max;

    // мы поддерживаем изображения только формата P6
    // с максимальным значением цвета 255
    if (sign != PPM_SIG || color_max != PPM_MAX
        || w < 0 || h < 0 || color_max < 0) {
        return {};
    }

    // пропускаем один байт - это конец строки
    const char next = ifs.get();
    if (next != '\n') {
        return {};
    }

    Image result(w, h, Color::Black());
    std::vector<char> buff(w * BYTES_PER_PIXEL);

    for (int y = 0; y < h; ++y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), w * BYTES_PER_PIXEL);
        if (!ifs) {
            return {};
        }

        for (int x = 0; x < w; ++x) {
            line[x].r = static_cast<byte>(buff[x * BYTES_PER_PIXEL + 0]);
            line[x].g = static_cast<byte>(buff[x * BYTES_PER_PIXEL + 1]);
            line[x].b = static_cast<byte>(buff[x * BYTES_PER_PIXEL + 2]);
        }
    }

    return result;
}

}  // namespace img_lib
