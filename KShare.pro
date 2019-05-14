TEMPLATE = subdirs
SUBDIRS += src

DISTFILES += \
    README.md \
    LICENSE \
    OlderSystemFix.patch \
    AppVeyor/appveyor.yml \
    AppVeyor/make_installer.sh \
    .circleci/config.yml \
    .travis.yml \
    install.sh
