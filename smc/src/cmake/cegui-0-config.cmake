set(prefix "/usr/local") 
set(exec_prefix "${prefix}")
set(libdir "${prefix}/lib")
set(CEGUI_PREFIX "${prefix}")
set(CEGUI_INCLUDE_DIRS "${prefix}/include/cegui-0")
set(CEGUI_LIBRARIES " -lCEGUIOpenGLRenderer-0 -lCEGUIBase-0")
string(STRIP "${CEGUI_LIBRARIES}" CEGUI_LIBRARIES)


