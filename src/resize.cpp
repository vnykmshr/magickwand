#include "./magickwand.h"

struct magickReq {
  Persistent<Function> cb;
  Isolate* isolate;
  unsigned char *resizedImage;
  char *exception;
  char *format;
  size_t resizedImageLen;
  int quality;

  bool autocrop;

  unsigned int width;
  unsigned int height;
  char imagefilepath[1];
};

/* Resize image here */
static void resize (uv_work_t *req) {
  struct magickReq *mgr = (struct magickReq *)req->data;
  ExceptionType severity;
  MagickWand *magick_wand = NewMagickWand();
  MagickBooleanType status;

  status = MagickReadImage(magick_wand, mgr->imagefilepath);

  if (status == MagickFalse) {
    mgr->exception = MagickGetException(magick_wand, &severity);
    DestroyMagickWand(magick_wand);
    return;
  }

  // Use shared dimension processing logic (resize, not thumbnail)
  processImageDimensions(magick_wand, &mgr->width, &mgr->height, mgr->autocrop, false);

  if (mgr->format) {
    MagickSetImageFormat(magick_wand, mgr->format);
  }

  if (mgr->quality) {
    MagickSetImageCompressionQuality(magick_wand, mgr->quality);
  }

  mgr->resizedImage = MagickGetImageBlob(magick_wand, &mgr->resizedImageLen);
  if (!mgr->resizedImage) {
    mgr->exception = MagickGetException(magick_wand, &severity);
  }

  DestroyMagickWand(magick_wand);
}

/* this is for the callback */
static void postResize(uv_work_t *req, int status) {
  struct magickReq *mgr = (struct magickReq *)req->data;
  Isolate* isolate = mgr->isolate;
  HandleScope scope(isolate);

  Local<Value> argv[3];
  Local<Context> context = isolate->GetCurrentContext();

  // Use shared callback args builder (quality=0 for resize)
  buildCallbackArgs(isolate, mgr->exception, mgr->resizedImage, mgr->resizedImageLen,
                    mgr->width, mgr->height, 0, argv);

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
  if (mgr->format)
    free(mgr->format);
  free(mgr);

  delete req;
}

void resizeAsync(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();

  const char *usage = "Too few arguments: Usage: resize(imagefile, width, height, quality, format, autocrop, cb)";
  if (args.Length() != 7) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, usage).ToLocalChecked()));
    return;
  }

  String::Utf8Value name(isolate, args[0]);
  String::Utf8Value format(isolate, args[4]);
  int width = args[1]->Int32Value(context).ToChecked();
  int height = args[2]->Int32Value(context).ToChecked();
  int quality = args[3]->Int32Value(context).ToChecked();

  bool autocrop = args[5]->BooleanValue(isolate);

  Local<Function> cb = Local<Function>::Cast(args[6]);

  if (width < 0 || height < 0) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid width/height arguments").ToLocalChecked()));
    return;
  }

  if (quality < 0 || quality > 100) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid quality parameter").ToLocalChecked()));
    return;
  }

  uv_work_t *req = new uv_work_t;
  struct magickReq *mgr = (struct magickReq *) calloc(1, sizeof(struct magickReq) + name.length());
  req->data = mgr;

  mgr->cb.Reset(isolate, cb);
  mgr->isolate = isolate;
  mgr->width = width;
  mgr->height = height;
  mgr->quality = quality;
  mgr->autocrop = autocrop;

  if (*format)
    mgr->format = strdup(*format);

  strncpy(mgr->imagefilepath, *name, name.length() + 1);

  uv_queue_work(uv_default_loop(), req, resize, postResize);
}
