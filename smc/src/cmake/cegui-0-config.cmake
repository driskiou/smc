set(prefix "/usr/") 
set(exec_prefix "${prefix}")
set(libdir "${prefix}/lib/x86_64-linux-gnu/")
set(CEGUI_PREFIX "${prefix}")
set(CEGUI_INCLUDE_DIRS "${prefix}/include/x86_64-linux-gnu/cegui-0.8.7")
set(CEGUI_LIBRARIES " -lCEGUIBase-0")
string(STRIP "${CEGUI_LIBRARIES}" CEGUI_LIBRARIES)


