## ESP3D [ESP700] format

Read / Stream  / Process script file
[ESP700]<script> json=<no> pwd=<admin/user password>

If no parameter the error is raised
### Script parameter
script can be 
#### A command line
commands are separated by `;`
e.g: `G28;G1 X10 Y20`

#### A file on flash
Start name with `/`
e.g: `/mysscript.gco`

#### A file on sd card
Start name with `/sd/`
e.g: `/sd/mysscript.gco`


### Response
* plain text
    `0k` or `Missing parameter` if no parameter provided
* json
```json
{"cmd":"700","status":"ok"}
```
or 
```json
If no parameter provided
{"cmd":"700","status":"error","data":"Missing parameter"} 
```
