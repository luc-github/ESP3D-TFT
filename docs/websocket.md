# Websocket

there are 2

-   terminal websocket
    used to stream data to webUIand exchange internal data

-   data websocket
    used to exchange data

## Terminal websocket

use subprotocol `webui-v3`

### <u>text mode</u>

Reserved
messages between webui / ESP
Format: `<label>:<message>`

-   from ESP to WebUI

    -   `currentID:<id>`
        Sent when client is connecting, it is the last ID used and become the active ID

    -   `activeID:<id>`
        Broadcast current active ID, when new client is connecting, client without this is <id> should close, ESP WS Server close all open WS connections but this one also

    -   `PING:<time left>:<time out>`
        It is a response to PING from client to inform the time left if no activity (see below)

    -   `ERROR:<code>:<message>`
        If an error raise when doing upload, it informs client it must stop uploading because sometimes the http answer is not possible,
        or cannot cancel the upload, this is a workaround as there is no API in current webserver to cancel active upload

    -   `NOTIFICATION:<message>`
        Forward the message sent by [ESP600] to webUI toast system

    -   `SENSOR: <value>[<unit>] <value2>[<unit2>] ...`
        The sensor connected to ESP like DHT22

-   from WebUI to ESP
    -   `PING:<current cookiesessionID / none >` if any, or "none" if none

### <u>binary mode</u>

Reserved

-   from ESP to WebUI
    stream data from ESP to WebUI

-   from WEBUI to ESP  
    [-> File transfert from WebUI to ESP : not implemented yet]

## Data websocket

use sub protocol `arduino`

### <u>text mode</u>

This mode is used to transfert all GCODE commands and their answers from printer/cnc

### <u>binary mode</u>

This mode is used to transfert files to / from esp board

it use frame of 1024 bytes, can be increased after test

the frame format is :
 2 bytes: for frame type
 2 bytes: for frame size to check some integrity, currently as already part of frame no checksume is used
 x bytes : extra data according frame type

 ## Frame types

 ### Query status frame  
 type: client -> esp
  Status Request
  | `S` | `R` | 0 | 0 |   
  | - | - | - | - |

with hexadecimal values: 

  |0x53 | 0x52 | 0 | 0 |   
  | - | - | - | - |
 

 Response frame use inverted header:
Response Status
 | `R` | `S` | 0 | 1 | `A` |
 | - | - | - | - | - |

with hexadecimal values: 

 | 0x52 | 0x53 | 0 | 1 | 0x41 |
 | - | - | - | - | - |

the first byte of answer is the state
|Code | Hexa | Meaning|
|-|-|-|
|`B` | 0x42| busy|
|`O` | 0x4F|idle/ok|
|`E` | 0x45|error|
|`A` | 0x41|abort|
|`D` | 0x44|download ongoing
|`U` | 0x55|upload ongoing

extra data may be added :

#### For Error:
error code and string, 
1 byte : error code: 0->255
1 byte : string size 0->255
XX bytes for the string
|`R`|`S`|x|x|`C`|4|X|..|..|
|-|-|-|-|-|-|-|-|-|


#### For Upload:
Upload informations: 
1 byte : path size 0->255
XX bytes : the path string
1 byte : the filename size 0->255
xx bytes : filename string
4 bytes : total file size
4 bytes : currently processed bytes
4 bytes : last packet id processed
|`R`|`S`|x|x|`U`|X|..|..|X|..|..|S1|S1|S1|S1|S2|S2|S2|S2|
|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|


#### For Download:
Download informations:
1 byte : path size 0->255
XX bytes : the path string
1 byte : the filename size 0->255
xx bytes : filename string
4 bytes : total file size
4 bytes : currently processed bytes
4 bytes : last packet id processed
|`R`|`S`|x|x|`D`|X|..|..|X|..|..|S1|S1|S1|S1|S2|S2|S2|S2|
|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|


### Start upload frame
header is : 
| `S` | `U` | 0 | 0 |   
| - | - | - | - |

the content is: 
1 byte : path size 0->255
XX bytes : the path string
1 byte : the filename size 0->255
xx bytes : filename string
4 bytes : total file size 

|`S`|`U`|x|x|`D`|X|..|..|X|..|..|S1|S1|S1|S1|
|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|

if answer is :

|`U`|`S`|0|1|`O`|
|-|-|-|-|-|

it means transfert can start


### Transfert upload frame
header is :
| `U` | `P` | x | x | ID | ID | ID | ID |..|..|  
| - | - | - | - | - | - | - | - | - | - |

4 bytes is packet id
XXXX bytes is data

if packet is received the answer is 
|`P`|`U`|0|5|`O`| ID | ID | ID | ID 
|-|-|-|-|-|-|-|-|-|


### Start dowload frame
header is :
| `S` | `D` | 0 | 0 |   
| - | - | - | - |

the content is: 
1 byte : path size 0->255
XX bytes : the path string
1 byte : the filename size 0->255
xx bytes : filename string
4 bytes : total file size 

|`S`|`D`|x|x|`D`|X|..|..|X|..|..|S1|S1|S1|S1|
|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|

if answer is :

|`D`|`S`|0|1|`O`|
|-|-|-|-|-|

it means transfert can start


### Transfert download frame
header is :
| `D` | `P` | x | x | ID | ID | ID | ID |..|..|  
| - | - | - | - | - | - | - | - | - | - |

4 bytes is packet id
XXXX bytes is data

if packet is received the answer is 
|`P`|`D`|0|5|`O`| ID | ID | ID | ID 
|-|-|-|-|-|-|-|-|-|


### Command frame
header is :
| `C` | `M` | 0 | 1 | X |
| - | - | - | - |-|

Commands:
|Code | Hexa | Meaning|
|-|-|-|
|`A` | 0x41| abort|

Abort command frame 

| `C` | `M` | 0 | 1 | `A` |
| - | - | - | - |-|
