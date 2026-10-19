#include <img_lib.h>
#include <jpeg_image.h>
#include <ppm_image.h>

#include <filesystem>
#include <string_view>
#include <iostream>
#include "jpeg_image.h"
#include "ppm_image.h"
#include <bmp_image.h>
#include <memory>

using namespace std;

enum class Format {
    UNKNOWN,
    JPEG,
    PPM,
    BMP
};

class ImageFormatInterface {
public:
    virtual ~ImageFormatInterface() = default;

    virtual bool SaveImage(const img_lib::Path& file,
                           const img_lib::Image& image) const = 0;

    virtual img_lib::Image LoadImage(
        const img_lib::Path& file) const = 0;
};

Format GetFormatByExtension(const img_lib::Path& input_file) {
    const string ext = input_file.extension().string();
    if (ext == ".jpg"sv || ext == ".jpeg"sv) {
        return Format::JPEG;
    }

    if (ext == ".ppm"sv) {
        return Format::PPM;
    }

    if (ext == ".bmp"sv) {
        return Format::BMP;
    }

    return Format::UNKNOWN;
}

class ImageJPEG : public ImageFormatInterface {
public:
    bool SaveImage(const img_lib::Path& file,
                   const img_lib::Image& image) const override {
        return img_lib::SaveJPEG(file, image);
    }

    img_lib::Image LoadImage(const img_lib::Path& file) const override {
        return img_lib::LoadJPEG(file);
    }
};

class ImagePPM : public ImageFormatInterface {
public:
    bool SaveImage(const img_lib::Path& file,
                   const img_lib::Image& image) const override {
        return img_lib::SavePPM(file, image);
    }

    img_lib::Image LoadImage(const img_lib::Path& file) const override {
        return img_lib::LoadPPM(file);
    }
};

class ImageBMP : public ImageFormatInterface {
public:
    bool SaveImage(const img_lib::Path& file,
                   const img_lib::Image& image) const override {
        return img_lib::SaveBMP(file, image);
    }

    img_lib::Image LoadImage(const img_lib::Path& file) const override {
        return img_lib::LoadBMP(file);
    }
};

unique_ptr<ImageFormatInterface> CreateImageFormat(Format format) {
    switch (format) {
        case Format::JPEG:
            return make_unique<ImageJPEG>();
        case Format::PPM:
            return make_unique<ImagePPM>();
        case Format::BMP:
            return make_unique<ImageBMP>();
        default:
            return nullptr;
    }
}

int main(int argc, const char** argv) {
    if (argc != 3) {
        cerr << "Usage: "sv << argv[0]
             << " <in_file> <out_file>"sv << endl;
        return 1;
    }

    img_lib::Path in_path = argv[1];
    img_lib::Path out_path = argv[2];

    auto input = CreateImageFormat(GetFormatByExtension(in_path));
    if (!input) {
        cerr << "Unknown input format"sv << endl;
        return 2;
    }

    auto output = CreateImageFormat(GetFormatByExtension(out_path));
    if (!output) {
        cerr << "Unknown output format"sv << endl;
        return 3;
    }

    img_lib::Image image = input->LoadImage(in_path);
    if (!image) {
        cerr << "Loading failed"sv << endl;
        return 4;
    }

    if (!output->SaveImage(out_path, image)) {
        cerr << "Saving failed"sv << endl;
        return 5;
    }

    cout << "Successfully converted"sv << endl;
}