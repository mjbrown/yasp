package com.mjbrown.yaspgateway;

public class YaspMessage {
    public static int ERROR_EVENT = 0;
    public static int LOOPBACK_CMD = 1;
    public static int LOOPBACK_RSP = 2;

    private Integer code = null;
    private byte[] payload = null;

    YaspMessage(int code, byte[] payload) {

    }
}
