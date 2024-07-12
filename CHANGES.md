# deinvert changelog

## 1.0 (2024-07-12)

* Maintenance release; no new features
* Long-term maintainability:
  * Add basic end-to-end tests, .clang-format, and build check pipelines
  * Use meson (instead of autotools - which stopped working for me) to find deps & build
  * Remove unmaintained macro-based build options
    * liquid-dsp and sndfile are now actual dependencies
* Fixes:
  * Default-initialize buffers
* Documentation:
  * There is a [wiki](https://github.com/windytan/deinvert/wiki) and more (in-)security reminders

## 0.3 (2018-05-31)

* Reduce artifacts from DC removal filter at startup
* Warn when falling back to defaults

## 0.2 (2018-01-03)

* Add DC removal filter to fix 'beeping' issues
