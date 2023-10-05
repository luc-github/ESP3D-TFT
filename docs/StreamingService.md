# Streaming Service Description

## Clients
The Gcode Stream Client is the actual streaming service, it is the only one that send msg to output client. On another way the GcodeStreamClient is only one of the active clients which will get the msg from the output client.
The GcodeStreamClient use the GCodeParser to parse in/out commands and handle messages correctly.

## Components
1 - Streaming Types
There are several types of streaming services:  
    A - Unknown  
    The type is not yet defined, this type cannot be streamed until it is identified.

    B - Single Command
    This is for any normal GCODE or ESPXXX command coming from any client, this type cannot be interrupted once started, ESP700 / ESP701 do not support it. This type will be added to the script queue and processed FIFO. 

    C - Multiple Commands
    This is used by scripts stored in EEPROM (resume/pause/abort), this type cannot be interrupted, and all commands will be executed one after another until the end, ESP700 / ESP701 do not support it. This type will be added to the script queue and processed FIFO

    D - Flash File Stream
    This is the one that will be used to stream a file from flash, this type can be interrupted only between each command execution.
    This type will put in hold if any command is present in scripts queue and resumed when the scripts queue is empty.
    This type is used with the command [ESP700]stream=/fs/mystream.gco
    the file name must have the /fs/ prefix to be sure that the file is handled properly, the [ESP701] commands can handle it but only between commands commands execution, that the purpose of the stream= tag. This type will be added to the stream queue and processed FIFO.
   
    E - Flash File script
    This is the one that will be used to stream a file from flash, unlike previous type this type cannot be interrupted and will be excuted until the end. This one is used with the command [ESP700]/fs/myfile.gco
    The file name must have the /fs/ prefix to be sure that the file is handled properly. The [ESP701] command do not handle it. The file should only contain few commands and it is used by macros only
    This type will be added to the script queue and processed FIFO


    F - SD File Stream
    This is the one that will be used to stream a file from sd card, this type can be interrupted only between each command execution.
    This type will put in hold if any command is present in scripts queue and resumed when the scripts queue is empty.
    This type is used with the command [ESP700]stream=/sd/mystream.gco
    the file name must have the /sd/ prefix to be sure that the file is handled properly, the [ESP701] commands can handle it but only between commands commands execution, that the purpose of the stream= tag. This type will be added to the stream queue and processed FIFO.
   
    G - SD File script
    This is the one that will be used to stream a file from sd card, unlike previous type this type cannot be interrupted and will be excuted until the end. This one is used with the command [ESP700]/sd/myfile.gco
    The file name must have the /sd/ prefix to be sure that the file is handled properly. The [ESP701] command do not handle it. The file should only contain few commands and it is used by macros only
    This type will be added to the script queue and processed FIFO
    
    E- Invalid
    The type is not yet identified, this type cannot be streamed and will raise an error.

2 - Streaming Queues
There are 3 types of queues for streams
    A - Stream queue
    This queue contain sd streams and fs streams, this queue has lower processing priority than script queue, which mean when the stream reach some state it can be interrupted, put in hold, and resumed.
    The active stream of this queue can be monitored and controlled with the command [ESP701]
    B - Script queue
    This queue contains single commands, multiple commands, sd scripts and fs scripts. The queue has higher priority than script queue and will be processed before the stream queue. There is not control of this queue from user point of view.

    C - Emmergency Queue => TBC
    This queue contain single commands that will be executed as soon as possible.


3 - Streaming queue states
The stream states only applied streams queue and only to the active stream, not to scripts queue.
    A - idle 
    The streams queue is empty, but the scripts queue may not be empty
    B - processing
    It means at least one stream in streams queue is being processed, even in hold due to scripts queue not being empty.
    C - paused
    The current stream in stream queue is paused.
    D - error
    Currently not implemented because when a stream go to error it is removed from stream queue to process the next one in stream queue if any.

3 - Stream command states
The stream command states cover all states of a processed command. 
    A - Undefined
    The state is undefined and the command will go to error state.
    B - Start
    This is the state to start a command or resume it. Next state is normally Ready to read cursor.

    C - Ready to read cursor
    This is the state before really processing command, this is the state used to put a stream in hold to allow scripts queue or to respond to external change of state for stream like pause/abort. If not change state requested then the next state will be read cursor state
    D - Read Cursor
    This state read coming processing command from file/multiple commands line or single command.
    According to the command the next state will be send gcode command state or send esp command state.
    E - Send gcode command
    Send gcode command to the output client, the command may have been completed with line number and checksum if is requested.
    Depending of the command the next state will be wait for ack if an ack is requested or ready to read next command if no ack is requested.
    F - Send esp command
    The command has been identified as ESP command, so it will be handled internaly and response will be sent to the original client
    The next state will be ready to read next command.

    G - Wait for ack
    If the command has been identified as GCode command and this command will send an ack, this state it to wait for it, if no ack received before a specific timout the next state will be the error state, if the ack is received before timeout then the next state will be ready to read next command.
    H - Resend gcode command
    In case of GCode command that is completed with line number and checksum and if there a checksum error received, this state will be resend the current gcode command again, ater a specific amount of failure the command will go to the error state.
    I - Pause
    This state will make the current stream queue on hold and process add a specific script to scripts queue if defined in settings, the next state will be paused.
    J - Paused
    This state will be waiting for resume or abort request to complete the stream.
    K - Resume
    If the current stream is paused it will resume it by adding a specific script to scripts queue if .defined in settings, and changing the next state as ready to read next command
    L - Abort
    This state will abort the current stream by adding a specific script to scripts queue if .defined in settings and changing the next state as end for the next command.
    M - End
    This state will close the current stream and remove it from the corresponding queue.
    N - Error
    If there is an error during the stream processing the error state will be raised, this state will  notify the user about the error and the next state will be end state to remove failing stream.

## Operations
the streaming is a loop of different operations
1 - Check external notifications / requests
This will check if there is an external request for change stream status e.g: pause or resume or abort

2 - Check what is the current active stream
This will check if there is a need of switching between queues and so switch between active streams, emmergency commands queue > scripts queue > streams queue

3 - Check what is the current active stream state
This will check if there is a need of changing the current state of the current active stream and applied it

4 - Check the client rx queue and process it
This will check if there is response  / command in the rx queue, parse the response if necessary to search for a command ack, add any external command to the stream queue coming, etc... 