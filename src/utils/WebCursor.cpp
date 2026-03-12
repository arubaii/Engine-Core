#include "utils/WebCursor.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(void, SetCanvasCursor, (const char* cssCursor), {
	const canvas = document.getElementById('canvas');
	if (!canvas) return;
	canvas.style.cursor = UTF8ToString(cssCursor);
});
#endif