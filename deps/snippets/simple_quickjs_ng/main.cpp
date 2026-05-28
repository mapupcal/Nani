#include <quickjs.h>

#include <cstring>
#include <iostream>

int main() {
  JSRuntime* runtime = JS_NewRuntime();
  if (runtime == nullptr) {
    std::cerr << "JS_NewRuntime failed.\n";
    return 1;
  }

  JSContext* context = JS_NewContext(runtime);
  if (context == nullptr) {
    std::cerr << "JS_NewContext failed.\n";
    JS_FreeRuntime(runtime);
    return 1;
  }

  const char* script = "1 + 2 + 3";
  JSValue value = JS_Eval(context, script, std::strlen(script), "simple_quickjs_ng.js",
                         JS_EVAL_TYPE_GLOBAL);
  if (JS_IsException(value)) {
    std::cerr << "JS_Eval failed.\n";
    JS_FreeValue(context, value);
    JS_FreeContext(context);
    JS_FreeRuntime(runtime);
    return 1;
  }

  int32_t result = 0;
  JS_ToInt32(context, &result, value);
  JS_FreeValue(context, value);
  JS_FreeContext(context);
  JS_FreeRuntime(runtime);

  std::cout << "QuickJS result: " << result << '\n';
  return 0;
}
