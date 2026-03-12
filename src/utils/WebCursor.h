#pragma once

#ifdef __EMSCRIPTEN__
#ifdef __cplusplus
extern "C" {
#endif

	void SetCanvasCursor(const char* cssCursor);

#ifdef __cplusplus
}
#endif
#endif