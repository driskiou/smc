set(prefix "/usr/") 
set(exec_prefix "${prefix}")
set(libdir "${prefix}/lib/x86_64-linux-gnu/")
set(CEGUI_PREFIX "${prefix}")
set(CEGUI_INCLUDE_DIRS "${prefix}/include/x86_64-linux-gnu/cegui-0.8.7")
set(CEGUI_OPENGL_LIBRARIES " -lCEGUIOpenGLRenderer-0 -lCEGUINullRenderer-0 -lGL -lGLU")
string(STRIP "${CEGUI_OPENGL_LIBRARIES}" CEGUI_OPENGL_LIBRARIES)


