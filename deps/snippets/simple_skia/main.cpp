#include <core/SkCanvas.h>
#include <core/SkColor.h>
#include <core/SkImageInfo.h>
#include <core/SkPaint.h>
#include <core/SkStream.h>
#include <core/SkSurface.h>
#include <effects/SkGradient.h>
#include <encode/SkPngEncoder.h>

#include <iostream>

int main() {
  constexpr int width = 512;
  constexpr int height = 512;
  constexpr float radius = 210.0f;

  auto surface = SkSurfaces::Raster(
      SkImageInfo::MakeN32Premul(width, height));
  if (surface == nullptr) {
    std::cerr << "Failed to create Skia raster surface.\n";
    return 1;
  }

  SkCanvas* canvas = surface->getCanvas();
  canvas->clear(SK_ColorWHITE);

  const SkColor4f colors[] = {
      SkColor4f{0.15f, 0.55f, 1.0f, 1.0f},
      SkColor4f{0.95f, 0.25f, 0.75f, 1.0f},
      SkColor4f{1.0f, 0.95f, 0.25f, 1.0f},
  };
  const float positions[] = {0.0f, 0.55f, 1.0f};

  SkPaint paint;
  paint.setAntiAlias(true);
  paint.setShader(SkShaders::RadialGradient(
      SkPoint::Make(width * 0.5f, height * 0.5f), radius,
      SkGradient(SkGradient::Colors(colors, positions, SkTileMode::kClamp),
                 SkGradient::Interpolation{})));

  canvas->drawCircle(width * 0.5f, height * 0.5f, radius, paint);

  SkPixmap pixmap;
  if (!surface->peekPixels(&pixmap)) {
    std::cerr << "Failed to read pixels from Skia surface.\n";
    return 1;
  }

  const char* output_path = "gradient_circle.png";
  SkFILEWStream output(output_path);
  if (!output.isValid()) {
    std::cerr << "Failed to open output file: " << output_path << '\n';
    return 1;
  }

  if (!SkPngEncoder::Encode(&output, pixmap, SkPngEncoder::Options{})) {
    std::cerr << "Failed to encode PNG: " << output_path << '\n';
    return 1;
  }

  std::cout << "Wrote " << output_path << '\n';
  return 0;
}
