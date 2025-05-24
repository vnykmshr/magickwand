#include "./magickwand.h" // Includes nan.h

static void resize_work_nan(uv_work_t *req);
static void postResize_nan(uv_work_t *req, int status);

static void resize_work_nan(uv_work_t *req) {
  processImage(req, false /* isThumbnail */, false /* isRotate */);
}

static void postResize_nan(uv_work_t *req, int status) {
  Nan::HandleScope scope;
  struct magickReq *mgr = (struct magickReq *)req->data;

  v8::Local<v8::Value> argv[3];

  if (mgr->exception) {
    argv[0] = Nan::Error(mgr->exception);
    argv[1] = Nan::Undefined();
    argv[2] = Nan::Undefined();
    if (mgr->exception) free(mgr->exception); 
  } else {
    argv[0] = Nan::Undefined();
    argv[1] = Nan::NewBuffer((char*)mgr->resizedImage, mgr->resizedImageLen).ToLocalChecked();
    
    v8::Local<v8::Object> infoObj = Nan::New<v8::Object>();
    Nan::Set(infoObj, Nan::New("width").ToLocalChecked(), Nan::New(mgr->width));
    Nan::Set(infoObj, Nan::New("height").ToLocalChecked(), Nan::New(mgr->height));
    argv[2] = infoObj;
  }

  Nan::Callback cb(Nan::New(mgr->cb));
  Nan::AsyncResource resource("magickwand:resize");
  cb.Call(3, argv, &resource);

  mgr->cb.Reset();
  if (mgr->resizedImage) MagickRelinquishMemory(mgr->resizedImage);
  if (mgr->format) free(mgr->format);
  if (mgr->imagefilepath) free(mgr->imagefilepath); // Free if it was strdup'ed
  // inputBuffer is not owned by mgr, it's a pointer to V8 managed memory (Buffer)
  // or part of the JS object that needs to be kept alive by the GC until the callback.
  // Nan::Persistent for the callback helps keep the context alive.
  free(mgr);

  delete req;
}

NAN_METHOD(resizeAsync) {
  Nan::HandleScope();

  const char *usage = "Usage: resize(imagefileOrBuffer, width, height, quality, format, autocrop, cb)";
  if (info.Length() != 7) {
    return Nan::ThrowError(usage);
  }
  if (!info[0]->IsString() && !info[0]->IsObject()) { // Simplified check, could be more specific for Buffer
    return Nan::ThrowError("First argument must be a string (filepath) or a Buffer.");
  }
  if (!info[6]->IsFunction()) {
    return Nan::ThrowError("Last argument must be a callback function.");
  }
  
  v8::Local<v8::Value> inputArg = info[0];
  int width = Nan::To<int32_t>(info[1]).FromMaybe(0);
  int height = Nan::To<int32_t>(info[2]).FromMaybe(0);
  int quality = Nan::To<int32_t>(info[3]).FromMaybe(0);
  Nan::Utf8String format_str(info[4]); // format is string, might be empty
  bool autocrop = Nan::To<bool>(info[5]).FromMaybe(false);
  v8::Local<v8::Function> cb_func = info[6].As<v8::Function>();

  if (width < 0 || height < 0) {
    return Nan::ThrowError("Invalid width/height arguments (must be non-negative).");
  }
  if (quality < 0 || quality > 100) {
    return Nan::ThrowError("Invalid quality parameter (must be 0-100).");
  }

  uv_work_t *req = new uv_work_t;
  struct magickReq *mgr = (struct magickReq *) calloc(1, sizeof(struct magickReq));
  
  if (!mgr) {
    delete req;
    return Nan::ThrowError("Failed to allocate memory for request struct.");
  }
  req->data = mgr;

  mgr->cb.Reset(cb_func);
  mgr->width = width;
  mgr->height = height;
  mgr->quality = quality;
  mgr->autocrop = autocrop;
  mgr->format = NULL;
  mgr->exception = NULL;
  mgr->resizedImage = NULL;
  mgr->resizedImageLen = 0;
  mgr->imagefilepath = NULL;
  mgr->inputBuffer = NULL;
  mgr->inputBufferLen = 0;

  // Handle input: buffer or filepath string
  if (node::Buffer::HasInstance(inputArg)) { // Use node::Buffer
    v8::Local<v8::Object> bufferObj = inputArg.As<v8::Object>();
    mgr->inputBuffer = (const unsigned char*) node::Buffer::Data(bufferObj); // Use node::Buffer
    mgr->inputBufferLen = node::Buffer::Length(bufferObj); // Use node::Buffer
    // Keep the buffer object alive by making the callback persistent,
    // which should keep the whole context (including arguments) alive.
  } else if (inputArg->IsString()) {
    Nan::Utf8String name(inputArg);
    if (name.length() == 0) {
      mgr->cb.Reset(); free(mgr); delete req; // Basic cleanup
      return Nan::ThrowError("Imagefile path is empty.");
    }
    mgr->imagefilepath = strdup(*name);
    if (!mgr->imagefilepath) {
      mgr->cb.Reset(); free(mgr); delete req;
      return Nan::ThrowError("Failed to allocate memory for image filepath string.");
    }
  } else {
    // Should have been caught by initial check, but as a safeguard:
    mgr->cb.Reset(); free(mgr); delete req;
    return Nan::ThrowError("First argument must be a string (filepath) or a Buffer.");
  }

  if (format_str.length() > 0) {
    mgr->format = strdup(*format_str);
    if (!mgr->format) {
        if(mgr->imagefilepath) free(mgr->imagefilepath);
        mgr->cb.Reset(); free(mgr); delete req;
        return Nan::ThrowError("Failed to allocate memory for format string.");
    }
  }

  uv_queue_work(uv_default_loop(), req, resize_work_nan, postResize_nan);
  info.GetReturnValue().Set(Nan::Undefined());
}
