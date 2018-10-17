package com.mjbrown.yaspgateway;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.bluetooth.le.ScanResult;
import android.util.Log;

import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.Semaphore;

public class YaspDevice {
    private static String TAG = "YaspDevice";
    public static int YASP_FOUND = 1;
    public static int YASP_CONNECTING = 2;
    public static int YASP_CONNECTED = 3;
    public static int YASP_DISCONNECTING = 4;
    public static int YASP_DISCONNECTED = 5;
    public static int YASP_NO_SERIAL = 6;

    public interface YaspCallback {
        void onStateChange(YaspDevice yaspDevice, int previous, int next);
        void onMessage(YaspDevice yaspDevice, YaspMessage yaspMessage);
    }

    private BluetoothDevice bluetoothDevice;

    private Integer state = YASP_FOUND;
    private BluetoothGatt bluetoothGatt;
    private YaspGattCallback yaspGattCallback = new YaspGattCallback();

    YaspDevice(BluetoothDevice bluetoothDevice) {
        this.bluetoothDevice = bluetoothDevice;
    }

    public String getName() { return bluetoothDevice.getName(); }

    public String getAddress() { return bluetoothDevice.getAddress(); }

    private YaspCallback yaspCallback;

    private void stateChange(int nextState) {
        int previous = state;
        state = nextState;
        if (previous != state) {
            yaspCallback.onStateChange(this, previous, state);
        }
    }

    public int getConnectionState() {
        return state;
    }

    public void connect(YaspCallback yaspCallback, Context context) {
        this.yaspCallback = yaspCallback;
        if (bluetoothGatt != null) {
            Log.e(TAG, "Tried to call connect while already connected/connecting!");
        } else {
            stateChange(YASP_CONNECTING);
            bluetoothGatt = bluetoothDevice.connectGatt(context, false, yaspGattCallback);
        }
    }

    public void disconnect() {
        if (bluetoothGatt != null) {
            Log.i(TAG, "Disconnecting from" + bluetoothDevice.getName() + "...");
            stateChange(YASP_DISCONNECTING);
            bluetoothGatt.disconnect();
        }
    }

    protected class YaspGattCallback extends BluetoothGattCallback {
        Boolean txInProgress = false;
        private final Integer SERIAL_BLE_MTU = 20;
        private final Integer MAX_PACKET_PAYLOAD = 300;
        Semaphore transmitBufferLock = new Semaphore(1);

        private List<Byte> serialTransmitBuffer = new ArrayList<>();

        private List<Byte> serialReceiveBuffer = new ArrayList<>();

        private final UUID UUID_CC_CONFIG = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

        UUID getCustomServiceUUID() {
            return UUID.fromString("07323bac-43a2-4af1-b25e-7aa3aeed1c1d");
        }

        UUID getCustomCharacteristicUUID() {
            return UUID.fromString("07323bad-43a2-4af1-b25e-7aa3aeed1c1d");
        }

        public synchronized void serialTransmit(List<Byte> data) {
            try {
                transmitBufferLock.acquire();
                serialTransmitBuffer.addAll(data);
            } catch (InterruptedException e) {
                Log.e(TAG, "Interrupted exception in serialTransmit!");
            } finally {
                transmitBufferLock.release();
            }
            if (!txInProgress) {
                resumeTransmit();
            }
        }

        void resumeTransmit() {
            if (serialTransmitBuffer.size() > 0) {
                Integer writeSize = serialTransmitBuffer.size() > SERIAL_BLE_MTU ? SERIAL_BLE_MTU : serialTransmitBuffer.size();
                List<Byte> writeData = new ArrayList<>(serialTransmitBuffer.subList(0, writeSize));
                Log.i("PL4 GATT", "TX Data: " + UtilityFunctions.byteListToHex(writeData));
                getCustomGattCharacteristic().setValue(UtilityFunctions.serialize(writeData));
                txInProgress = bluetoothGatt.writeCharacteristic(getCustomGattCharacteristic());
                if (txInProgress) {
                    try {
                        transmitBufferLock.acquire();
                        serialTransmitBuffer = serialTransmitBuffer.subList(writeSize, serialTransmitBuffer.size());
                    } catch (InterruptedException e) {
                        Log.e(TAG, "Interrupted during resumeTransmit!");
                    } finally {
                        transmitBufferLock.release();
                    }
                } else {
                    Log.d("PL4 GATT", "writeCharacteristic returned false");
                }
            } else {
                txInProgress = false;
            }
        }

        synchronized void serialReceive(byte[] data) {
            for (byte b: data) {
                serialReceiveBuffer.add(b);
            }
            while (serialReceiveBuffer.size() > 1) {
                Log.i(TAG, "RX Buffer: " + UtilityFunctions.byteListToHex(serialReceiveBuffer));
                Integer payloadLength = serialReceiveBuffer.get(1)*256 + serialReceiveBuffer.get(0);
                if (payloadLength > MAX_PACKET_PAYLOAD) {
                    Log.e(TAG, String.format("Payload length exceeds maximum (%d > %d)!", payloadLength, MAX_PACKET_PAYLOAD));
                    serialReceiveBuffer.clear();
                    break;
                }
                if (serialReceiveBuffer.size() - 4 < payloadLength) {
                    break;
                } else {
                    Byte handle = serialReceiveBuffer.get(2);
                    Byte command = serialReceiveBuffer.get(3);
                    byte[] payload = new byte[payloadLength];
                    for (int i = 0; i < payloadLength; i++) {
                        payload[i] = serialReceiveBuffer.get(4+i);
                    }
                    Log.i(TAG,String.format("(Length: %d) (Handle: %d) (Command: %d)", payloadLength, handle,command));
                    serialReceiveBuffer = serialReceiveBuffer.subList(4+payloadLength, serialReceiveBuffer.size());

                    //messageReceived(command, handle, payload);
                }
            }
        }

        private BluetoothGattCharacteristic customGattCharacteristic;
        BluetoothGattCharacteristic getCustomGattCharacteristic() {
            return customGattCharacteristic;
        }

        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            super.onConnectionStateChange(gatt, status, newState);
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.i(TAG, gatt.getDevice().getAddress() + " - connection state connected.");
                gatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.i(TAG, gatt.getDevice().getAddress() + " - connection state disconnected.");
                bluetoothGatt.close();
                bluetoothGatt = null;
                stateChange(YASP_DISCONNECTED);
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);
            for (BluetoothGattService service: gatt.getServices()) {
                Log.v(TAG, "Gatt Service: " + service.getUuid().toString());
                for (BluetoothGattCharacteristic characteristic: service.getCharacteristics()) {
                    Log.v(TAG, "Gatt Characteristic: " + characteristic.getUuid().toString());
                }
            }
            BluetoothGattService customGattService = gatt.getService(getCustomServiceUUID());
            if (customGattService == null) {
                Log.e(TAG, "Required custom GATT service not found!");
                stateChange(YASP_NO_SERIAL);
            } else {
                customGattCharacteristic = customGattService.getCharacteristic(getCustomCharacteristicUUID());
                if (customGattCharacteristic == null) {
                    Log.e(TAG, "Required custom GATT characteristic not found!");
                    stateChange(YASP_NO_SERIAL);
                } else {
                    gatt.setCharacteristicNotification(customGattCharacteristic, true);
                    BluetoothGattDescriptor customGattDescriptor = customGattCharacteristic.getDescriptor(UUID_CC_CONFIG);
                    if (customGattDescriptor == null) {
                        Log.e(TAG, "Required custom GATT client configuration descriptor not found!");
                        stateChange(YASP_NO_SERIAL);
                    } else {
                        if (!Arrays.equals(customGattDescriptor.getValue(), BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE)) {
                            customGattDescriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                            gatt.writeDescriptor(customGattDescriptor);
                        } else {
                            stateChange(YASP_CONNECTED);
                        }
                    }
                }
            }
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicRead(gatt, characteristic, status);
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicWrite(gatt, characteristic, status);
            txInProgress = false;
            resumeTransmit();
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicChanged(gatt, characteristic);
            if (characteristic.getUuid().equals(getCustomCharacteristicUUID())) {
                serialReceive(characteristic.getValue());
            }
        }

        @Override
        public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            super.onDescriptorRead(gatt, descriptor, status);
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            super.onDescriptorWrite(gatt, descriptor, status);
            if (descriptor != null) {
                if (descriptor.getUuid() != null) {
                    if (descriptor.getCharacteristic().getUuid().equals(getCustomCharacteristicUUID()) &&
                            descriptor.getUuid().equals(UUID_CC_CONFIG) &&
                            Arrays.equals(descriptor.getValue(), BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE)) {
                        Log.i(TAG, "GATT characteristic notification enabled for " + bluetoothDevice.getName() + "...");
                        stateChange(YASP_CONNECTED);
                    }
                }
            }
        }

        @Override
        public void onReliableWriteCompleted(BluetoothGatt gatt, int status) {
            super.onReliableWriteCompleted(gatt, status);
        }

        @Override
        public void onReadRemoteRssi(BluetoothGatt gatt, int rssi, int status) {
            super.onReadRemoteRssi(gatt, rssi, status);
        }

        @Override
        public void onMtuChanged(BluetoothGatt gatt, int mtu, int status) {
            super.onMtuChanged(gatt, mtu, status);
        }
    }
}
