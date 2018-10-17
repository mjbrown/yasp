package com.mjbrown.yaspgateway;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class YaspService extends Service implements YaspDevice.YaspCallback{
    private final String TAG = "YaspService";
    public static String ACTION_SCAN_EVENT = "com.mjbrown.yaspgateway.SCAN_EVENT";
    public static String CONNECTION_EVENT = "com.mjbrown.yaspgateway.CONNECTION_EVENT";

    public class YaspBinder extends Binder { public YaspService getYaspService() { return YaspService.this; } }
    private final IBinder iBinder = new YaspBinder();

    @Override
    public IBinder onBind(Intent intent) {
        return iBinder;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_STICKY;
    }

    @Override
    public void onStateChange(YaspDevice yaspDevice, int previous, int next) {
        Intent intent = new Intent();
        intent.setAction(CONNECTION_EVENT);
        sendBroadcast(intent);
    }

    @Override
    public void onMessage(YaspDevice yaspDevice, YaspMessage yaspMessage) {

    }

    List<YaspDevice> yaspDevices = Collections.synchronizedList(new ArrayList<YaspDevice>());

    public List<YaspDevice> getYaspDevices() { return yaspDevices; }

    public void connectDevice(YaspScanResult scanResult) {
        YaspDevice yaspDevice;
        found: {
            for (YaspDevice device: yaspDevices) {
                if (device.getAddress().equals(scanResult.getBluetoothDevice().getAddress())) {
                    yaspDevice = device;
                    break found;
                }
            }
            yaspDevice = new YaspDevice(scanResult.getBluetoothDevice());
            yaspDevices.add(yaspDevice);
        }
        yaspScanResults.remove(scanResult);
        Intent intent = new Intent();
        intent.setAction(ACTION_SCAN_EVENT);
        sendBroadcast(intent);
        yaspDevice.connect(this, getApplicationContext());
    }

    List<YaspScanResult> yaspScanResults = Collections.synchronizedList(new ArrayList<YaspScanResult>());

    public List<YaspScanResult> getScanResults() {
        return yaspScanResults;
    }

    public void resetScanResults() {
        yaspScanResults.clear();
    }

    private ScanCallback scanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            super.onScanResult(callbackType, result);
            //Log.d(TAG, "onScanResult()");
            found: {
                for (YaspScanResult yaspScanResult: yaspScanResults) {
                    if (yaspScanResult.getBluetoothDevice().getAddress().equals(result.getDevice().getAddress())) {
                        yaspScanResult.updateScanResult(result);
                        break found;
                    }
                }
                yaspScanResults.add(new YaspScanResult(result));
                Intent intent = new Intent();
                intent.setAction(ACTION_SCAN_EVENT);
                sendBroadcast(intent);
            }
        }

        @Override
        public void onBatchScanResults(List<ScanResult> results) {
            super.onBatchScanResults(results);
            Log.e(TAG, "onBatchScanResults called unexpectedly!");
        }

        @Override
        public void onScanFailed(int errorCode) {
            super.onScanFailed(errorCode);
            Log.e(TAG, String.format("onScanFailed called with error code %d!", errorCode));
        }
    };

    ScanSettings scanSettings = new ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_BALANCED).build();
    BluetoothLeScanner bluetoothLeScanner = BluetoothAdapter.getDefaultAdapter().getBluetoothLeScanner();
    List<ScanFilter> scanFilters = new ArrayList<>();
    boolean isScanning = false;

    void startScan() {
        if (!isScanning) {
            isScanning = true;
            bluetoothLeScanner.startScan(scanFilters, scanSettings, scanCallback);
        }
    }

    void stopScan() {
        if (isScanning) {
            isScanning = false;
            bluetoothLeScanner.stopScan(scanCallback);
        }
    }
}
