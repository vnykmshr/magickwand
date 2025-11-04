#include "./magickwand.h"

typedef struct thumbnailReq {
  Persistent<Function> cb;
  Isolate* isolate;
  unsigned char *resizedImage;
  char *exception;
  size_t resizedImageLen;

  bool autocrop;

  unsigned int width;
  unsigned int height;
  unsigned int quality;
  char imagefilepath[1];
} ThumbnailReq;

/* Thumbnail image here */
static void thumbnail(uv_work_t *req) {
  ThumbnailReq *mgr = (ThumbnailReq *)req->data;
  ExceptionType severity;
  MagickWand *magick_wand = NewMagickWand();
  MagickBooleanType status;

  status = MagickReadImage(magick_wand, mgr->imagefilepath);

  if (status == MagickFalse) {
    mgr->exception = MagickGetException(magick_wand, &severity);
    DestroyMagickWand(magick_wand);
    return;
  }

  // Use shared dimension processing logic (thumbnail, not resize)
  processImageDimensions(magick_wand, &mgr->width, &mgr->height, mgr->autocrop, true);

  if (mgr->quality)
    MagickSetImageCompressionQuality(magick_wand, mgr->quality);

  mgr->resizedImage = MagickGetImageBlob(magick_wand, &mgr->resizedImageLen);

  if (!mgr->resizedImage) {
    mgr->exception = MagickGetException(magick_wand, &severity);
  }

  DestroyMagickWand(magick_wand);
}

static void postThumbnail(uv_work_t *req, int status) {
  ThumbnailReq *mgr = (ThumbnailReq *)req->data;
  Isolate* isolate = mgr->isolate;
  HandleScope scope(isolate);

  Local<Value> argv[3];
  Local<Context> context = isolate->GetCurrentContext();

  // Use shared callback args builder (include quality for thumbnail)
  buildCallbackArgs(isolate, mgr->exception, mgr->resizedImage, mgr->resizedImageLen,
                    mgr->width, mgr->height, mgr->quality, argv);

  Local<Function> cb = Local<Function>::New(isolate, mgr->cb);
  TryCatch try_catch(isolate);

  cb->Call(context, Null(isolate), 3, argv).IsEmpty();

  if (try_catch.HasCaught()) {
    node::FatalException(isolate, try_catch);
  }

  mgr->cb.Reset();
  if (mgr->resizedImage) {
    MagickRelinquishMemory(mgr->resizedImage);
  }
  free(mgr);

  delete req;
}

void thumbnailAsync(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();

  const char *usage = "Too few arguments: Usage: thumbnail(pathtoimgfile, width, height, quality, autocrop, cb)";
  int argc = 0;
  if (args.Length() != 6) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, usage).ToLocalChecked()));
    return;
  }

  String::Utf8Value name(isolate, args[argc++]);
  int width = args[argc++]->Int32Value(context).ToChecked();
  int height = args[argc++]->Int32Value(context).ToChecked();
  int quality = args[argc++]->Int32Value(context).ToChecked();
  bool autocrop = args[argc++]->BooleanValue(isolate);

  Local<Function> cb = Local<Function>::Cast(args[argc++]);

  if (width < 0 || height < 0) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid width/height arguments").ToLocalChecked()));
    return;
  }

  if (quality < 0 || quality > 100) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid quality parameter").ToLocalChecked()));
    return;
  }

  uv_work_t *req = new uv_work_t;
  ThumbnailReq *mgr = (ThumbnailReq *) calloc(1, sizeof(ThumbnailReq) + name.length());
  req->data = mgr;

  mgr->cb.Reset(isolate, cb);
  mgr->isolate = isolate;
  mgr->width = width;
  mgr->height = height;
  mgr->quality = quality;
  mgr->autocrop = autocrop;

  strncpy(mgr->imagefilepath, *name, name.length() + 1);

  uv_queue_work(uv_default_loop(), req, thumbnail, postThumbnail);
}
