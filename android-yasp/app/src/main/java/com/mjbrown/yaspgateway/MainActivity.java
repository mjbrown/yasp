package com.mjbrown.yaspgateway;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.IBinder;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.design.widget.TabLayout;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.content.ContextCompat;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;

public class MainActivity extends AppCompatActivity {
    final static String TAG = "MainActivity";
    final static int REQUEST_ENABLE_BT_INTENT = 1;
    YaspService yaspService = null;

    public interface MainActivityListener {
        void changeTab(String tag);
    }

    private SectionsPagerAdapter sectionsPagerAdapter;
    private ViewPager viewPager;
    TabLayout tabLayout;

    protected ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            YaspService.YaspBinder yaspBinder = (YaspService.YaspBinder) service;
            yaspService = yaspBinder.getYaspService();
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            yaspService = null;
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        sectionsPagerAdapter = new SectionsPagerAdapter(getSupportFragmentManager());
        viewPager = findViewById(R.id.container);
        viewPager.setAdapter(sectionsPagerAdapter);

        tabLayout = findViewById(R.id.tabs);
        viewPager.addOnPageChangeListener(new TabLayout.TabLayoutOnPageChangeListener(tabLayout));
        viewPager.addOnPageChangeListener(new ViewPager.OnPageChangeListener() {
            @Override public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) { }

            @Override
            public void onPageSelected(int position) {
                if (yaspService != null) {
                    if (position == 0) {
                        yaspService.startScan();
                        Log.d(TAG, "onPageSelected() startScan()");
                    } else {
                        yaspService.stopScan();
                        Log.d(TAG, "onPageSelected() stopScan()");
                    }
                }
            }

            @Override public void onPageScrollStateChanged(int state) { }
        });
        tabLayout.addOnTabSelectedListener(new TabLayout.ViewPagerOnTabSelectedListener(viewPager));

        int permissionCheck = ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION);
        if (permissionCheck != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[] {Manifest.permission.ACCESS_FINE_LOCATION, Manifest.permission.ACCESS_COARSE_LOCATION}, 2);
        }
        permissionCheck = ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_ADMIN);
        if (permissionCheck != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[] {Manifest.permission.BLUETOOTH_ADMIN, Manifest.permission.BLUETOOTH}, 2);
        }

        if (!BluetoothAdapter.getDefaultAdapter().isEnabled()) {
            Intent enableBt = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBt, REQUEST_ENABLE_BT_INTENT);
        }
        Intent intent = new Intent(getApplicationContext(), YaspService.class);
        startService(intent);
        this.getApplicationContext().bindService(intent, serviceConnection, Context.BIND_AUTO_CREATE | Context.BIND_ABOVE_CLIENT);

        FloatingActionButton fab = findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (yaspService != null) {
                    yaspService.startScan();
                } else {
                    Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                            .setAction("Action", null).show();
                }
            }
        });
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        this.getApplicationContext().unbindService(serviceConnection);
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    private class SectionsPagerAdapter extends FragmentPagerAdapter {
        SectionsPagerAdapter(FragmentManager fm) { super(fm); }

        @Override
        public Fragment getItem(int position) {
            if (position == 0) {
                return new ScanFragment();
            } else if (position == 1) {
                return new ConnectedFragment();
            } else {
                return new SettingsFragment();
            }
        }

        @Override
        public int getCount() {
            return 3;
        }
    }
}
