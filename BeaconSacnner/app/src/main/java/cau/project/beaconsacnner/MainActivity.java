package cau.project.beaconsacnner;

import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.text.format.DateFormat;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.wizturn.sdk.central.Central;
import com.wizturn.sdk.central.CentralManager;
import com.wizturn.sdk.peripheral.OnConnectListener;
import com.wizturn.sdk.peripheral.Peripheral;
import com.wizturn.sdk.peripheral.PeripheralScanListener;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/**
 * Created by Mingyu Park on 2015-12-24.
 */

public class MainActivity extends AppCompatActivity {
    private final int REQUEST_ENABLE_BT = 1000;

    private boolean state;
    private HashMap<String, Peripheral> peripheralHashMap;
    private LinearLayout peripheralList;
    private LinearLayout pairingText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //initialize and implement listener event
        CentralManager.getInstance().init(getApplicationContext());
        CentralManager.getInstance().setPeripheralScanListener(new PeripheralScanListener() {
            @Override
            public void onPeripheralScan(Central central, final Peripheral peripheral) {
                peripheralHashMap.put(peripheral.getBDAddress(), peripheral);
                new MakeButtonTask().execute();
            }
        });

        state = false;
        peripheralHashMap = new HashMap<>();
        peripheralList = (LinearLayout)findViewById(R.id.peripheralList);
        pairingText = (LinearLayout)findViewById(R.id.pairing_text);
    }

    class MakeButtonTask extends AsyncTask<Void, Void, Void>{
        @Override
        protected Void doInBackground(Void... params) {
            publishProgress();
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... value){
            Iterator<Map.Entry<String, Peripheral>> iterator = peripheralHashMap.entrySet().iterator();
            if(peripheralList.getChildCount() > 0)
                peripheralList.removeViews(0, peripheralList.getChildCount());
            while(iterator.hasNext())
                addButton(iterator.next().getValue());
        }
    }
    class PrintLogTask extends AsyncTask<Void, Void, Void>{
        private Peripheral peripheral;
        private boolean isConnected;

        public PrintLogTask(Peripheral peripheral, boolean isConnected){
            this.peripheral = peripheral;
            this.isConnected = isConnected;
        }

        @Override
        protected Void doInBackground(Void... params) {
            publishProgress();
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... value){
            while(CentralManager.getInstance().isConnected()) {
                printLog(peripheral, isConnected);
                try{Thread.sleep(1000);}
                catch(InterruptedException interruptedEx){}
            }
        }
    }
    public void addButton(final Peripheral peripheral){
        Button newBtn = new Button(this);

        StringBuilder stringBuilder = new StringBuilder();

        stringBuilder.append("UUID : ");
        stringBuilder.append(peripheral.getProximityUUID());
        stringBuilder.append("\n");

        stringBuilder.append("BD Address : ");
        stringBuilder.append(peripheral.getBDAddress());
        stringBuilder.append("\t");

        stringBuilder.append("BD NAME : ");
        stringBuilder.append(peripheral.getBDName());
        stringBuilder.append("\n");

        stringBuilder.append("Major : ");
        stringBuilder.append(peripheral.getMajor());
        stringBuilder.append("\t");

        stringBuilder.append("Minor : ");
        stringBuilder.append(peripheral.getMinor());
        stringBuilder.append("\t");

        stringBuilder.append("RSSI : ");
        stringBuilder.append(peripheral.getRssi());

        newBtn.setText(stringBuilder.toString());
        newBtn.setGravity(Gravity.LEFT);
        newBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (CentralManager.getInstance().isConnected()) {
                    CentralManager.getInstance().disconnectWithDelay();
                }
                CentralManager.getInstance().connectWithDelay(peripheral, new OnConnectListener() {
                    @Override
                    public void onConnected(Peripheral peripheral, boolean b) {
                        PrintLogTask printLogTask = new PrintLogTask(peripheral, true);
                        printLogTask.execute();
                    }

                    @Override
                    public void onError(Peripheral peripheral) {
                        PrintLogTask printLogTask = new PrintLogTask(peripheral, false);
                        printLogTask.execute();
                    }

                    @Override
                    public void onDisconnected(Peripheral peripheral) {
                    }
                });
            }
        });
        peripheralList.addView(newBtn);
    }
    public void printLog(final Peripheral peripheral, boolean isConnected){
        if(!isConnected){
            Toast.makeText(this, "Connect Faliure", Toast.LENGTH_SHORT).show();
            return;
        }
        StringBuilder stringBuilder = new StringBuilder();

        stringBuilder.append(DateFormat.format("yy:mm:dd hh:nn:ss", System.currentTimeMillis()));
        stringBuilder.append("\n");

        stringBuilder.append(peripheral.getBDAddress());
        stringBuilder.append("\t");
        stringBuilder.append(peripheral.getMajor());

        TextView textView = new TextView(this);
        textView.setGravity(Gravity.LEFT);
        textView.setText(stringBuilder.toString());

        pairingText.addView(textView);
    }
    public void scanClick(View view) {
        int id = view.getId();
        if(id == R.id.btnScan){
            if(state)
                Stop();
            else
                Scan();
        }
    }
    public void Scan(){
        state = true;

        Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
        startActivity(enableBtIntent);

        if(CentralManager.getInstance().isBluetoothEnabled()) {
            CentralManager.getInstance().startScanning();
        }
    }
    public void Stop(){
        state = false;
        CentralManager.getInstance().stopScanning();
    }
}
