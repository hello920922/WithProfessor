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
    BTConnector btConnector;
    LinearLayout peripheralList;
    TextView logView;
    EditText writeView;

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

        btConnector = new BTConnector(this) {
            @Override
            protected void discoveryAvailableDevice(final BluetoothDevice bluetoothDevice) {
                Button button = new Button(this.getContext());
                button.setText(bluetoothDevice.getName() + "\n" + bluetoothDevice.getAddress());
                button.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        btConnector.connectDevice(bluetoothDevice);
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
            btConnector.startDiscovery();
        }
        else if (v.getId() == R.id.btn_write){
            btConnector.writeMessage(writeView.getText().toString().getBytes());
        }
    }
}