#include "./magickwand.h" // Includes nan.h

static void rotate_work_nan(uv_work_t *req);
static void postRotate_nan(uv_work_t *req, int status);

static void rotate_work_nan(uv_work_t *req) {
  // Call processImage with isThumbnail = false, isRotate = true
  processImage(req, false /* isThumbnail */, true /* isRotate */);
}

static void postRotate_nan(uv_work_t *req, int status) {
  Nan::HandleScope scope;
  struct magickReq *mgr = (struct magickReq *)req->data;

  v8::Local<v8::Value> argv[3]; // Callback arguments: err, buffer, info

  if (mgr->exception) {
    argv[0] = Nan::Error(mgr->exception);
    argv[1] = Nan::Undefined();
    argv[2] = Nan::Undefined();
    if (mgr->exception) free(mgr->exception); 
  } else {
    argv[0] = Nan::Undefined();
    argv[1] = Nan::NewBuffer((char*)mgr->resizedImage, mgr->resizedImageLen).ToLocalChecked();
    
    v8::Local<v8::Object> infoObj = Nan::New<v8::Object>();
    // Dimensions are updated in mgr->width and mgr->height by processImage after rotation.
    Nan::Set(infoObj, Nan::New("width").ToLocalChecked(), Nan::New(mgr->width));
    Nan::Set(infoObj, Nan::New("height").ToLocalChecked(), Nan::New(mgr->height));

    argv[2] = infoObj;
  }

  Nan::Callback cb(Nan::New(mgr->cb));
  Nan::AsyncResource resource("magickwand:rotate");
  cb.Call(3, argv, &resource);

  mgr->cb.Reset();
  if (mgr->resizedImage) MagickRelinquishMemory(mgr->resizedImage);
  // No format string for rotate
  if (mgr->imagefilepath) free(mgr->imagefilepath);
  free(mgr);

  delete req;
}

NAN_METHOD(rotateAsync) {
  Nan::HandleScope();

  // Args: imagefileOrBuffer, degrees, cb
  const char *usage = "Usage: rotate(imagefileOrBuffer, degrees, cb)";
  if (info.Length() != 3) {
    return Nan::ThrowError(usage);
  }
  if (!info[0]->IsString() && !info[0]->IsObject()) { 
    return Nan::ThrowError("First argument must be a string (filepath) or a Buffer.");
  }
  if (!info[1]->IsNumber()) {
    return Nan::ThrowError("Second argument (degrees) must be a number.");
  }
  if (!info[2]->IsFunction()) {
    return Nan::ThrowError("Third argument must be a callback function.");
  }
  
  v8::Local<v8::Value> inputArg = info[0];
  double degrees = Nan::To<double>(info[1]).FromMaybe(0.0);
  v8::Local<v8::Function> cb_func = info[2].As<v8::Function>();

  uv_work_t *req = new uv_work_t;
  struct magickReq *mgr = (struct magickReq *) calloc(1, sizeof(struct magickReq));
  
  if (!mgr) {
    delete req;
    return Nan::ThrowError("Failed to allocate memory for request struct.");
  }
  req->data = mgr;

  mgr->cb.Reset(cb_func);
  mgr->degrees = degrees;
  // Set other potentially relevant fields to defaults if processImage expects them
  mgr->width = 0; // Not resizing, but processImage might look at it
  mgr->height = 0;
  mgr->quality = 0; // Default quality
  mgr->autocrop = false;
  mgr->format = NULL;
  mgr->exception = NULL;
  mgr->resizedImage = NULL;
  mgr->resizedImageLen = 0;
  mgr->imagefilepath = NULL;
  mgr->inputBuffer = NULL;
  mgr->inputBufferLen = 0;

  if (node::Buffer::HasInstance(inputArg)) {
    v8::Local<v8::Object> bufferObj = inputArg.As<v8::Object>();
    mgr->inputBuffer = (const unsigned char*) node::Buffer::Data(bufferObj);
    mgr->inputBufferLen = node::Buffer::Length(bufferObj);
  } else if (inputArg->IsString()) {
    Nan::Utf8String name(inputArg);
    if (name.length() == 0) {
      mgr->cb.Reset(); free(mgr); delete req;
      return Nan::ThrowError("Imagefile path is empty.");
    }
    mgr->imagefilepath = strdup(*name);
    if (!mgr->imagefilepath) {
      mgr->cb.Reset(); free(mgr); delete req;
      return Nan::ThrowError("Failed to allocate memory for image filepath string.");
    }
  } else {
    mgr->cb.Reset(); free(mgr); delete req;
    return Nan::ThrowError("First argument must be a string (filepath) or a Buffer.");
  }

  uv_queue_work(uv_default_loop(), req, rotate_work_nan, postRotate_nan);
  info.GetReturnValue().Set(Nan::Undefined());
}
