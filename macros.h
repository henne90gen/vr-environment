#pragma once

// TODO this macro is a hack. The implementation if taken from gl_context.cxx/get_gl_id
#define get_gl_id(x) (const GLuint &)(x) - 1
