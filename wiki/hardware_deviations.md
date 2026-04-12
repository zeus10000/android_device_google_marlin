# Hardware Deviations — Google Pixel XL (marlin) on LineageOS 23.2

## Peripheral Manager (vendor.per_mgr)

**Affected feature:** GPS subsystem initialization timing

**Stock behavior:** Qualcomm pm-service binary manages peripheral power states
and provides modem readiness information to the GNSS HAL.

**LineageOS 23.2 behavior:** The stock pm-service binary (compiled for Android 10)
crashes on Android 16 due to binder wire protocol incompatibility. Android 16
introduced separate parcel headers for system ("SYST") and vendor ("VNDR") binder
contexts. The legacy binary sends VNDR-formatted parcels to the system
servicemanager, which rejects them, causing a null pointer crash during cleanup.

**Mitigation:** A clean-room open-source PeripheralManager stub
(`vendor.peripheral_mgr_stub`) replaces the proprietary binary. It registers
`vendor.qcom.PeripheralManager` with vndservicemanager and responds to all
binder queries with OK status, allowing the GNSS HAL to initialize and GPS
to function normally.

**User impact:** None expected. GPS functions normally via the stub. If any
advanced peripheral power management features are needed (modem subsystem
restart monitoring), they will not be available.

## CLAT BPF Acceleration

**Affected feature:** 464XLAT (IPv6-only network IPv4 compatibility)

**Stock behavior:** CLAT uses BPF programs for accelerated packet translation.

**LineageOS 23.2 behavior:** The clatd BPF programs require BPF helper functions
not available in the 4.4 kernel backport. ClatCoordinator logs a warning and
CLAT falls back to userspace packet processing via the `clatd` daemon.

**User impact:** None. 464XLAT works identically via userspace processing.
Throughput on IPv6-only networks may be marginally lower under heavy load.
