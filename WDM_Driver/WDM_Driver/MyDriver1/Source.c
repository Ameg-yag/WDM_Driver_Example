// Don't use ntddk.h as wdm is forward moving (driver will be forward compatible)
#include <wdm.h>


// Define the device name
UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\wdm_driver_test");
UNICODE_STRING SymLinkName = RTL_CONSTANT_STRING(L"\\??\\wdm_driver_test");

PDEVICE_OBJECT DeviceObject = NULL;

VOID Unload(PDRIVER_OBJECT DriverObject)
{
	// WDK "Treat warning as error" activated. Using the UNREFERENCED_PARAMETER macro
	UNREFERENCED_PARAMETER(DriverObject);
	IoDeleteSymbolicLink(&SymLinkName);
	IoDeleteDevice(DeviceObject);
	KdPrint(("Driver unloaded \r\n"));
}

// https://docs.microsoft.com/en-us/windows-hardware/drivers/wdf/driverentry-for-kmdf-drivers
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	NTSTATUS status = STATUS_SUCCESS;

	DriverObject->DriverUnload = Unload;

	// Create a device
	// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iocreatedevice
	IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);

	// If not successfully creating the device, print error
	if (!NT_SUCCESS(status)) {
		KdPrint(("Device creation failed \r\n"));
		return status;
	}

	// Create a symbolic link to expose the device to a user-mode application
	IoCreateSymbolicLink(&SymLinkName, &DeviceName);

	// Check return value for errors
	if (!NT_SUCCESS(status)) {
		KdPrint(("Symlink creation failed \r\n"));
		// Delete device
		IoDeleteDevice(DeviceObject);
		return status;
	}

	KdPrint(("Driver successfully loaded \r\n"));
	return status;
}