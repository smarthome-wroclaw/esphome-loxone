# Changelog

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
