# KShare
A [ShareX](https://getsharex.com/) inspired cross platform utility written with Qt.  
Originally written by [ArsenArsen](https://github.com/ArsenArsen) and here enhanced with [these](https://github.com/Gurkengewuerz/KShare/projects/1) features.

|  | Linux | Windows |
|--------|-------------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| master | [![CircleCI](https://circleci.com/gh/Gurkengewuerz/KShare/tree/master.svg?style=svg)](https://circleci.com/gh/Gurkengewuerz/KShare/tree/master) | [![Build status](https://ci.appveyor.com/api/projects/status/ujxmg1dk7f5p8ijh/branch/master?svg=true)](https://ci.appveyor.com/project/Gurkengewuerz/kshare/branch/master) |
| dev | [![CircleCI](https://circleci.com/gh/Gurkengewuerz/KShare/tree/dev.svg?style=svg)](https://circleci.com/gh/Gurkengewuerz/KShare/tree/dev) | [![Build status](https://ci.appveyor.com/api/projects/status/ujxmg1dk7f5p8ijh/branch/dev?svg=true)](https://ci.appveyor.com/project/Gurkengewuerz/kshare/branch/dev) |

## Screenshot
Made with KShare itself, of course :)  
![](https://i.imgur.com/oJrCNkq.png)

## 🎉 Features
* 💻 open source
* 🪶 lightweight
* 🖧 cross-platform
* 🔍 magnifying lense for image capture
* ⌨️ customizable keyboard shortcuts
* ✒️ image annotation (_rectangle_, _ellipse_, _freehand_, _line_, _arrow_, _text_)
* 📂 upload screenshots, files, clipboard or text
* 🤏 color picker
* 📤 multiple destinations implemented (_imgur_, _gdrive_, _clipboard_)
* 🔧 custom uploader (_works perfectly with ![**php_filehost**](https://github.com/Gurkengewuerz/php_filehost)_)
* 🖌️ themes (_dark_, ![_breeze dark_, _breeze light_](https://github.com/Alexhuszagh/BreezeStyleSheets), ![_qdarkstyle_](https://github.com/ColinDuquesnoy/QDarkStyleSheet))

## Usage
Please note that KShare is not compatiable with Wayland due to some permission issues. Please use X.Org instead.

## Dependencies
* Qt 5 Widgets
* Qt 5 GUI
* Qt 5 Network
* Qt 5 Multimedia
* Qt 5 X11Extras | Winextras
* [QHotkey](https://github.com/Skycoder42/QHotkey)
* libavformat
* libavcodec
* libavutil
* libswscale

Additionally, on Linux, you require:
* XCB
* XCB xfixes
* XCB cursor
* Notifications Daemon with org.freedesktop.notifications DBus support (like dunst)

Despite the name implying so, this project does not depend on the KDE API at all.

## Install
|Distro|Link|
|:----:|:--:|
|Arch Linux (development)|[kshare-git](https://app.circleci.com/github/Gurkengewuerz/KShare/pipelines?branch=master)|
|Ubuntu/Debian (development)|[.deb](https://app.circleci.com/github/Gurkengewuerz/KShare/pipelines?branch=dev)|
|Windows (development)|[Installer](https://ci.appveyor.com/project/Gurkengewuerz/kshare/branch/dev/artifacts)|
|Arch Linux |[kshare](https://github.com/Gurkengewuerz/KShare/blob/master/packages/arch/Stable-KShare/PKGBUILD)|
|Ubuntu/Debian |[.deb](https://app.circleci.com/github/Gurkengewuerz/KShare/pipelines?branch=master)|
|Windows |[Installer](https://ci.appveyor.com/project/Gurkengewuerz/kshare/branch/master/artifacts)|

For other UNIX-like platforms, and MSYS2 (for Windows):

You have to obtain the dependencies though.
```bash
git clone --recursive https://github.com/Gurkengewuerz/KShare.git
cd KShare
qmake # Might be qmake-qt5 on your system
make
```

On systems with FFMpeg pre-3.1 you need to apply `OlderSystemFix.patch` to `src/recording/encoders/encoder.cpp`.
On systems with Qt pre-5.7 you need to install the Qt version from their website.
You can attempt to `curl https://raw.githubusercontent.com/Gurkengewuerz/KShare/master/install.sh | bash`

###### Started on 19th of April 2017 by [ArsenArsen](https://github.com/ArsenArsen) to bring some attention and improvement to Linux screenshotting.
