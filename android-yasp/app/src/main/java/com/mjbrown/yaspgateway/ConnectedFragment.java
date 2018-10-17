package com.mjbrown.yaspgateway;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

public class ConnectedFragment extends YaspFragment {
    public static String TAG = "ConnectedFragment";
    ListView listView;
    ConnectedListAdapter connectedListAdapter;

    IntentFilter intentFilter = new IntentFilter(YaspService.CONNECTION_EVENT);
    BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            connectedListAdapter.notifyDataSetChanged();
        }
    };

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.fragment_listview, container, false);
        listView = rootView.findViewById(R.id.listview_container);
        connectedListAdapter = new ConnectedListAdapter();
        listView.setAdapter(connectedListAdapter);
        Activity activity = getActivity();
        if (activity != null) { activity.registerReceiver(broadcastReceiver, intentFilter); }
        return rootView;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Activity activity = getActivity();
        if (activity != null) { activity.unregisterReceiver(broadcastReceiver); }
    }

    private class ConnectedListAdapter extends BaseAdapter {
        private int ONLY_ITEM_VIEWTYPE = 0;

        @Override
        public int getCount() {
            if (yaspService == null) {
                return 0;
            } else {
                return yaspService.getYaspDevices().size();
            }
        }

        @Override
        public YaspDevice getItem(int position) {
            if (yaspService == null) {
                Log.e(TAG, "getItem called when yaspService is unavailable.");
                return null;
            } else {
                return yaspService.getYaspDevices().get(position);
            }
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public int getViewTypeCount() { return 1; }

        @Override
        public int getItemViewType(int postion) {
            return ONLY_ITEM_VIEWTYPE;
        }

        @Override
        public View getView(final int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                LayoutInflater inflater = LayoutInflater.from(getContext());
                convertView = inflater.inflate(R.layout.listheader_connected, parent, false);
            }
            TextView textView_deviceName = convertView.findViewById(R.id.textview_scan_device_name);
            if (getItem(position).getName() != null) {
                textView_deviceName.setText(getItem(position).getName());
            } else {
                textView_deviceName.setText(R.string.not_available);
            }
            TextView textView_address = convertView.findViewById(R.id.textview_scan_mac_address);
            textView_address.setText(getItem(position).getAddress());
            Button button_disconnect = convertView.findViewById(R.id.button_disconnect);
            int connection_state = getItem(position).getConnectionState();
            if (connection_state == YaspDevice.YASP_CONNECTING) {
                button_disconnect.setText(R.string.connecting);
                button_disconnect.setEnabled(false);
            } else if ((connection_state == YaspDevice.YASP_CONNECTED) ||
                       (connection_state == YaspDevice.YASP_NO_SERIAL)) {
                button_disconnect.setText(R.string.disconnect);
                button_disconnect.setEnabled(true);
                button_disconnect.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        getItem(position).disconnect();
                    }
                });
            } else if (connection_state == YaspDevice.YASP_DISCONNECTING) {
                button_disconnect.setText(R.string.disconnecting);
                button_disconnect.setEnabled(false);
            } else if ((connection_state == YaspDevice.YASP_FOUND) ||
                       (connection_state == YaspDevice.YASP_DISCONNECTED)) {
                button_disconnect.setText(R.string.connect);
                button_disconnect.setEnabled(true);
                button_disconnect.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        getItem(position).connect(yaspService, getContext());
                    }
                });
            }
            return convertView;
        }
    }
}
