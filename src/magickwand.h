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
#include <MagickWand/MagickWand.h>

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
