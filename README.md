# cremote

[![Build Status](https://travis-ci.com/vtavernier/cremote.svg?token=qW3fGvxWik6fwUsruShT&branch=master)](https://travis-ci.com/vtavernier/cremote)
[![GitHub release](https://img.shields.io/github/release/vtavernier/cremote)](https://github.com/vtavernier/cremote/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

This repository contains the source code for my DIY Canon EOS Bluetooth remote.
It is based on the technical details from
http://www.doc-diy.net/photo/eos_wired_remote/ for the hardware part.

The hardware is an [Arduino Pro Mini 8MHz](https://store.arduino.cc/arduino-pro-mini)
coupled to a [HM-11](http://wiki.seeedstudio.com/Bluetooth_V4.0_HM_11_BLE_Module/)
bluetooth module.

## Project structure

* [`cr-android`](cr-android/): Android application code
* [`cr-arduino`](cr-arduino/): Arduino program code

## Author

Vincent Tavernier <vince.tavernier@gmail.com>

## License

This project is licensed under the [MIT License](LICENSE).
