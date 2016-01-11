package cau.project.beaconscanner;

import android.bluetooth.BluetoothDevice;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

/**
 * Created by Mingyu Park on 2016-01-11.
 */
public class MainActivity extends AppCompatActivity implements View.OnClickListener{
    BLEConnector bleConnector;
    LinearLayout peripheralList;
    TextView logView;
    EditText writeView;
    int i=0;

    int state;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

        ((Button)findViewById(R.id.btn_scan)).setOnClickListener(this);
        ((Button)findViewById(R.id.btn_write)).setOnClickListener(this);
        peripheralList = (LinearLayout)findViewById(R.id.peripheral_list);
        logView = (TextView)findViewById(R.id.txt_log);
        writeView = (EditText)findViewById(R.id.write_message);

        bleConnector = new BLEConnector(this) {
            @Override
            protected void discoveryAvailableDevice(final BluetoothDevice bluetoothDevice, int rssi, byte[] scanRecord) {
                Button button = new Button(this.getContext());
                button.setText((i++) + bluetoothDevice.getName() + "\n" + bluetoothDevice.getAddress());
                button.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        bleConnector.connectDevice(bluetoothDevice);
                    }
                });
                peripheralList.addView(button);
            }

            @Override
            public void readHandler(byte[] data) {
                logView.append(new String(data) + "\n");
            }
        };
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.btn_scan) {
            bleConnector.startDiscovery();
        }
        else if (v.getId() == R.id.btn_write){
            bleConnector.writeMessage(writeView.getText().toString().getBytes());
        }
    }
}