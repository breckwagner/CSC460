
#ifndef SHARED_H_
#define SHARED_H_

#define ROOMBA_RPC 0x01
#define REMOTE_PROCEDURE_CALL 0x01

#define RPC_ROOMBA_RPC 0x02
#define RPC_FIRE 0x03


#define LOW_BYTE(v)   ((unsigned char) (v))
#define HIGH_BYTE(v)  ((unsigned char) (((unsigned int) (v)) >> 8))


#endif /* ROOMBA_H_ */
