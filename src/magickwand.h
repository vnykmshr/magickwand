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

#endif /* _MAGICKWAND_H */
