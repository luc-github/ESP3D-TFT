## ESP3D [ESP701] format

Query and Control ESP700 stream
`[ESP701]action=<PAUSE/RESUME/ABORT> json=<no> pwd=<admin/user password>`

If no parameter the status is given

### Status
`[ESP701]`
response according to status:

* No stream   
   1 - plain text and no error
      `no stream`
   2 - plain text and error  
      `no stream, last error <error code>`  
   3 - json and no error
```json
   {"cmd":"701","status":"ok","data":{"status":"no stream"}}
```
   4 - json and error 
```json
   {"cmd":"701","status":"ok","data":{"status":"no stream", "code":"1"}}
```
where error code is:  

|code| Meaning
|- |-
|1 | ESP3D_ERROR_TIME_OUT
|2 | ESP3D_ERROR_CANNOT_SEND_DATA
|3 | ESP3D_ERROR_LINE_NUMBER
|4 | ESP3D_ERROR_ACK_NUMBER
|5 | ESP3D_ERROR_MEMORY_PROBLEM
|6 | ESP3D_ERROR_RESEND
|7 | ESP3D_ERROR_NUMBER_MISMATCH
|8 | ESP3D_ERROR_LINE_IGNORED
|9 | ESP3D_ERROR_FILE_SYSTEM
|10 | ESP3D_ERROR_CHECKSUM 
|11 | ESP3D_ERROR_UNKNOW
|12 |ESP3D_ERROR_FILE_NOT_FOUND
|13 |ESP3D_ERROR_STREAM_ABORTED 

* Pause
1 - plain text
    `pause`
2 - json
```json
{"cmd":"701","status":"ok","data":{"status":"pause", "total":"112345", "progress":"25","type":"2","name":"/myscript.gco"}}
```
if script is line of commands the name is not provide
```json
{"cmd":"701","status":"ok","data":{"status":"pause", "total":"112345", "progress":"25","type":"1"}}
```
* Processing
1 - plain text
    `processing`
2 - json
```json
{"cmd":"701","status":"ok","data":{"status":"processing", "total":"112345", "progress":"25","type":"2","name":"/myscript.gco"}}
```
if script is line of commands the name is not provide
```json
{"cmd":"701","status":"ok","data":{"status":"processing", "total":"112345", "progress":"25","type":"1"}}
```

where type is:
|code| Meaning | example
|- |- |-
|1 | ESP3D_TYPE_SCRIPT_STREAM, a line of command separated by `;` | `G28;G1 X0 Y10`
|2 | ESP3D_TYPE_FS_STREAM, a file on flash | `/myscript.gco`
| 3| ESP3D_TYPE_SD_STREAM, a file on SD card | `/sd/script.gco`