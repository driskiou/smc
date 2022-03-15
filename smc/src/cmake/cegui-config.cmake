set(prefix "/usr/local") 
set(exec_prefix "${prefix}")
set(libdir "${prefix}/lib")
set(CEGUI_PREFIX "${prefix}")
set(CEGUI_INCLUDE_DIRS "${prefix}/include/CEGUI")
set(CEGUI_LIBRARIES " -lCEGUIBase -lCEGUINullRenderer")
string(STRIP "${CEGUI_LIBRARIES}" CEGUI_LIBRARIES)


