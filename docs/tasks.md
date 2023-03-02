## Tasks

| Source                     | name                     | core            | size        | priority  (0~25)   | description             |
|-----------                 |-------------             |-----------      |-------------|-------------       |-------------            |
|mdnc.c                      | mdns                     | 0               | 4096        | 1                  | mdns component          |
|ssdp.c                      | ssdp_running_task        | tskNO_AFFINITY  | 4096        | tskIDLE_PRIORITY+5 | ssdp component          |
|esp3d-tft-network.cpp       | tftNetwork               | 0               | 4096        | 0                  | Network tasks           |
|esp3d-tft-stream.cpp        | tftStream                | 1               | 4096        | 0                  | Stream tasks            |
|esp3d-tft-ui.cpp            | tftUI                    | 1               | 4096        | 0                  | UI tasks                |
|esp3d_serial_client.cpp     | esp3d_serial_rx_task     | 1               | 4096        | 10                 | serial tasks            |
|esp3d_socket_server.cpp     | esp3d_socket_rx_task     | 0               | 4096        | 5                  | telnet tasks            |
|esp3d_usb_serial_client.cpp | esp3d_usb_serial_task    | 1               | 4096        | 10                 | serial usb tasks        |
|usb_serial.cpp              | usb_lib                  | 1               | 4096        | 10                 | usb tasks               |
|esp3d_gcode_host_service.cpp| esp3d_gcode_host_task    | 1               | 4096        | 5                  | stream tasks            |
|esp3d_http_service.cpp      | httpd                    | 0               | 1024*8      | tskIDLE_PRIORITY+5 | httpd tasks             |
