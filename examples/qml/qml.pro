TEMPLATE = subdirs
QT_FOR_CONFIG += qml

qtHaveModule(quick) {
    SUBDIRS += \
        qmlextensionplugins
}

SUBDIRS += \
          referenceexamples \
          tutorials

EXAMPLE_FILES = \
    dynamicscene \
    qml-i18n \
    locale
