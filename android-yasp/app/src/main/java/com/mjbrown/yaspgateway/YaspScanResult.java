package com.mjbrown.yaspgateway;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.ScanResult;

import java.sql.Timestamp;

public class YaspScanResult {
    private Timestamp timeFound;
    private Timestamp timeLastSeen;
    private ScanResult scanResult;

    YaspScanResult(ScanResult scanResult) {
        timeFound = new Timestamp(new java.util.Date().getTime());
        timeLastSeen = new Timestamp(new java.util.Date().getTime());
        updateScanResult(scanResult);
    }

    public void updateScanResult(ScanResult scanResult) {
        this.scanResult = scanResult;
        timeLastSeen.setTime(new java.util.Date().getTime());
    }

    public boolean isOlderThan(long time) {
        long now = new java.util.Date().getTime();
        return (timeLastSeen.getTime() < (now - time));
    }

    public Integer getScanRssi() { return scanResult.getRssi(); }

    public Timestamp getTimeFound() { return timeFound; }

    public BluetoothDevice getBluetoothDevice() {
        return scanResult.getDevice();
    }
}
