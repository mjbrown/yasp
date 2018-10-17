package com.mjbrown.yaspgateway;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

import java.util.Locale;

public class ScanFragment extends YaspFragment {
    public static String TAG = "ScanFragment";
    private final int REFRESH_RATE = 3000;
    private final int SCAN_START_DELAY = 1000;

    ListView listView;
    ScanListAdapter scanListAdapter;
    Handler refreshHandler = new Handler();
    Runnable refreshScanList = new Runnable() { @Override public void run() { updateScanResults(REFRESH_RATE); } };
    Runnable restartScan = new Runnable() { @Override public void run() { restartScan(SCAN_START_DELAY); } };

    IntentFilter intentFilter = new IntentFilter(YaspService.ACTION_SCAN_EVENT);
    BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            updateScanResults(REFRESH_RATE);
        }
    };

    private void updateScanResults(int nextRefresh) {
        refreshHandler.removeCallbacksAndMessages(refreshScanList);
        scanListAdapter.notifyDataSetChanged();
        refreshHandler.postDelayed(refreshScanList, nextRefresh);
    }

    private void restartScan(int nextRetry) {
        refreshHandler.removeCallbacksAndMessages(restartScan);
        if (yaspService != null) {
            yaspService.startScan();
        } else {
            refreshHandler.postDelayed(restartScan, nextRetry);
        }
    }

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.fragment_listview, container, false);
        listView = rootView.findViewById(R.id.listview_container);
        scanListAdapter = new ScanListAdapter();
        listView.setAdapter(scanListAdapter);
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                yaspService.connectDevice(yaspService.getScanResults().get(position));
            }
        });
        Activity activity = getActivity();
        if (activity != null) { activity.registerReceiver(broadcastReceiver, intentFilter); }
        return rootView;
    }

    @Override
    public void onResume() {
        super.onResume();
        restartScan(SCAN_START_DELAY);
        Log.d(TAG, "onResume()");
    }

    @Override
    public void onPause() {
        super.onPause();
        refreshHandler.removeCallbacksAndMessages(null);
        if (yaspService != null) {
            yaspService.stopScan();
        }
        Log.d(TAG, "onPause()");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Activity activity = getActivity();
        if (activity != null) { activity.unregisterReceiver(broadcastReceiver); }
    }

    private class ScanListAdapter extends BaseAdapter {
        private int ONLY_ITEM_VIEWTYPE = 0;

        @Override
        public int getCount() {
            if (yaspService == null) {
                return 0;
            } else {
                return yaspService.getScanResults().size();
            }
        }

        @Override
        public YaspScanResult getItem(int position) {
            if (yaspService == null) {
                Log.e(TAG, "getItem called when yaspService is unavailable.");
                return null;
            } else {
                return yaspService.getScanResults().get(position);
            }
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public int getViewTypeCount() { return 1; }

        @Override
        public int getItemViewType(int position) {
            return ONLY_ITEM_VIEWTYPE;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                LayoutInflater inflater = LayoutInflater.from(getContext());
                convertView = inflater.inflate(R.layout.listheader_scan, parent, false);
            }
            YaspScanResult scanResult = getItem(position);
            TextView textView_deviceName = convertView.findViewById(R.id.textview_scan_device_name);
            String deviceName = scanResult.getBluetoothDevice().getName();
            if (deviceName == null) {
                textView_deviceName.setText(R.string.not_available);
            } else {
                textView_deviceName.setText(deviceName);
            }
            TextView textView_macAddress = convertView.findViewById(R.id.textview_scan_mac_address);
            textView_macAddress.setText(scanResult.getBluetoothDevice().getAddress());
            TextView textView_rssi = convertView.findViewById(R.id.textview_scan_rssi);
            textView_rssi.setText(String.format(Locale.getDefault(), "%ddB", scanResult.getScanRssi()));
            return convertView;
        }
    }
}
