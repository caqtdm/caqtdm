# EPICS iOS build configuration

The iOS workflow builds EPICS Base R7.0.10 for arm64 iPhoneOS and arm64 iOS
Simulator targets. These overrides are copied over the matching files in the
EPICS source tree before the build; they are specific to the Xcode SDKs on the
runner and the iOS 18 deployment target used with Qt 6.12.0.

The ios-arm target uses the iPhoneOS SDK. The ios-sim-arm64 target uses the
iPhoneSimulator SDK. Both produce static libraries so caQtDM can link EPICS
into its mobile app.
