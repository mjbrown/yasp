package com.mjbrown.yaspgateway;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.support.v4.app.Fragment;
import android.util.Log;

public class YaspFragment extends Fragment {
    private final String TAG = "YaspFragment";
    public interface YaspFragmentListener {
        void changeTab(String fragmentTag);
    }

    YaspFragmentListener yaspFragmentListener;

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        try {
            yaspFragmentListener = (YaspFragmentListener) getActivity();
        }catch (ClassCastException e) {
            Log.e(TAG, "Super class must implement YaspFragmentListener");
        }
    }

    protected ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder iBinder) {
            YaspService.YaspBinder controlBinder = (YaspService.YaspBinder) iBinder;
            serviceAvailable(controlBinder.getYaspService());
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            serviceUnavailable();
        }
    };

    YaspService yaspService = null;
    protected void serviceAvailable(YaspService yaspService) {
        this.yaspService = yaspService;
    }

    protected void serviceUnavailable() {
        yaspService = null;
    }

    @Override
    public void onStart() {
        super.onStart();
        Context context = this.getContext();
        if (context != null) {
            Intent intent = new Intent(context, YaspService.class);
            context.bindService(intent, mConnection, Context.BIND_ABOVE_CLIENT);
        }
    }

    @Override
    public void onStop() {
        super.onStop();
        Context context = this.getContext();
        if (context != null) {
            context.unbindService(mConnection);
        }
    }
}
