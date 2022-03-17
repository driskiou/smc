set(prefix "/usr/local") 
set(exec_prefix "${prefix}")
set(libdir "${prefix}/lib")
set(CEGUI_OPENGL_PREFIX "${prefix}")
set(CEGUI_OPENGL_INCLUDE_DIRS "${prefix}/include/cegui-0")
set(CEGUI_OPENGL_LIBRARIES " lCEGUIOpenGLRenderer-0 -lCEGUIBase-0 ")
string(STRIP "${CEGUI_OPENGL_LIBRARIES}" CEGUI_OPENGL_LIBRARIES)


