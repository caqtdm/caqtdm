# iOS CI build

The workflow builds Qt 6.12.0, OpenSSL from the latest patch in the selected
3.5 LTS series, Qwt 6.3.0, and EPICS Base R7.0.10 for arm64 iPhoneOS and
arm64 iOS Simulator. The iOS minimum version is 18.0 across these dependencies
and the caQtDM app. Qt, OpenSSL, EPICS, and Qwt have separate caches keyed to
their build inputs and the Xcode/SDK versions.

The iPhoneOS artifact is unsigned. It verifies that caQtDM compiles and links
for a device, but it cannot be installed on an iPhone without a signing identity
and provisioning profile. The simulator artifact is also a build artifact; this
workflow does not run a simulator or test live EPICS, Modbus, GPS, or OPC UA
connections.
