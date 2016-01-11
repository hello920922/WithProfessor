package cau.project.beaconscanner;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.Toast;

import java.util.List;

/**
 * Created by Mingyu Park on 2016-01-10.
 */
public abstract class BTConnector extends Handler{
    // Define constant to process handler message
    public static final int BLUETOOTH_READ_MESSAGE = 0;
    public static final int BLUETOOTH_WRITE_MESSAGE = 1;
    public static final int BLUETOOTH_DISCOVERY_MESSAGE = 2;
    public static final int BLUETOOTH_CONNECT_MESSAGE = 3;
    public static final int BLUETOOTH_NOT_SUPPORT_MESSAGE = 4;

    public static final int WRITE_TYPE_DEFAULT = BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT;
    public static final int WRITE_TYPE_NO_RESPONSE = BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE;
    public static final int WRITE_TYPE_SIGNED = BluetoothGattCharacteristic.WRITE_TYPE_SIGNED;

    // Define constant to process intent
    public static final int REQUEST_ENABLE_BT = 0;

    // Define constant to log message
    private final String LOGTAG = "BTConnector";

    // Define variables to processing
    private Context context;
    private BroadcastReceiver mBroadcastReceiver;

    // Define variables to BLE
    private BluetoothAdapter mBluetoothAdapter;

    // variables to process callback function
    private BluetoothGattCallback mBluetoothGattCallback;
    private BluetoothGatt mDefaultBluetoothGatt;
    private BluetoothGattCharacteristic mDefaultBluetoothGattCharacteristic;

    // variable to send or receive
    private byte[] data;

    // Initialize variables to processing
    public BTConnector(Context context) {
        this.context = context;

        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        mBroadcastReceiver = makeBroadcastReceiver();
        mBluetoothGattCallback = new BluetoothGattCallback() {
            @Override
            public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    Log.d(LOGTAG, "Connected");
                    gatt.discoverServices();
                }
                else if (newState == BluetoothProfile.STATE_DISCONNECTED){
                    Log.d(LOGTAG, "Disconnected");
                }
                else if (newState == BluetoothProfile.STATE_CONNECTING){
                    Log.d(LOGTAG, "Connecting...");
                }
                else if (newState == BluetoothProfile.STATE_DISCONNECTING){
                    Log.d(LOGTAG, "Disconnecting...");
                }
                else if (newState == BluetoothGatt.GATT_FAILURE){
                    Log.d(LOGTAG, "Failure");
                }
            }

            @Override
            public void onServicesDiscovered(BluetoothGatt gatt, int status) {
                            List<BluetoothGattService> gattServices = gatt.getServices();
                            for (BluetoothGattService gattService : gattServices) {
                                List<BluetoothGattCharacteristic> gattCharacteristics = gattService.getCharacteristics();
                                for (BluetoothGattCharacteristic gattCharacteristic : gattCharacteristics) {
                                    if (isWritableCharacteristic(gattCharacteristic) && isReadableCharacteristic(gattCharacteristic) && isNotificationCharacteristic(gattCharacteristic)) {
                                        gatt.setCharacteristicNotification(gattCharacteristic, true);
                                        mDefaultBluetoothGatt = gatt;
                                        mDefaultBluetoothGattCharacteristic = gattCharacteristic;
                                        Log.d(LOGTAG, "Available service uuid : " + gattService.getUuid());
                                        Log.d(LOGTAG, "Available characteristic uuid : " + gattCharacteristic.getUuid());
                                        return;
                        }
                    }
                }
            }

            @Override
            public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
                if (status == BluetoothGatt.GATT_SUCCESS)
                    Log.d(LOGTAG, "Success to read");
                else if(status == BluetoothGatt.GATT_FAILURE)
                    Log.d(LOGTAG, "Fail to read");
            }

            @Override
            public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
                if (status == BluetoothGatt.GATT_SUCCESS)
                    Log.d(LOGTAG, "Success to write");
                else if(status == BluetoothGatt.GATT_FAILURE)
                    Log.d(LOGTAG, "Fail to write");
            }

            @Override
            public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
                byte[] readData = characteristic.getValue();
                if (readData == null || readData.length <= 0) {
                    return;
                }
                data = readData;
                Log.d(LOGTAG, "Read message : " + new String(data));
                sendEmptyMessage(BLUETOOTH_READ_MESSAGE);
            }
        };
    }

    @Override
    public void handleMessage(Message msg){
        super.handleMessage(msg);
        switch (msg.what){
            case BLUETOOTH_READ_MESSAGE :
                readHandler(data);
                break;
            case BLUETOOTH_WRITE_MESSAGE :
                writeHandler((byte[])msg.obj, msg.arg1);
                break;
            case BLUETOOTH_DISCOVERY_MESSAGE :
                discoveryAvailableDevice((BluetoothDevice) msg.obj);
                break;
            case BLUETOOTH_CONNECT_MESSAGE :
                connectHandler((BluetoothDevice) msg.obj);
                break;
            case BLUETOOTH_NOT_SUPPORT_MESSAGE:
                notSupportBluetooth();
                break;
        }
    }

    // API users should implement this method
    protected abstract void discoveryAvailableDevice(BluetoothDevice bluetoothDevice);
    public abstract void readHandler(byte[] data);

    // After check whether the device support bluetooth and bluetooth state, start discovery
    public void startDiscovery() {
        if (mBluetoothAdapter == null) {
            Log.e(LOGTAG, "This device does not support bluetooth");
            this.sendEmptyMessage(BLUETOOTH_NOT_SUPPORT_MESSAGE);
            return;
        }
        if (!mBluetoothAdapter.isEnabled()) {
            Intent btEnIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            if(context instanceof AppCompatActivity)
                ((AppCompatActivity)context).startActivityForResult(btEnIntent, REQUEST_ENABLE_BT);
            else
                context.startActivity(btEnIntent);
        }
        if (!mBluetoothAdapter.isDiscovering())
            mBluetoothAdapter.startDiscovery();
    }

    // Just only cancel discovery
    public void stopDiscovery(){
        if(mBluetoothAdapter != null && mBluetoothAdapter.isDiscovering())
            mBluetoothAdapter.cancelDiscovery();
    }

    // Cancel discovery or disconnect
    public void disconnect() {
        if (mBluetoothAdapter != null && mBluetoothAdapter.isDiscovering())
            mBluetoothAdapter.cancelDiscovery();
        if (mDefaultBluetoothGattCharacteristic != null)
            mDefaultBluetoothGattCharacteristic = null;
        if (mDefaultBluetoothGatt != null)
            mDefaultBluetoothGatt.disconnect();
    }

    // If device does not support bluetooth, this method is executed
    protected void notSupportBluetooth(){
        Toast.makeText(context, "This device does not support bluetooth", Toast.LENGTH_SHORT).show();
    }

    // Make broadcast receiver and define behavior
    private BroadcastReceiver makeBroadcastReceiver() {
        BroadcastReceiver newBroadcastReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (action.equals(BluetoothAdapter.ACTION_DISCOVERY_STARTED))
                    discoveryStartAction();
                else if (action.equals(BluetoothDevice.ACTION_FOUND))
                    foundAction((BluetoothDevice) intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE));
                else if (action.equals(BluetoothAdapter.ACTION_DISCOVERY_FINISHED))
                    discoveryFinishAction();
            }
        };
        IntentFilter filter = new IntentFilter(BluetoothAdapter.ACTION_DISCOVERY_STARTED);
        context.registerReceiver(newBroadcastReceiver, filter);
        filter = new IntentFilter(BluetoothDevice.ACTION_FOUND);
        context.registerReceiver(newBroadcastReceiver, filter);
        filter = new IntentFilter(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        context.registerReceiver(newBroadcastReceiver, filter);

        return newBroadcastReceiver;
    }

    // Users can reimplement this method
    protected void discoveryStartAction() {
        Log.d(LOGTAG, "Discovery start");
    }

    private void foundAction(BluetoothDevice bluetoothDevice) {
        String deviceName = bluetoothDevice.getName();
        String deviceAddr = bluetoothDevice.getAddress();
        Log.d(LOGTAG, "Found device : " + deviceName + "\t(" + deviceAddr + ")");

        Message msg = Message.obtain();
        msg.what = BLUETOOTH_DISCOVERY_MESSAGE;
        msg.obj = bluetoothDevice;
        this.sendMessage(msg);
    }

    // Users can reimplement this method
    protected void discoveryFinishAction() {
        Log.d(LOGTAG, "Discovery finish");
        if (mBluetoothAdapter.isDiscovering())
            mBluetoothAdapter.cancelDiscovery();
    }

    public void connectDevice(String deviceAddress) {
        connectDevice(mBluetoothAdapter.getRemoteDevice(deviceAddress));
    }

    public void connectDevice(BluetoothDevice bluetoothDevice) {
        if(mBluetoothAdapter.isDiscovering())
            mBluetoothAdapter.cancelDiscovery();

        Message msg = Message.obtain();
        msg.what = BLUETOOTH_CONNECT_MESSAGE;
        msg.obj = bluetoothDevice;
        sendMessage(msg);
    }

    // Connect with BLE device using handler
    private void connectHandler(BluetoothDevice bluetoothDevice){
        if(mBluetoothAdapter != null && mBluetoothAdapter.isDiscovering())
            mBluetoothAdapter.cancelDiscovery();
        bluetoothDevice.connectGatt(context, false, mBluetoothGattCallback);
    }

    // Write to BLE device using handler
    private void writeHandler(byte[] message, int writeType){
        mDefaultBluetoothGattCharacteristic.setValue(message);
        mDefaultBluetoothGattCharacteristic.setWriteType(writeType);
        mDefaultBluetoothGatt.writeCharacteristic(mDefaultBluetoothGattCharacteristic);
        Log.d(LOGTAG, "Write message : " + new String(message));
    }

    // Default write method
    public void writeMessage(byte[] message){
        Message msg = Message.obtain();
        msg.what = BLUETOOTH_WRITE_MESSAGE;
        msg.obj = message;
        msg.arg1 = WRITE_TYPE_DEFAULT;

        sendMessage(msg);
    }

    // Write method with write type
    public void writeMessage(byte[] message, int writeType){
        Message msg = Message.obtain();
        msg.what = BLUETOOTH_WRITE_MESSAGE;
        msg.obj = message;
        msg.arg1 = writeType;

        sendMessage(msg);
    }

    // If Activity is changed, users can change handle activity using this method
    public void changeContext(Context context){
        this.context = context;

        IntentFilter filter = new IntentFilter(BluetoothAdapter.ACTION_DISCOVERY_STARTED);
        context.registerReceiver(mBroadcastReceiver, filter);
        filter = new IntentFilter(BluetoothDevice.ACTION_FOUND);
        context.registerReceiver(mBroadcastReceiver, filter);
        filter = new IntentFilter(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        context.registerReceiver(mBroadcastReceiver, filter);
    }

    // Return this object's target context
    public Context getContext(){return context;}

    // Check the characteristic's writable
    private boolean isWritableCharacteristic(BluetoothGattCharacteristic chr) {
        if(chr == null) return false;

        final int charaProp = chr.getProperties();
        if (((charaProp & BluetoothGattCharacteristic.PROPERTY_WRITE) |
                (charaProp & BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE)) > 0) {
            return true;
        } else {
            return false;
        }
    }

    // Check the characteristic's readable
    private boolean isReadableCharacteristic(BluetoothGattCharacteristic chr) {
        if(chr == null) return false;

        final int charaProp = chr.getProperties();
        if((charaProp & BluetoothGattCharacteristic.PROPERTY_READ) > 0) {
            return true;
        } else {
            return false;
        }
    }

    // Check the characteristic's notification
    private boolean isNotificationCharacteristic(BluetoothGattCharacteristic chr) {
        if(chr == null) return false;

        final int charaProp = chr.getProperties();
        if((charaProp & BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
            return true;
        } else {
            return false;
        }
    }
}
