DEFINES += QGC_ENABLE_BLUETOOTH

QT += core gui widgets qml quick network positioning \
location sql texttospeech serialport xml svg

contains(DEFINES, QGC_ENABLE_BLUETOOTH){
    QT += bluetooth
}

include($$PWD/../../common-librarys/libs.pri)
include($$PWD/../VGQtLocationPlugin/QGCLocationPlugin.pri)
include($$PWD/VGPlantViewerUI/VGPlantViewerUI.pri)

AndroidBuild {
    QT += androidextras
}
WindowsBuild{
    QT += axcontainer
}

CONFIG += qt \
    thread \
    c++11

INCLUDEPATH +=  $$PWD/../protobufSrc \
                $$PWD/../base \
                $$PWD/../share \
                $$PWD/../VGQtLocationPlugin \
                $$PWD/../PX4 \
                $$PWD/VGPlantViewerUI \
                $$PWD/VGPlantViewerUI/land \
                $$PWD/VGPlantViewerUI/plant \
                $$PWD/VGPlantViewerUI/survey \
                $$PWD/VGPlantViewerUI/qx \
                $$PWD/../../opensource-libraries/mavlink/v2.0/common \
                $$PWD/../../opensource-libraries/mavlink/v2.0 \
                $$PWD/../../common-librarys/vgbase/src \
                $$PWD/../../common-librarys/vgcomm/src



# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.

SOURCES += main.cpp \
    VGApplication.cpp \
    CmdLineOptParser.cc \
    das.pb.cc \
    VGDbManager.cpp \
    VGNetManager.cpp \
    VGTcpClient.cpp \
    VGToolBox.cpp \
    VGVehicle.cpp \
    VGVehicleLogManager.cpp \
    VGVehicleManager.cpp \
    srcload/VGIconEngine.cpp \
    srcload/VGImageProvider.cpp \
    ../base/logHelper.cpp \
    ../base/VGGlobalFun.cpp \
    ../base/JQChecksum.cpp \
    ../base/VGMavLinkCode.cpp \
    ../base/ParameterLimit.cpp \
    ../base/ParametersLimit.cpp \
    ../protobufSrc/google/protobuf/compiler/importer.cc \
    ../protobufSrc/google/protobuf/compiler/parser.cc \
    ../protobufSrc/google/protobuf/io/coded_stream.cc \
    ../protobufSrc/google/protobuf/io/gzip_stream.cc \
    ../protobufSrc/google/protobuf/io/printer.cc \
    ../protobufSrc/google/protobuf/io/tokenizer.cc \
    ../protobufSrc/google/protobuf/io/zero_copy_stream.cc \
    ../protobufSrc/google/protobuf/io/zero_copy_stream_impl.cc \
    ../protobufSrc/google/protobuf/io/zero_copy_stream_impl_lite.cc \
    ../protobufSrc/google/protobuf/stubs/atomicops_internals_x86_gcc.cc \
    ../protobufSrc/google/protobuf/stubs/atomicops_internals_x86_msvc.cc \
    ../protobufSrc/google/protobuf/stubs/common.cc \
    ../protobufSrc/google/protobuf/stubs/once.cc \
    ../protobufSrc/google/protobuf/stubs/stringprintf.cc \
    ../protobufSrc/google/protobuf/stubs/structurally_valid.cc \
    ../protobufSrc/google/protobuf/stubs/strutil.cc \
    ../protobufSrc/google/protobuf/stubs/substitute.cc \
    ../protobufSrc/google/protobuf/descriptor.cc \
    ../protobufSrc/google/protobuf/descriptor.pb.cc \
    ../protobufSrc/google/protobuf/descriptor_database.cc \
    ../protobufSrc/google/protobuf/dynamic_message.cc \
    ../protobufSrc/google/protobuf/extension_set.cc \
    ../protobufSrc/google/protobuf/extension_set_heavy.cc \
    ../protobufSrc/google/protobuf/generated_message_reflection.cc \
    ../protobufSrc/google/protobuf/generated_message_util.cc \
    ../protobufSrc/google/protobuf/message.cc \
    ../protobufSrc/google/protobuf/message_lite.cc \
    ../protobufSrc/google/protobuf/reflection_ops.cc \
    ../protobufSrc/google/protobuf/repeated_field.cc \
    ../protobufSrc/google/protobuf/service.cc \
    ../protobufSrc/google/protobuf/text_format.cc \
    ../protobufSrc/google/protobuf/unknown_field_set.cc \
    ../protobufSrc/google/protobuf/wire_format.cc \
    ../protobufSrc/google/protobuf/wire_format_lite.cc \
    MissionItem.cc \
    MissionManager.cc \
    JsonHelper.cc

RESOURCES += VGPlantViewer.qrc

HEADERS += \
    VGApplication.h \
    CmdLineOptParser.h \
    das.pb.h \
    VGDbManager.h \
    VGNetManager.h \
    VGTcpClient.h \
    VGToolBox.h \
    VGVehicle.h \
    VGVehicleLogManager.h \
    VGVehicleManager.h \
    srcload/VGIconEngine.h \
    srcload/VGImageProvider.h \
    ../base/JQChecksum.h \
    ../base/VGMavLinkCode.h \
    ../base/logHelper.h \
    ../base/QLog.h \
    ../base/VGGlobalFun.h \
    ../base/ParameterLimit.h \
    ../base/ParametersLimit.h \
    ../protobufSrc/google/protobuf/compiler/importer.h \
    ../protobufSrc/google/protobuf/compiler/parser.h \
    ../protobufSrc/google/protobuf/io/coded_stream.h \
    ../protobufSrc/google/protobuf/io/coded_stream_inl.h \
    ../protobufSrc/google/protobuf/io/gzip_stream.h \
    ../protobufSrc/google/protobuf/io/printer.h \
    ../protobufSrc/google/protobuf/io/tokenizer.h \
    ../protobufSrc/google/protobuf/io/zero_copy_stream.h \
    ../protobufSrc/google/protobuf/io/zero_copy_stream_impl.h \
    ../protobufSrc/google/protobuf/io/zero_copy_stream_impl_lite.h \
    ../protobufSrc/google/protobuf/stubs/atomicops.h \
    ../protobufSrc/google/protobuf/stubs/atomicops_internals_arm_gcc.h \
    ../protobufSrc/google/protobuf/stubs/atomicops_internals_arm_qnx.h \
    ../protobufSrc/google/protobuf/stubs/atomicops_internals_atomicword_compat.h \
    ../protobufSrc/google/protobuf/stubs/atomicops_internals_macosx.h \
    ../protobufSrc/google/protobuf/stubs/atomicops_internals_mips_gcc.h \
    ../protobufSrc/google/protobuf/stubs/atomicops_internals_pnacl.h \
    ../protobufSrc/google/protobuf/stubs/atomicops_internals_x86_gcc.h \
    ../protobufSrc/google/protobuf/stubs/atomicops_internals_x86_msvc.h \
    ../protobufSrc/google/protobuf/stubs/common.h \
    ../protobufSrc/google/protobuf/stubs/hash.h \
    ../protobufSrc/google/protobuf/stubs/map-util.h \
    ../protobufSrc/google/protobuf/stubs/once.h \
    ../protobufSrc/google/protobuf/stubs/platform_macros.h \
    ../protobufSrc/google/protobuf/stubs/stl_util.h \
    ../protobufSrc/google/protobuf/stubs/stringprintf.h \
    ../protobufSrc/google/protobuf/stubs/strutil.h \
    ../protobufSrc/google/protobuf/stubs/substitute.h \
    ../protobufSrc/google/protobuf/stubs/template_util.h \
    ../protobufSrc/google/protobuf/stubs/type_traits.h \
    ../protobufSrc/google/protobuf/descriptor.h \
    ../protobufSrc/google/protobuf/descriptor.pb.h \
    ../protobufSrc/google/protobuf/descriptor_database.h \
    ../protobufSrc/google/protobuf/dynamic_message.h \
    ../protobufSrc/google/protobuf/extension_set.h \
    ../protobufSrc/google/protobuf/generated_enum_reflection.h \
    ../protobufSrc/google/protobuf/generated_message_reflection.h \
    ../protobufSrc/google/protobuf/generated_message_util.h \
    ../protobufSrc/google/protobuf/message.h \
    ../protobufSrc/google/protobuf/message_lite.h \
    ../protobufSrc/google/protobuf/reflection_ops.h \
    ../protobufSrc/google/protobuf/repeated_field.h \
    ../protobufSrc/google/protobuf/service.h \
    ../protobufSrc/google/protobuf/text_format.h \
    ../protobufSrc/google/protobuf/unknown_field_set.h \
    ../protobufSrc/google/protobuf/wire_format.h \
    ../protobufSrc/google/protobuf/wire_format_lite.h \
    ../protobufSrc/google/protobuf/wire_format_lite_inl.h \
    ../protobufSrc/config.h \
    MissionItem.h \
    MissionManager.h \
    JsonHelper.h
    
AndroidBuild {
    OTHER_FILES += \
        android/AndroidManifest.xml \
        android/src/QGCActivity.java \
        android/res/xml/device_filter.xml

    contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
        CONFIG(debug, debug|release) {
                ANDROID_EXTRA_LIBS = \
                    $$OUT_PWD/../../output/debug/libvgbase.so \
                    $$OUT_PWD/../../output/debug/libvgcomm.so \
                    $$OUT_PWD/../../output/debug/libvglog.so \
                    $$OUT_PWD/../../output/debug/libshare.so \
        }
        CONFIG(release, debug|release) {
                ANDROID_EXTRA_LIBS = \
                    $$OUT_PWD/../../output/release/libvgbase.so \
                    $$OUT_PWD/../../output/release/libvgcomm.so \
                    $$OUT_PWD/../../output/release/libvglog.so \
                    $$OUT_PWD/../../output/release/libshare.so \
        }
    }
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    xml.files += UavParamsAndEvents.xml
    xml.path = /assets/res
    INSTALLS += xml
}

LIBS += -L$${OUT_ROOT} -lvgbase -lvgcomm -lvglog -lshare
