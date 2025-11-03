#include "./magickwand.h"

void init(Local<Object> exports, Local<Value> module, void* priv) {
  MagickWandGenesis();
  NODE_SET_METHOD(exports, "resize", resizeAsync);
  NODE_SET_METHOD(exports, "thumbnail", thumbnailAsync);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, init)
