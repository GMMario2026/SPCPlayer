QT += multimedia openglwidgets

SOURCES += \
    pyin/Yin.cpp \
    pyin/YinUtil.cpp \
    snes_spc/SNES_SPC_misc.cpp \
    snes_spc/SNES_SPC_state.cpp \
    snes_spc/SNES_SPC.cpp \
    snes_spc/SPC_DSP.cpp \
    split700/SPCFile.cpp \
    split700/SPCSampDir.cpp \
    vamp-sdk/FFT.cpp \
    vamp-sdk/FFTimpl.cpp \
    Config.cc \
    ConfigDialog.cc \
    GameState.cc \
    MainWindow.cc \
    Misc.cc \
    Player.cc \
    Renderer.cc \
    SPC.cc

HEADERS += \
    pyin/MeanFilter.h \
    pyin/Yin.h \
    pyin/YinUtil.h \
    snes_spc/blargg_common.h \
    snes_spc/blargg_config.h \
    snes_spc/blargg_endian.h \
    snes_spc/blargg_source.h \
    snes_spc/SNES_SPC.h \
    snes_spc/SPC_CPU.h \
    snes_spc/SPC_DSP.h \
    split700/cpath.h \
    split700/SPCFile.h \
    split700/SPCSampDir.h \
    vamp-sdk/FFT.h \
    vamp-sdk/plugguard.h \
    Config.h \
    ConfigDialog.h \
    GameState.h \
    Globals.h \
    MainWindow.h \
    Misc.h \
    Renderer.h \
    SPC.h

FORMS += ConfigDialog.ui

RESOURCES = Player.qrc

RC_ICONS = "Images/SPCPlayer Icon.ico"
