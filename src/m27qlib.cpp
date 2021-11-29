#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "libusbk/libusbk.h"
#include "m27qlib/m27qlib.h"

static UINT Delay{ 50 };

static const uint16_t VID{ 0x2109 };
static const uint16_t PID{ 0x8883 };

static KUSB_DRIVER_API Usb;
static KUSB_HANDLE usbHandle = NULL;
static KLST_DEVINFO_HANDLE deviceInfo = NULL;

UINT M27Q_Init()
{
    KLST_HANDLE deviceList = NULL;

    LstK_Init(&deviceList, KLST_FLAG_NONE);

    if (!LstK_FindByVidPid(deviceList, VID, PID, &deviceInfo))
    {
        return GetLastError();
    };

    LibK_LoadDriverAPI(&Usb, deviceInfo->DriverID);

    if (!Usb.Init(&usbHandle, deviceInfo))
    {
        return GetLastError();
    };

    Usb.Init(&usbHandle, deviceInfo);

    LstK_Free(deviceList);

    return 420;
}

UINT M27Q_DeInit()
{
    if (usbHandle)
        Usb.Free(usbHandle);
    return 0;
}

PUINT M27Q_UsbWrite(UCHAR request, USHORT value, USHORT index, USHORT length, PUCHAR buffer)
{
    PUINT transferred{0};
    WINUSB_SETUP_PACKET setupPacket;
    setupPacket.RequestType = 0x40;
    setupPacket.Request = request;
    setupPacket.Value = value;
    setupPacket.Index = index;
    setupPacket.Length = length;

    Usb.ControlTransfer(usbHandle, setupPacket, buffer, length, transferred, NULL);

    Sleep(Delay);

    return transferred;
}

PUINT M27Q_UsbRead(UCHAR request, USHORT value, USHORT index, USHORT length, PUCHAR buffer)
{
    PUINT transferred{0};
    WINUSB_SETUP_PACKET setupPacket;
    setupPacket.RequestType = 0xC0;
    setupPacket.Request = request;
    setupPacket.Value = value;
    setupPacket.Index = index;
    setupPacket.Length = length;

    Sleep(50);

    Usb.ControlTransfer(usbHandle, setupPacket, buffer, length, transferred, NULL);

    return transferred;
}

UCHAR M27Q_GetOSD(PUCHAR data, USHORT length)
{
    UCHAR buffer[64] = {
        0x6E,
        0x51,
        static_cast<UCHAR>(0x81 + length),
        0x01 };

    for (UINT idx = 0; idx < length; idx++)
    {
        buffer[4 + idx] = data[idx];
    }

    M27Q_UsbWrite(178, 0, 0, length + 4, buffer);
    M27Q_UsbRead(162, 0, 111, 12, buffer);

    return buffer[10];
}

void M27Q_SetOSD(PUCHAR data, USHORT length)
{
    UCHAR buffer[64]{
        0x6E,
        0x51,
        static_cast<UCHAR>(0x81 + length),
        0x03 };

    for (UINT idx = 0; idx < length; idx++)
    {
        buffer[4 + idx] = data[idx];
    }

    M27Q_UsbWrite(178, 0, 0, length + 4, buffer);
}

UINT M27Q_GetBrightness()
{
    UCHAR data[1]{ 0x10 };
    return static_cast<UINT>(M27Q_GetOSD(data, 1));
}

UINT M27Q_SetBrightness(UINT value)
{
    UCHAR data[3] = { 0x10, 0x00, static_cast<UCHAR>(std::max<UINT>(0, std::min<UINT>(100, value))) };
    M27Q_SetOSD(data, 3);
    return 0;
}

UINT M27Q_SetCrosshair(UINT value)
{
    UCHAR data[3] = { 0xE0, 0x37, static_cast<UCHAR>(value) };
    M27Q_SetOSD(data, 3);
    return 0;
}

UINT M27Q_SetDelay(UINT value)
{
    Delay = value;
    return 0;
}