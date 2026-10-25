# Changelog

## [1.10.0](https://github.com/smarthome-wroclaw/esphome-loxone/compare/v1.9.0...v1.10.0) (2026-10-25)


### Features

* generate Loxone template page for automatic entities ([#24](https://github.com/smarthome-wroclaw/esphome-loxone/issues/24)) ([438fd07](https://github.com/smarthome-wroclaw/esphome-loxone/commit/438fd07da83104ace1fa018a1bf20ee1a940e415))

## [1.9.0](https://github.com/smarthome-wroclaw/esphome-loxone/compare/v1.8.0...v1.9.0) (2026-10-25)


### Features

* auto-control lights from Loxone commands ([#23](https://github.com/smarthome-wroclaw/esphome-loxone/issues/23)) ([fbd2887](https://github.com/smarthome-wroclaw/esphome-loxone/commit/fbd28873e4204d196c6008b7a855e6061ea6f4a8))
* auto-send light brightness to Loxone ([#21](https://github.com/smarthome-wroclaw/esphome-loxone/issues/21)) ([b0cff28](https://github.com/smarthome-wroclaw/esphome-loxone/commit/b0cff285b10e517ce4ffce01e21c29e03a71342c))

## [1.8.0](https://github.com/smarthome-wroclaw/esphome-loxone/compare/v1.7.0...v1.8.0) (2026-10-25)


### Features

* auto-send sensor states to Loxone ([#19](https://github.com/smarthome-wroclaw/esphome-loxone/issues/19)) ([ec6bb6b](https://github.com/smarthome-wroclaw/esphome-loxone/commit/ec6bb6b757a12f5cb48e4103ee37bc9bf22de361))

## [1.7.0](https://github.com/smarthome-wroclaw/esphome-loxone/compare/v1.6.1...v1.7.0) (2026-09-09)


### Features

* readable TX/RX logging for Loxone traffic ([#17](https://github.com/smarthome-wroclaw/esphome-loxone/issues/17)) ([8aeaf65](https://github.com/smarthome-wroclaw/esphome-loxone/commit/8aeaf650bd205c2cea8cf6093658b89a391b75a5))

## [1.6.1](https://github.com/smarthome-wroclaw/esphome-loxone/compare/v1.6.0...v1.6.1) (2026-09-08)


### Documentation

* document the template: block and /loxone/ endpoint ([#15](https://github.com/smarthome-wroclaw/esphome-loxone/issues/15)) ([b643598](https://github.com/smarthome-wroclaw/esphome-loxone/commit/b6435983f2bc68b98fbd7516156996b929ea62b8))

## [1.6.0](https://github.com/smarthome-wroclaw/esphome-loxone/compare/v1.5.0...v1.6.0) (2026-09-08)


### Features

* /loxone/ endpoint serving importable Loxone Config templates ([#13](https://github.com/smarthome-wroclaw/esphome-loxone/issues/13)) ([bb98796](https://github.com/smarthome-wroclaw/esphome-loxone/commit/bb9879658ddb75776f1bf57236ec14e750213872))

## [1.5.0](https://github.com/smarthome-wroclaw/esphome-loxone/compare/v1.4.0...v1.5.0) (2026-09-08)


### Features

* connection status entities (connected / last_message / last_message_age) ([#11](https://github.com/smarthome-wroclaw/esphome-loxone/issues/11)) ([1023fd3](https://github.com/smarthome-wroclaw/esphome-loxone/commit/1023fd31ae72ecc6adf85333fd513e38f6ed310b))

## [1.4.0](https://github.com/smarthome-wroclaw/esphome-loxone/compare/v1.3.1...v1.4.0) (2026-09-08)


### Features

* dump_config + optional config text sensors ([#9](https://github.com/smarthome-wroclaw/esphome-loxone/issues/9)) ([a0da486](https://github.com/smarthome-wroclaw/esphome-loxone/commit/a0da4865055f51bfca8cec4e90976e3acc8791d2))

## [1.3.1](https://github.com/smarthome-wroclaw/esphome-loxone/compare/v1.3.0...v1.3.1) (2026-09-08)


### Bug Fixes

* do not call AsyncClient::remoteIP() (fails to link) ([#7](https://github.com/smarthome-wroclaw/esphome-loxone/issues/7)) ([551b35c](https://github.com/smarthome-wroclaw/esphome-loxone/commit/551b35ce8c984dd06c29f0cbb3072c20cf6cf456))

## [1.3.0](https://github.com/smarthome-wroclaw/esphome-loxone/compare/v1.2.2...v1.3.0) (2026-09-08)

First release of the [smarthome-wroclaw](https://github.com/smarthome-wroclaw/esphome-loxone)
fork of [hzkincony/esphome-loxone](https://github.com/hzkincony/esphome-loxone).

### Bug Fixes

* build on ESPHome 2026.x — replace the legacy `ESP32 Async UDP` library name (which ESPHome 2026's resolver rejects with `UnknownPackageError`) with the selective-compilation names `Network` + `AsyncUDP`, and gate the schema with `cv.only_with_arduino` so an `esp-idf` build fails config validation with a clear message instead of a mid-compile crash ([a2d864a](https://github.com/smarthome-wroclaw/esphome-loxone/commit/a2d864a73fa4dc7e2086d080f43d7cdda8ffe169))

### Miscellaneous

* set up release-please; bump `actions/checkout` v2 → v4 ([#2](https://github.com/smarthome-wroclaw/esphome-loxone/issues/2)) ([cadc919](https://github.com/smarthome-wroclaw/esphome-loxone/commit/cadc9190b895f625b44fa6e780daab043e180942))
