Drop the nrf5_SDK_15.2 distribution zip file here, and unzip the contents into nRF5_SDK.

Embedded studio 3.4 used to compile.

nrfutil pkg generate --hw-version 52 --sd-req 0xAE --application-version 4 --application Output/Debug/Exe/ble_app_buttonless_dfu_pca10056_s140.hex --key-file private.pem app_dfu_package.zip
