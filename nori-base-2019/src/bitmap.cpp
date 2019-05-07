/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <nori/bitmap.h>
#include <ImfInputFile.h>
#include <ImfOutputFile.h>
#include <ImfChannelList.h>
#include <ImfStringAttribute.h>
#include <ImfVersion.h>
#include <ImfIO.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

NORI_NAMESPACE_BEGIN

Bitmap::Bitmap(const std::string &filename) {
    Imf::InputFile file(filename.c_str());
    const Imf::Header &header = file.header();
    const Imf::ChannelList &channels = header.channels();

    Imath::Box2i dw = file.header().dataWindow();
    resize(dw.max.y - dw.min.y + 1, dw.max.x - dw.min.x + 1);

    cout << "Reading a " << cols() << "x" << rows() << " OpenEXR file from \""
         << filename << "\"" << endl;

    const char *ch_r = nullptr, *ch_g = nullptr, *ch_b = nullptr;
    for (Imf::ChannelList::ConstIterator it = channels.begin(); it != channels.end(); ++it) {
        std::string name = toLower(it.name());

        if (it.channel().xSampling != 1 || it.channel().ySampling != 1) {
            /* Sub-sampled layers are not supported */
            continue;
        }

        if (!ch_r && (name == "r" || name == "red" ||
                endsWith(name, ".r") || endsWith(name, ".red"))) {
            ch_r = it.name();
        } else if (!ch_g && (name == "g" || name == "green" ||
                endsWith(name, ".g") || endsWith(name, ".green"))) {
            ch_g = it.name();
        } else if (!ch_b && (name == "b" || name == "blue" ||
                endsWith(name, ".b") || endsWith(name, ".blue"))) {
            ch_b = it.name();
        }
    }

    if (!ch_r || !ch_g || !ch_b)
        throw NoriException("This is not a standard RGB OpenEXR file!");

    size_t compStride = sizeof(float),
           pixelStride = 3 * compStride,
           rowStride = pixelStride * cols();

    char *ptr = reinterpret_cast<char *>(data());

    Imf::FrameBuffer frameBuffer;
    frameBuffer.insert(ch_r, Imf::Slice(Imf::FLOAT, ptr, pixelStride, rowStride)); ptr += compStride;
    frameBuffer.insert(ch_g, Imf::Slice(Imf::FLOAT, ptr, pixelStride, rowStride)); ptr += compStride;
    frameBuffer.insert(ch_b, Imf::Slice(Imf::FLOAT, ptr, pixelStride, rowStride));
    file.setFrameBuffer(frameBuffer);
    file.readPixels(dw.min.y, dw.max.y);
}

void Bitmap::saveEXR(const std::string &filename) {
    cout << "Writing a " << cols() << "x" << rows()
         << " OpenEXR file to \"" << filename << "\"" << endl;

    std::string path = filename + ".exr";

    Imf::Header header((int) cols(), (int) rows());
    header.insert("comments", Imf::StringAttribute("Generated by Nori"));

    Imf::ChannelList &channels = header.channels();
    channels.insert("R", Imf::Channel(Imf::FLOAT));
    channels.insert("G", Imf::Channel(Imf::FLOAT));
    channels.insert("B", Imf::Channel(Imf::FLOAT));

    Imf::FrameBuffer frameBuffer;
    size_t compStride = sizeof(float),
           pixelStride = 3 * compStride,
           rowStride = pixelStride * cols();

    char *ptr = reinterpret_cast<char *>(data());
    frameBuffer.insert("R", Imf::Slice(Imf::FLOAT, ptr, pixelStride, rowStride)); ptr += compStride;
    frameBuffer.insert("G", Imf::Slice(Imf::FLOAT, ptr, pixelStride, rowStride)); ptr += compStride;
    frameBuffer.insert("B", Imf::Slice(Imf::FLOAT, ptr, pixelStride, rowStride));

    Imf::OutputFile file(path.c_str(), header);
    file.setFrameBuffer(frameBuffer);
    file.writePixels((int) rows());
}

void Bitmap::savePNG(const std::string &filename) {
    cout << "Writing a " << cols() << "x" << rows()
         << " PNG file to \"" << filename << "\"" << endl;

    std::string path = filename + ".png";

    uint8_t *rgb8 = new uint8_t[3 * cols() * rows()];
    uint8_t *dst = rgb8;
    for (int i = 0; i < rows(); ++i) {
        for (int j = 0; j < cols(); ++j) {
            Color3f tonemapped = coeffRef(i, j).toSRGB();
            dst[0] = (uint8_t) clamp(255.f * tonemapped[0], 0.f, 255.f);
            dst[1] = (uint8_t) clamp(255.f * tonemapped[1], 0.f, 255.f);
            dst[2] = (uint8_t) clamp(255.f * tonemapped[2], 0.f, 255.f);
            dst += 3;
        }
    }

    int ret = stbi_write_png(path.c_str(), cols(), rows(), 3, rgb8, 3 * cols());
    if (ret == 0) {
        cout << "Bitmap::savePNG(): Could not save PNG file \"" << path << "%s\"" << endl;
    }

    delete[] rgb8;
}

NORI_NAMESPACE_END
