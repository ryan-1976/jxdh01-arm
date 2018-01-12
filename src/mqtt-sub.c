#include "stdio.h"
//#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"
#include "circlebuff.h"
#include <unistd.h>
#include <stdlib.h>
//#define ADDRESS     "tcp://localhost:1883"
#define ADDRESS     "tcp://192.168.7.240:1883"
#define CLIENTID    "11111111111111sub"
#define CLIENTID1   "11111111111122sub"
#define TOPIC       "mqtt/11111111111111"
#define TOPIC1       "mqtt/11111111111122"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

extern DATAS_BUFF_T   comBuff0; 

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}
int msgarrvd1(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;

    printf(" new mqtt Message arrived:");
    printf(" topic: %s", topicName);
	printf(" topicLen:%d\n ",message->payloadlen);

    payloadptr = message->payload;
    // for(i=0; i<message->payloadlen; i++)
    // {
        // putchar(*payloadptr++);
    // }
    // putchar('\n');
	
	pthread_mutex_lock(&comBuff0.lock);

	AP_circleBuff_WritePacket(payloadptr++,message->payloadlen,MQTPA2DTU);
	pthread_cond_signal(&comBuff0.newPacketFlag);
	pthread_mutex_unlock(&comBuff0.lock);
	
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;

    printf(" new mqtt Message arrived:");
    printf(" topic: %s", topicName);
	printf(" topicLen:%d\n ",message->payloadlen);
   // printf(" message: ");


    payloadptr = message->payload;
   // for(i=0; i<message->payloadlen; i++)
    // {
        // putchar(*payloadptr++);
    // }
    // putchar('\n');
	
	pthread_mutex_lock(&comBuff0.lock);

	AP_circleBuff_WritePacket(payloadptr++,message->payloadlen,MQTPA2DTU);
	pthread_cond_signal(&comBuff0.newPacketFlag);
	pthread_mutex_unlock(&comBuff0.lock);
	
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

void *mqtt_sub_treat(int argc, char* argv[])
{
    MQTTClient client,client1;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;

	//--------------------------------------------------------------
	printf("--11-----enter mqtt_bub_treat-----------------\n");
    MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 60;
    conn_opts.cleansession = 1;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("sub Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC, QOS);
//--------------------------------------------------------------
//    MQTTClient_create(&client1, ADDRESS, CLIENTID1,
//        MQTTCLIENT_PERSISTENCE_NONE, NULL);
//    conn_opts.keepAliveInterval = 20;
//    conn_opts.cleansession = 1;
//
//    MQTTClient_setCallbacks(client1, NULL, connlost, msgarrvd1, delivered);
//
//    if ((rc = MQTTClient_connect(client1, &conn_opts)) != MQTTCLIENT_SUCCESS)
//    {
//        printf("Failed to connect, return code %d\n", rc);
//        exit(EXIT_FAILURE);
//    }
//    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
//           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID1, QOS);
//    MQTTClient_subscribe(client1, TOPIC1, QOS);
//--------------------------------------------------------------
	
    do 
    {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    //return rc;
}
