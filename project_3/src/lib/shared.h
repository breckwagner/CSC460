
#ifndef SHARED_H_
#define SHARED_H_

#define S_SENSATIVITY 50


#define ROOMBA_RPC 0x01

// Sent to tell remotecontroller to excute RPC
#define REMOTE_PROCEDURE_CALL 0x01

// Sent after REMOTE_PROCEDURE_CALL to specify the rpc is a roomba cmd
#define RPC_ROOMBA_RPC 0x02
#define RPC_FIRE 0x03
#define RPC_MODE_SELECT 0x04

#define RPC_MANUAL_MODE 0x00
#define RPC_AUTONOMOUS_MODE 0x01


#define LOW_BYTE(v)   ((unsigned char) (v))
#define HIGH_BYTE(v)  ((unsigned char) (((unsigned int) (v)) >> 8))


#endif /* ROOMBA_H_ */
