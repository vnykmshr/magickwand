#ifndef _MAGICKWAND_H
#define _MAGICKWAND_H

#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif

#include <node.h>
#include <node_buffer.h>
#include <uv.h>
#include <stdlib.h>
#include <string.h>

// ImageMagick 7 uses MagickWand/, ImageMagick 6 uses wand/
// Try ImageMagick 6 path first (lowercase), fallback to ImageMagick 7
#if __has_include(<wand/MagickWand.h>)
  #include <wand/MagickWand.h>
  #define MAGICK_VERSION_6
#else
  #include <MagickWand/MagickWand.h>
  #define MAGICK_VERSION_7
#endif

// API compatibility between ImageMagick 6 and 7
// ImageMagick 6 MagickResizeImage takes 5 params (width, height, filter, blur)
// ImageMagick 7 MagickResizeImage takes 4 params (width, height, filter)
#ifdef MAGICK_VERSION_6
  #define ResizeImage(wand, width, height, filter) \
    MagickResizeImage(wand, width, height, filter, 1.0)
#else
  #define ResizeImage(wand, width, height, filter) \
    MagickResizeImage(wand, width, height, filter)
#endif

using namespace node;
using namespace v8;

void init(Local<Object> exports, Local<Value> module, void* priv);

void resizeAsync(const FunctionCallbackInfo<Value>& args);
void thumbnailAsync(const FunctionCallbackInfo<Value>& args);

// Common functions shared between resize and thumbnail
bool processImageDimensions(MagickWand *magick_wand, unsigned int *width, unsigned int *height, bool autocrop, bool useThumbnail);
void buildCallbackArgs(Isolate* isolate, char* exception, unsigned char* resizedImage, size_t resizedImageLen,
                       unsigned int width, unsigned int height, unsigned int quality, Local<Value> argv[3]);

#endif /* _MAGICKWAND_H */
