package com.mjbrown.yaspgateway;

import java.util.ArrayList;
import java.util.List;

public class UtilityFunctions {
    public final static char[] hexArray = "0123456789ABCDEF".toCharArray();

    public static byte[] serialize(List<Byte> obj) {
        byte[] data = new byte[obj.size()];
        for (int i = 0; i < obj.size(); i++) { data[i] = obj.get(i); }
        return data;
    }

    public static List<Byte> deserialize(byte[] bytes) {
        List<Byte> data = new ArrayList<>();
        for (byte aByte : bytes) { data.add(aByte); }
        return data;
    }

    public static String byteListToHex(List<Byte> bytes) {
        char[] hexChars = new char[bytes.size() * 2];
        for ( int j = 0; j < bytes.size(); j++ ) {
            int v = bytes.get(j) & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);
    }
}
