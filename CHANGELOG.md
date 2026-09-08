# Changelog

## [1.3.0](https://github.com/smarthome-wroclaw/esphome-loxone/compare/v1.2.2...v1.3.0) (2026-09-08)

First release of the [smarthome-wroclaw](https://github.com/smarthome-wroclaw/esphome-loxone)
fork of [hzkincony/esphome-loxone](https://github.com/hzkincony/esphome-loxone).

### Bug Fixes

* build on ESPHome 2026.x — replace the legacy `ESP32 Async UDP` library name (which ESPHome 2026's resolver rejects with `UnknownPackageError`) with the selective-compilation names `Network` + `AsyncUDP`, and gate the schema with `cv.only_with_arduino` so an `esp-idf` build fails config validation with a clear message instead of a mid-compile crash ([a2d864a](https://github.com/smarthome-wroclaw/esphome-loxone/commit/a2d864a73fa4dc7e2086d080f43d7cdda8ffe169))

### Miscellaneous

* set up release-please; bump `actions/checkout` v2 → v4 ([#2](https://github.com/smarthome-wroclaw/esphome-loxone/issues/2)) ([cadc919](https://github.com/smarthome-wroclaw/esphome-loxone/commit/cadc9190b895f625b44fa6e780daab043e180942))
